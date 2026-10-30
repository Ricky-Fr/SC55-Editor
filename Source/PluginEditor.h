#pragma once

#include <array>
#include <memory>
#include "PluginProcessor.h"
#include "PartParams.h"
#include "InstrumentData.h"
#include "SysExParamSlider.h"
#include "RoundLedButton.h"
#include "FloppyButton.h"
#include "PartListView.h"
#include "AdvancedView.h"
#include "SystemView.h"
#include "HeaderBar.h"
#include "SC55LookAndFeel.h"

class SC55Editor : public juce::AudioProcessorEditor,
                   private juce::Timer
{
public:
    explicit SC55Editor (SC55Processor&);
    ~SC55Editor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Colonne 0 = INSTRUMENT, colonnes suivantes = parametres de la table SC55::getPartParamInfo
    static constexpr int kNumColumns = SC55::NumPartParams + 1;
    static constexpr int kDrumIdBase = 1000;   // identifiants des jeux de batterie dans la liste

    int  currentPart() const              { return partBox.getSelectedId() - 1; }
    bool isGlobalActive (int paramId) const;

    // Affichage
    void refreshForCurrentPart();
    void populateInstrumentBox();
    void updateInstrumentBoxSelection();
    void refreshInstrument();
    void refreshSlidersFromState();
    void updateLedStates();
    void refreshMixLeds();
    void showPartList (bool shouldShow);
    void showAdvanced (bool shouldShow);
    void showSystem (bool shouldShow);
    void updateViewTitle();      // Main / View / Advanced / System dans le bandeau du haut

    // Sliders et instrument
    void onParamSliderChanged (int paramId, int displayValue, bool immediate);
    void onInstrumentSliderChanged (int displayValue, bool immediate);
    void onInstrumentBoxChanged();

    // Mute / solo
    void onMuteClicked();
    void onSoloClicked();

    // Presets
    void populatePresetBox();
    void onPresetSelected();
    void onSavePreset();
    void onDeletePreset();
    void disarmDelete();
    void timerCallback() override;      // annule la confirmation de suppression

    SC55LookAndFeel lookAndFeel;       // declaree en premier : detruite en dernier
    SC55Processor& audioProcessor;

    // Bandeau du haut : logo, titre avec le nom de la vue, signature
    HeaderBar        headerBar;

    // Presets
    juce::Label      presetLabel { {}, "PRESET" };
    juce::ComboBox   presetBox;
    FloppyButton     saveButton;
    juce::TextButton deleteButton { "Suppr." };
    bool             deleteArmed = false;

    // Part, ALL, MUTE, SOLO, VIEW
    juce::ComboBox   partBox;
    RoundLedButton   allButton  { "ALL",  13.0f, 18.0f };
    RoundLedButton   muteButton { "MUTE", 13.0f, 18.0f };
    RoundLedButton   soloButton { "SOLO", 13.0f, 18.0f };
    juce::TextButton viewButton { "VIEW" };
    juce::TextButton advancedButton { "ADVANCED" };
    juce::TextButton systemButton { "SYSTEM" };
    juce::ComboBox   instrumentBox;

    std::array<juce::Label, kNumColumns> labels;
    std::array<std::unique_ptr<SysExParamSlider>, kNumColumns> sliders;
    std::array<std::unique_ptr<RoundLedButton>, SC55::NumPartParams> globalButtons;

    juce::TextButton resetButton  { "GS Reset" };
    juce::TextButton resendButton { "Renvoyer l'etat" };
    juce::Label      status;

    // Ecran des 16 parts, superpose a l'ecran de reglage d'une part
    PartListView     partListView { audioProcessor };

    // Ecran des reglages avances (Tone Modify, Part Setup, Pitch), superpose lui aussi
    AdvancedView     advancedView { audioProcessor };

    // Ecran des reglages generaux (accord, reverb, chorus)
    SystemView       systemView { audioProcessor };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SC55Editor)
};
