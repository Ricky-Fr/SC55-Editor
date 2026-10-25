#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>

SC55Processor::SC55Processor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      sysexQueue ([this] (const juce::MidiMessage& m) { pushToAudioThread (m); })
{
    // Valeurs par defaut GS
    for (int part = 0; part < 16; ++part)
        for (int id = 0; id < SC55::NumPartParams; ++id)
            partValues[(size_t) part][(size_t) id] = SC55::getDefaultRaw (SC55::getPartParamInfo (id), part);

    for (int id = 0; id < SC55::NumPartParams; ++id)
        globalValues[(size_t) id] = SC55::getPartParamInfo (id).gRawDefault;

    deviceId = (juce::uint8) globalValues[(size_t) SC55::PartMidiChannel];
    partAudible.fill (true);

    // Rappel du dernier preset utilise
    currentPresetName = PresetManager::kDefaultName;

    const auto lastPreset = presetManager.getLastPresetName();
    if (lastPreset.isNotEmpty() && applyPresetByName (lastPreset) && SC55Config::kSendPresetOnStartup)
        resendAll();
}

bool SC55Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SC55Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();   // pas d'audio : le son vient du SC-55
    midi.clear();     // on ne fait pas de MIDI thru

    const juce::SpinLock::ScopedLockType lock (pendingLock);
    if (! pending.isEmpty())
    {
        midi.addEvents (pending, 0, buffer.getNumSamples(), 0);
        pending.clear();
    }
}

void SC55Processor::pushToAudioThread (const juce::MidiMessage& message)
{
    const juce::SpinLock::ScopedLockType lock (pendingLock);
    pending.addEvent (message, 0);
}

// ---------------------------------------------------------------------------
// SysEx Roland GS
// ---------------------------------------------------------------------------

// Parts 1..16 -> chiffre d'adresse n (la part 10 est en 0, les parts 1-9 en 1-9, 11-16 en A-F)
int SC55Processor::partToBlock (int part)
{
    static const int map[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 10, 11, 12, 13, 14, 15 };
    return map[juce::jlimit (0, 15, part)];
}

void SC55Processor::queueDt1 (juce::uint8 a1, juce::uint8 a2, juce::uint8 a3,
                              const std::vector<juce::uint8>& data,
                              bool immediate, int quietAfterMs)
{
    // Format : F0 41 <dev> 42 12 <a1 a2 a3> <data...> <checksum> F7
    // (JUCE ajoute F0 et F7 lui-meme)
    std::vector<juce::uint8> msg { 0x41, deviceId, 0x42, 0x12, a1, a2, a3 };
    msg.insert (msg.end(), data.begin(), data.end());

    int sum = a1 + a2 + a3;
    for (auto d : data)
        sum += d;
    const auto checksum = (juce::uint8) ((128 - (sum % 128)) % 128);
    msg.push_back (checksum);

    const auto sysex = juce::MidiMessage::createSysExMessage (msg.data(), (int) msg.size());

    // La cle identifie le parametre : deux messages de meme adresse sont fusionnes
    const juce::uint32 key = ((juce::uint32) a1 << 16) | ((juce::uint32) a2 << 8) | (juce::uint32) a3;

    sysexQueue.enqueue (key, sysex, immediate, quietAfterMs);
}

// --- Parametres de part ------------------------------------------------------

void SC55Processor::setPartParam (int part, int paramId, int rawValue, bool immediate)
{
    part = juce::jlimit (0, 15, part);
    paramId = juce::jlimit (0, (int) SC55::NumPartParams - 1, paramId);

    const auto& info = SC55::getPartParamInfo (paramId);
    rawValue = juce::jlimit (info.rawMin, info.rawMax, rawValue);

    partValues[(size_t) part][(size_t) paramId] = rawValue;

    // Adresse : 40 <partBase | n> <offset>
    queueDt1 (0x40, (juce::uint8) (info.partBase | partToBlock (part)), info.offset,
              { (juce::uint8) rawValue }, immediate);
}

int SC55Processor::getPartParam (int part, int paramId) const
{
    return partValues[(size_t) juce::jlimit (0, 15, part)]
                     [(size_t) juce::jlimit (0, (int) SC55::NumPartParams - 1, paramId)];
}

// --- Instrument --------------------------------------------------------------

