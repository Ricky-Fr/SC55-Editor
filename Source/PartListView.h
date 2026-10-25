#pragma once

#include <array>
#include <functional>
#include <memory>
#include "PluginProcessor.h"
#include "SysExParamSlider.h"

// Bouton rond avec pictogramme de haut-parleur : SOLO (ondes) ou MUTE (croix).
class SpeakerButton : public juce::Button
{
public:
    enum class Kind { Solo, Mute };

    explicit SpeakerButton (Kind buttonKind);

    // 0 = eteint, 1 = allume, 2 = allume attenue (ex. part rendue muette par un solo)
    void setLitState (int newState);

    void paintButton (juce::Graphics& g, bool isHighlighted, bool isDown) override;

private:
    Kind kind;
    int litState = 0;
};

// Voyant rond rose "EFX" (indicateur seul, non cliquable).
class EfxIndicator : public juce::Component
{
public:
    EfxIndicator();

    void setLit (bool shouldBeLit);
    void paint (juce::Graphics& g) override;

private:
    bool lit = false;
};

// Ecran qui affiche les 16 parts d'un coup : numero, SOLO, MUTE, EFX, instrument,
// volume (modifiable) et bouton EDIT. Se superpose a l'ecran de reglage d'une part.
class PartListView : public juce::Component
{
public:
    explicit PartListView (SC55Processor& processorToUse);

    // Appeles par l'ecran
    std::function<void (int part)>               onEditRequested;   // bouton EDIT d'une ligne
    std::function<void()>                        onCloseRequested;  // croix en haut a droite
    std::function<void (const juce::String&)>    onStatus;          // message pour la ligne de statut
    std::function<bool()>                        isAllActive;       // etat du bouton ALL (solo multiple si allume)

    // Relit l'etat du processeur (volumes, instruments, mute, solo, EFX)
    void refresh();

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    struct Row
    {
        SpeakerButton solo { SpeakerButton::Kind::Solo };
        SpeakerButton mute { SpeakerButton::Kind::Mute };
        EfxIndicator  efx;
        std::unique_ptr<SysExParamSlider> level;
        juce::TextButton edit { "EDIT" };
        juce::String instrumentName;
    };

    juce::String instrumentNameForPart (int part) const;

    SC55Processor& processor;
    std::array<Row, 16> rows;
    juce::TextButton closeButton { "X" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PartListView)
};
