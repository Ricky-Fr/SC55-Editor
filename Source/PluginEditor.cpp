#include "PluginEditor.h"

namespace
{
    const juce::Colour kGlobalThumbColour (0xffff9a2e);   // curseur orange quand le slider est en mode global
}

SC55Editor::SC55Editor (SC55Processor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // --- Part + bouton ALL ---
    for (int i = 1; i <= 16; ++i)
        partBox.addItem ("Part " + juce::String (i), i);
    partBox.setSelectedId (1, juce::dontSendNotification);
    partBox.onChange = [this]
    {
        populateInstrumentBox();
        refreshInstrument();
        refreshSlidersFromState();
    };
    addAndMakeVisible (partBox);

    allButton.onClick = [this]
    {
        updateLedStates();
        refreshSlidersFromState();
        status.setText (allButton.getToggleState() ? "ALL actif : commandes globales possibles"
                                                   : "ALL inactif : reglages de la part",
                        juce::dontSendNotification);
    };
    addAndMakeVisible (allButton);

    // --- Liste des instruments ---
    instrumentBox.onChange = [this] { onInstrumentBoxChanged(); };
    addAndMakeVisible (instrumentBox);

    // --- Colonne 0 : slider INSTRUMENT (numero 1..128) ---
    {
        SysExParamSlider::Config config;
        config.minValue     = 1;
        config.maxValue     = 128;
        config.defaultValue = 1;
        config.formatter    = [] (int v) { return juce::String (v).paddedLeft ('0', 3); };
        config.parser       = [] (const juce::String& t) { return t.getIntValue(); };

        sliders[0] = std::make_unique<SysExParamSlider> (config,
            [this] (int value, bool immediate) { onInstrumentSliderChanged (value, immediate); });
        addAndMakeVisible (*sliders[0]);

        labels[0].setText ("INSTRUMENT", juce::dontSendNotification);
    }

    // --- Colonnes suivantes : un slider + un bouton GLOBAL par parametre de la table ---
    for (int id = 0; id < SC55::NumPartParams; ++id)
    {
        const auto& info = SC55::getPartParamInfo (id);
        const size_t column = (size_t) id + 1;

        SysExParamSlider::Config config;
        config.minValue     = info.rawMin - info.displayOffset;
        config.maxValue     = info.rawMax - info.displayOffset;
        config.defaultValue = SC55::getDefaultRaw (info, 0) - info.displayOffset;
        config.formatter    = [this, id] (int v) { return SC55::formatPartParam (id, v, isGlobalActive (id)); };
        config.parser       = [id] (const juce::String& t) { return SC55::parsePartParam (id, t); };

        sliders[column] = std::make_unique<SysExParamSlider> (config,
            [this, id] (int value, bool immediate) { onParamSliderChanged (id, value, immediate); });
        addAndMakeVisible (*sliders[column]);

        labels[column].setText (info.label, juce::dontSendNotification);

        if (info.globalKind != SC55::GlobalNone)
        {
            auto button = std::make_unique<RoundLedButton> ("GLOBAL", 10.0f, 13.0f);
            button->onClick = [this]
            {
                updateLedStates();
                refreshSlidersFromState();
            };
            addAndMakeVisible (*button);
            globalButtons[(size_t) id] = std::move (button);
        }
    }

    for (auto& label : labels)
    {
        label.setFont (juce::Font (juce::FontOptions (12.5f)));
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, juce::Colour (0xffd0d0d0));
        addAndMakeVisible (label);
    }

    // --- Boutons ---
    resetButton.onClick = [this]
    {
        audioProcessor.sendGsReset();
        status.setText ("GS Reset envoye", juce::dontSendNotification);
    };
    addAndMakeVisible (resetButton);

    resendButton.onClick = [this]
    {
        audioProcessor.resendAll();
        status.setText ("Etat renvoye (16 parts + globaux)", juce::dontSendNotification);
    };
    addAndMakeVisible (resendButton);

    // --- Ligne de statut (style ecran LCD) ---
    status.setText ("Pret", juce::dontSendNotification);
    status.setColour (juce::Label::textColourId, juce::Colour (0xff9be564));
    status.setColour (juce::Label::backgroundColourId, juce::Colour (0xff0d1a0d));
    status.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (status);

    populateInstrumentBox();
    refreshInstrument();
    updateLedStates();
    refreshSlidersFromState();

    setSize (720, 520);
}

// ---------------------------------------------------------------------------
// Mode global
// ---------------------------------------------------------------------------

