#include "PluginProcessor.h"
#include "PluginEditor.h"

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

// --- GS Reset / renvoi de l'etat ---------------------------------------------

void SC55Processor::sendGsReset()
{
    // GS Reset : F0 41 10 42 12 40 00 7F 00 41 F7, suivi d'une pause
    queueDt1 (0x40, 0x00, 0x7F, { 0x00 }, true, SC55Config::kAfterGsResetDelayMs);
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
}

// ---------------------------------------------------------------------------
// Etat (sauvegarde dans le projet de la DAW)
// ---------------------------------------------------------------------------

void SC55Processor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("SC55State");

    for (int part = 0; part < 16; ++part)
    {
        for (int id = 0; id < SC55::NumPartParams; ++id)
            xml.setAttribute ("p" + juce::String (part) + "_" + juce::String (id),
                              partValues[(size_t) part][(size_t) id]);

        xml.setAttribute ("t" + juce::String (part) + "_cc0", partTones[(size_t) part].cc0);
        xml.setAttribute ("t" + juce::String (part) + "_pc",  partTones[(size_t) part].pc);
    }

    for (int id = 0; id < SC55::NumPartParams; ++id)
        xml.setAttribute ("g" + juce::String (id), globalValues[(size_t) id]);

    copyXmlToBinary (xml, destData);
}

void SC55Processor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName ("SC55State"))
        return;

    for (int part = 0; part < 16; ++part)
    {
        for (int id = 0; id < SC55::NumPartParams; ++id)
        {
            const auto& info = SC55::getPartParamInfo (id);
            const auto name = "p" + juce::String (part) + "_" + juce::String (id);
            partValues[(size_t) part][(size_t) id] =
                juce::jlimit (info.rawMin, info.rawMax,
                              xml->getIntAttribute (name, SC55::getDefaultRaw (info, part)));
        }

        partTones[(size_t) part].cc0 = juce::jlimit (0, 127, xml->getIntAttribute ("t" + juce::String (part) + "_cc0", 0));
        partTones[(size_t) part].pc  = juce::jlimit (0, 127, xml->getIntAttribute ("t" + juce::String (part) + "_pc", 0));
    }

    for (int id = 0; id < SC55::NumPartParams; ++id)
    {
        const auto& info = SC55::getPartParamInfo (id);
        globalValues[(size_t) id] = juce::jlimit (info.gRawMin, info.gRawMax,
                                                  xml->getIntAttribute ("g" + juce::String (id), info.gRawDefault));
    }

    deviceId = (juce::uint8) globalValues[(size_t) SC55::PartMidiChannel];
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