void SC55Processor::setPartInstrument (int part, int pc, int cc0, bool immediate)
{
    part = juce::jlimit (0, 15, part);
    pc   = juce::jlimit (0, 127, pc);
    cc0  = juce::jlimit (0, 127, cc0);

    partTones[(size_t) part].pc  = pc;
    partTones[(size_t) part].cc0 = cc0;

    // 40 1n 00 : Tone Number = CC#0 (variation) puis Program Change, sur 2 octets
    queueDt1 (0x40, (juce::uint8) (0x10 | partToBlock (part)), 0x00,
              { (juce::uint8) cc0, (juce::uint8) pc }, immediate);
}

int SC55Processor::getPartTonePc (int part) const   { return partTones[(size_t) juce::jlimit (0, 15, part)].pc; }
int SC55Processor::getPartToneCc0 (int part) const  { return partTones[(size_t) juce::jlimit (0, 15, part)].cc0; }

// --- Commandes globales ------------------------------------------------------

void SC55Processor::setGlobalParam (int paramId, int rawValue, bool immediate)
{
    paramId = juce::jlimit (0, (int) SC55::NumPartParams - 1, paramId);

    const auto& info = SC55::getPartParamInfo (paramId);
    rawValue = juce::jlimit (info.gRawMin, info.gRawMax, rawValue);

    globalValues[(size_t) paramId] = rawValue;

    switch (info.globalKind)
    {
        case SC55::GlobalMasterSysEx:
            queueDt1 (info.g1, info.g2, info.g3, { (juce::uint8) rawValue }, immediate);
            break;

        case SC55::GlobalAllParts:
            // Pas de SysEx master pour ce parametre : on l'applique a chaque part
            for (int part = 0; part < 16; ++part)
                setPartParam (part, paramId, rawValue, immediate);
            break;

        case SC55::GlobalDeviceId:
            // Reglage local : Device ID utilise par l'editeur, aucun message envoye
            deviceId = (juce::uint8) rawValue;
            break;

        case SC55::GlobalNone:
        default:
            break;
    }
}

int SC55Processor::getGlobalParam (int paramId) const
{
    return globalValues[(size_t) juce::jlimit (0, (int) SC55::NumPartParams - 1, paramId)];
}

// --- Mute / Solo -------------------------------------------------------------
//
// Il n'existe pas de SysEx "mute" dans la documentation du SC-55. Le mute reel d'une
// part passe par son interrupteur "Rx. Note Message" (40 1n 08) : OFF = la part ignore
// les notes, ON = fonctionnement normal. Ni le volume ni aucun autre reglage n'est
// modifie. Les notes deja en cours sont coupees par un "All Sounds Off" sur le canal
// MIDI de la part (voir silenceHangingNotes).
// applyMixState() ne reenvoie que les parts dont l'audibilite a change.

void SC55Processor::sendPartRxNote (int part)
{
    queueDt1 (0x40, (juce::uint8) (0x10 | partToBlock (part)), 0x08,
              { (juce::uint8) (partAudible[(size_t) part] ? 1 : 0) }, true);
}

void SC55Processor::silenceHangingNotes (int part)
{
    const int rxChannel = partValues[(size_t) part][(size_t) SC55::PartMidiChannel];   // 0..15, 16 = OFF
    if (rxChannel > 15)
        return;

    // Un "All Sounds Off" coupe toutes les parts de ce canal : on s'abstient si une autre
    // part encore audible l'utilise aussi.
    for (int other = 0; other < 16; ++other)
        if (other != part && partAudible[(size_t) other]
            && partValues[(size_t) other][(size_t) SC55::PartMidiChannel] == rxChannel)
            return;

    sysexQueue.enqueue (0xB0000000u | (juce::uint32) rxChannel,
                        juce::MidiMessage::allSoundOff (rxChannel + 1), true);
}

void SC55Processor::applyMixState (bool force)
{
    const bool anySolo = std::any_of (partSoloed.begin(), partSoloed.end(), [] (bool b) { return b; });

    for (int part = 0; part < 16; ++part)
    {
        const bool audible = ! partMuted[(size_t) part] && (! anySolo || partSoloed[(size_t) part]);

        if (force || audible != partAudible[(size_t) part])
        {
            partAudible[(size_t) part] = audible;
            sendPartRxNote (part);

            if (! audible && SC55Config::kSilenceHangingNotesOnMute)
                silenceHangingNotes (part);
        }
    }
}

void SC55Processor::setPartMuted (int part, bool shouldBeMuted)
{
    partMuted[(size_t) juce::jlimit (0, 15, part)] = shouldBeMuted;
    applyMixState();
}