// La commande est globale seulement si ALL ET le bouton GLOBAL du slider sont allumes
bool SC55Editor::isGlobalActive (int paramId) const
{
    const auto& info = SC55::getPartParamInfo (paramId);
    const auto& button = globalButtons[(size_t) paramId];

    return info.globalKind != SC55::GlobalNone
        && allButton.getToggleState()
        && button != nullptr
        && button->getToggleState();
}

void SC55Editor::updateLedStates()
{
    const bool all = allButton.getToggleState();

    for (auto& button : globalButtons)
        if (button != nullptr)
            button->setEffectiveActive (all);
}

// ---------------------------------------------------------------------------
// Instrument
// ---------------------------------------------------------------------------

void SC55Editor::populateInstrumentBox()
{
    instrumentBox.clear (juce::dontSendNotification);

    if (currentPart() == kDrumPart)
    {
        for (int i = 0; i < SC55::kNumDrumSets; ++i)
            instrumentBox.addItem (SC55::kDrumSets[i].name, kDrumIdBase + i);
        return;
    }

    // Un sous-menu par famille de 8 instruments ; les variations suivent un separateur
    auto* root = instrumentBox.getRootMenu();

    for (int family = 0; family < 16; ++family)
    {
        juce::PopupMenu sub;
        const int firstPc = family * 8 + 1;
        const int lastPc  = firstPc + 7;

        for (int i = 0; i < SC55::kNumInstruments; ++i)
        {
            const auto& e = SC55::kInstruments[i];
            if (e.pc >= firstPc && e.pc <= lastPc && e.cc0 == 0)
                sub.addItem (1 + i, e.name);
        }

        bool separatorAdded = false;
        for (int i = 0; i < SC55::kNumInstruments; ++i)
        {
            const auto& e = SC55::kInstruments[i];
            if (e.pc >= firstPc && e.pc <= lastPc && e.cc0 != 0)
            {
                if (! separatorAdded)
                {
                    sub.addSeparator();
                    separatorAdded = true;
                }
                sub.addItem (1 + i, e.name);
            }
        }

        root->addSubMenu (SC55::kInstrumentFamilies[family], sub);
    }
}

void SC55Editor::updateInstrumentBoxSelection()
{
    const int part = currentPart();
    const int pc   = audioProcessor.getPartTonePc (part) + 1;    // 1..128
    const int cc0  = audioProcessor.getPartToneCc0 (part);

    if (part == kDrumPart)
    {
        const int i = SC55::findDrumSetIndex (pc);
        if (i >= 0)
            instrumentBox.setSelectedId (kDrumIdBase + i, juce::dontSendNotification);
        else
            instrumentBox.setText (SC55::getDrumSetName (pc), juce::dontSendNotification);
    }
    else
    {
        const int i = SC55::findInstrumentIndex (pc, cc0);
        if (i >= 0)
            instrumentBox.setSelectedId (1 + i, juce::dontSendNotification);
        else
            instrumentBox.setText (SC55::getInstrumentName (pc, cc0), juce::dontSendNotification);
    }
}

void SC55Editor::refreshInstrument()
{
    sliders[0]->setValueSilently (audioProcessor.getPartTonePc (currentPart()) + 1);
    updateInstrumentBoxSelection();
}

void SC55Editor::onInstrumentSliderChanged (int displayValue, bool immediate)
{
    const int part = currentPart();

    // Le slider choisit l'instrument de base (variation 0)
    audioProcessor.setPartInstrument (part, displayValue - 1, 0, immediate);
    updateInstrumentBoxSelection();

    status.setText ("Part " + juce::String (part + 1) + " INSTRUMENT = "
                        + juce::String (displayValue).paddedLeft ('0', 3) + " " + instrumentBox.getText(),
                    juce::dontSendNotification);
}

void SC55Editor::onInstrumentBoxChanged()
{
    const int part = currentPart();
    const int id   = instrumentBox.getSelectedId();

    int pc = 0, cc0 = 0;

    if (id >= kDrumIdBase && id - kDrumIdBase < SC55::kNumDrumSets)
        pc = SC55::kDrumSets[id - kDrumIdBase].pc - 1;
    else if (id >= 1 && id <= SC55::kNumInstruments)
    {
        pc  = SC55::kInstruments[id - 1].pc - 1;
        cc0 = SC55::kInstruments[id - 1].cc0;
    }
    else
        return;

    audioProcessor.setPartInstrument (part, pc, cc0, true);
    sliders[0]->setValueSilently (pc + 1);

    status.setText ("Part " + juce::String (part + 1) + " INSTRUMENT = "
                        + juce::String (pc + 1).paddedLeft ('0', 3) + " " + instrumentBox.getText()
                        + (cc0 != 0 ? " (var " + juce::String (cc0) + ")" : juce::String()),
                    juce::dontSendNotification);
}

// ---------------------------------------------------------------------------
// Sliders de parametres
// ---------------------------------------------------------------------------

void SC55Editor::onParamSliderChanged (int paramId, int displayValue, bool immediate)
{
    const auto& info = SC55::getPartParamInfo (paramId);
    const int raw = displayValue + info.displayOffset;

    if (isGlobalActive (paramId))
    {
        audioProcessor.setGlobalParam (paramId, raw, immediate);

        if (info.globalKind == SC55::GlobalDeviceId)
            status.setText ("Device ID de l'editeur = " + juce::String (displayValue)
                                + " (a regler aussi sur le SC-55)",
                            juce::dontSendNotification);
        else
            status.setText ("GLOBAL " + juce::String (info.label) + " = "
                                + SC55::formatPartParam (paramId, displayValue, true),
                            juce::dontSendNotification);
    }
    else
    {
        const int part = currentPart();
        audioProcessor.setPartParam (part, paramId, raw, immediate);

        status.setText ("Part " + juce::String (part + 1) + " " + juce::String (info.label)
                            + " = " + SC55::formatPartParam (paramId, displayValue, false),
                        juce::dontSendNotification);
    }
}

// Affiche, pour chaque slider, la valeur de la part ou la valeur globale selon le mode
void SC55Editor::refreshSlidersFromState()
{
    const int part = currentPart();

    for (int id = 0; id < SC55::NumPartParams; ++id)
    {
        const auto& info = SC55::getPartParamInfo (id);
        auto& slider = *sliders[(size_t) id + 1];

        if (isGlobalActive (id))
        {
            slider.setValueRange (info.gRawMin - info.displayOffset, info.gRawMax - info.displayOffset);
            slider.setValueSilently (audioProcessor.getGlobalParam (id) - info.displayOffset);
            slider.setDefaultValue (info.gRawDefault - info.displayOffset);
            slider.setColour (juce::Slider::thumbColourId, kGlobalThumbColour);
        }
        else
        {
            slider.setValueRange (info.rawMin - info.displayOffset, info.rawMax - info.displayOffset);
            slider.setValueSilently (audioProcessor.getPartParam (part, id) - info.displayOffset);
            slider.setDefaultValue (SC55::getDefaultRaw (info, part) - info.displayOffset);
            slider.removeColour (juce::Slider::thumbColourId);
        }

        slider.refreshDisplayedText();
        slider.repaint();
    }
}

// ---------------------------------------------------------------------------
// Dessin et disposition
// ---------------------------------------------------------------------------

void SC55Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff23272a));
    g.setColour (juce::Colour (0xffd0d0d0));
    g.setFont (18.0f);
    g.drawText ("SC-55 Editor - test SysEx", getLocalBounds().removeFromTop (36),
                juce::Justification::centred);
}

void SC55Editor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (28);                                   // titre

    // Ligne Part (liste raccourcie) + bouton ALL
    auto topRow = area.removeFromTop (28);
    partBox.setBounds (topRow.removeFromLeft (130));
    topRow.removeFromLeft (14);
    allButton.setBounds (topRow.removeFromLeft (70));
    area.removeFromTop (8);

    // Liste des instruments sur toute la largeur
    instrumentBox.setBounds (area.removeFromTop (28));
    area.removeFromTop (12);

    status.setBounds (area.removeFromBottom (28));
    area.removeFromBottom (8);

    auto buttons = area.removeFromBottom (32);
    resetButton.setBounds (buttons.removeFromLeft (buttons.getWidth() / 2 - 4));
    buttons.removeFromLeft (8);
    resendButton.setBounds (buttons);
    area.removeFromBottom (8);

    // Une colonne par parametre : nom, bouton GLOBAL, slider
    const int columnWidth = area.getWidth() / kNumColumns;
    for (int c = 0; c < kNumColumns; ++c)
    {
        auto column = area.removeFromLeft (columnWidth);

        labels[(size_t) c].setBounds (column.removeFromTop (24));

        auto globalRow = column.removeFromTop (24);
        if (c > 0 && globalButtons[(size_t) c - 1] != nullptr)
            globalButtons[(size_t) c - 1]->setBounds (globalRow.reduced (6, 0));

        sliders[(size_t) c]->setBounds (column.reduced (4, 0));
    }
}