bool SC55Processor::isPartMuted (int part) const
{
    return partMuted[(size_t) juce::jlimit (0, 15, part)];
}

void SC55Processor::setAllMuted (bool shouldBeMuted)
{
    partMuted.fill (shouldBeMuted);
    applyMixState();
}

bool SC55Processor::areAllMuted() const
{
    return std::all_of (partMuted.begin(), partMuted.end(), [] (bool b) { return b; });
}

bool SC55Processor::isPartAudible (int part) const
{
    return partAudible[(size_t) juce::jlimit (0, 15, part)];
}

void SC55Processor::toggleSolo (int part, bool additive)
{
    part = juce::jlimit (0, 15, part);

    if (additive)
    {
        // Premier appui = solo de la part, appuis suivants = ajout / retrait d'autres parts
        partSoloed[(size_t) part] = ! partSoloed[(size_t) part];
    }
    else
    {
        // Solo exclusif : un second appui sur la seule part en solo annule le solo
        const auto count = std::count (partSoloed.begin(), partSoloed.end(), true);
        const bool wasOnlySolo = partSoloed[(size_t) part] && count == 1;

        partSoloed.fill (false);
        partSoloed[(size_t) part] = ! wasOnlySolo;
    }

    applyMixState();
}

bool SC55Processor::isPartSoloed (int part) const
{
    return partSoloed[(size_t) juce::jlimit (0, 15, part)];
}

bool SC55Processor::isPartEfxModified (int) const
{
    return false;   // a completer avec la gestion des effets
}

void SC55Processor::runWhenSendingFinished (std::function<void()> callback)
{
    sysexQueue.runWhenEmpty (std::move (callback));
}

// --- GS Reset / renvoi de l'etat ---------------------------------------------

void SC55Processor::sendGsReset()
{
    // GS Reset : F0 41 10 42 12 40 00 7F 00 41 F7, suivi d'une pause
    queueDt1 (0x40, 0x00, 0x7F, { 0x00 }, true, SC55Config::kAfterGsResetDelayMs);

    // Le reset remet toutes les parts en ecoute : on reapplique le mute / solo en cours
    partAudible.fill (true);
    applyMixState();
}

void SC55Processor::resendAll()
{
    for (int part = 0; part < 16; ++part)
    {
        setPartInstrument (part, partTones[(size_t) part].pc, partTones[(size_t) part].cc0, true);

        for (int id = 0; id < SC55::NumPartParams; ++id)
            setPartParam (part, id, partValues[(size_t) part][(size_t) id], true);
    }

    for (int id = 0; id < SC55::NumPartParams; ++id)
        if (SC55::getPartParamInfo (id).globalKind == SC55::GlobalMasterSysEx)
            setGlobalParam (id, globalValues[(size_t) id], true);

    applyMixState (true);   // mute / solo : les 16 parts
}

// ---------------------------------------------------------------------------
// Etat <-> XML
// ---------------------------------------------------------------------------

std::unique_ptr<juce::XmlElement> SC55Processor::createStateXml (bool includeDeviceId) const
{
    auto xml = std::make_unique<juce::XmlElement> ("SC55State");

    for (int part = 0; part < 16; ++part)
    {
        for (int id = 0; id < SC55::NumPartParams; ++id)
            xml->setAttribute ("p" + juce::String (part) + "_" + juce::String (id),
                               partValues[(size_t) part][(size_t) id]);

        xml->setAttribute ("t" + juce::String (part) + "_cc0", partTones[(size_t) part].cc0);
        xml->setAttribute ("t" + juce::String (part) + "_pc",  partTones[(size_t) part].pc);
    }

    for (int id = 0; id < SC55::NumPartParams; ++id)
        if (includeDeviceId || id != SC55::PartMidiChannel)   // le Device ID depend du materiel, pas du son
            xml->setAttribute ("g" + juce::String (id), globalValues[(size_t) id]);

    return xml;
}

void SC55Processor::applyStateXml (const juce::XmlElement& xml, bool includeDeviceId)
{
    for (int part = 0; part < 16; ++part)
    {
        for (int id = 0; id < SC55::NumPartParams; ++id)
        {
            const auto& info = SC55::getPartParamInfo (id);
            const auto name = "p" + juce::String (part) + "_" + juce::String (id);
            partValues[(size_t) part][(size_t) id] =
                juce::jlimit (info.rawMin, info.rawMax,
                              xml.getIntAttribute (name, SC55::getDefaultRaw (info, part)));
        }

        partTones[(size_t) part].cc0 = juce::jlimit (0, 127, xml.getIntAttribute ("t" + juce::String (part) + "_cc0", 0));
        partTones[(size_t) part].pc  = juce::jlimit (0, 127, xml.getIntAttribute ("t" + juce::String (part) + "_pc", 0));
    }

    for (int id = 0; id < SC55::NumPartParams; ++id)
    {
        if (! includeDeviceId && id == SC55::PartMidiChannel)
            continue;

        const auto& info = SC55::getPartParamInfo (id);
        globalValues[(size_t) id] = juce::jlimit (info.gRawMin, info.gRawMax,
                                                  xml.getIntAttribute ("g" + juce::String (id), info.gRawDefault));
    }

    deviceId = (juce::uint8) globalValues[(size_t) SC55::PartMidiChannel];
}

// Preset "Default" : valeurs par defaut du SC-55
std::unique_ptr<juce::XmlElement> SC55Processor::createDefaultStateXml()
{
    auto xml = std::make_unique<juce::XmlElement> ("SC55Preset");

    for (int part = 0; part < 16; ++part)
    {
        for (int id = 0; id < SC55::NumPartParams; ++id)
            xml->setAttribute ("p" + juce::String (part) + "_" + juce::String (id),
                               SC55::getDefaultRaw (SC55::getPartParamInfo (id), part));

        xml->setAttribute ("t" + juce::String (part) + "_cc0", 0);
        xml->setAttribute ("t" + juce::String (part) + "_pc", 0);
    }

    for (int id = 0; id < SC55::NumPartParams; ++id)
        if (id != SC55::PartMidiChannel)
            xml->setAttribute ("g" + juce::String (id), SC55::getPartParamInfo (id).gRawDefault);

    return xml;
}

// ---------------------------------------------------------------------------
// Etat de la DAW
// ---------------------------------------------------------------------------
//
// L'etat complet est stocke dans le projet de la DAW : un projet reste donc
// autonome, meme si les fichiers de presets ne sont pas disponibles.

void SC55Processor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = createStateXml (true);
    xml->setAttribute ("preset", currentPresetName);
    copyXmlToBinary (*xml, destData);
}

void SC55Processor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName ("SC55State"))
        return;

    applyStateXml (*xml, true);
    currentPresetName = xml->getStringAttribute ("preset", currentPresetName);
}

// ---------------------------------------------------------------------------
// Presets
// ---------------------------------------------------------------------------

bool SC55Processor::applyPresetByName (const juce::String& name)
{
    const bool isDefault = presetManager.isDefault (name);

    std::unique_ptr<juce::XmlElement> xml = isDefault ? createDefaultStateXml() : presetManager.load (name);
    if (xml == nullptr)
        return false;

    applyStateXml (*xml, false);
    currentPresetName = isDefault ? juce::String (PresetManager::kDefaultName) : name;
    return true;
}

bool SC55Processor::loadPreset (const juce::String& name, bool sendToDevice)
{
    if (! applyPresetByName (name))
        return false;

    presetManager.setLastPresetName (currentPresetName);

    if (sendToDevice)
        resendAll();

    return true;
}

bool SC55Processor::saveCurrentAsPreset (const juce::String& rawName, juce::String& error)
{
    const auto name = rawName.trim().substring (0, 48);

    if (name.isEmpty())
    {
        error = "Nom de preset vide";
        return false;
    }

    if (presetManager.isDefault (name))
    {
        error = "Le preset Default est protege : choisis un autre nom";
        return false;
    }

    auto xml = createStateXml (false);
    xml->setTagName ("SC55Preset");
    xml->setAttribute ("name", name);

    if (! presetManager.save (name, *xml, error))
        return false;

    currentPresetName = name;
    presetManager.setLastPresetName (name);
    return true;
}

bool SC55Processor::deletePreset (const juce::String& name)
{
    if (presetManager.isDefault (name) || ! presetManager.remove (name))
        return false;

    if (currentPresetName == name)
        currentPresetName = juce::String();   // les valeurs restent, mais ne correspondent plus a un preset

    if (presetManager.getLastPresetName() == name)
        presetManager.setLastPresetName (PresetManager::kDefaultName);

    return true;
}

juce::AudioProcessorEditor* SC55Processor::createEditor()
{
    return new SC55Editor (*this);
}

// Point d'entree de JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SC55Processor();
}
