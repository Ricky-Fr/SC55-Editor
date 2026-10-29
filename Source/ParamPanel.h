#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>
#include "SysExParamSlider.h"

// Format d'affichage d'un slider
enum class ParamFormat
{
    Plain,       // 64
    Signed,      // +0, -25
    Note,        // C-1 ... G9
    FineHz,      // +0.0 Hz
    Semitones,   // 17 st
    Pan,         // L63, 0, R63
    TuneCents    // +0.0 ct (valeur en dixiemes de cent)
};

// Base des ecrans de reglage superposes (ADVANCED, SYSTEM) : quatre zones separees par des
// filets, chacune avec un titre et des lignes "nom + slider" ou "nom + liste".
//
//        zone 0 | zone 1
//        -------+-------      le filet vertical est au milieu, le filet horizontal est
//        zone 2 | zone 3      a la hauteur choisie par l'ecran derive
//
// L'ecran derive ajoute ses lignes et dit comment lire / ecrire chaque valeur.
class ParamPanel : public juce::Component
{
public:
    std::function<void()>                      onCloseRequested;   // croix en haut a droite
    std::function<void (const juce::String&)>  onStatus;           // message pour la ligne de statut

    // Relit toutes les valeurs (sans rien envoyer)
    void refresh();

    void paint (juce::Graphics& g) override;
    void resized() override;

protected:
    // Reference a un parametre : 'source' designe la table / l'origine (defini par l'ecran derive)
    struct ParamRef
    {
        int source;
        int id;
    };

    struct Range
    {
        int rawMin;
        int rawMax;
        int displayOffset;      // valeur affichee = valeur envoyee - displayOffset
    };

    static constexpr int kGroupGap = 5;     // espace entre deux groupes de lignes

    // zoneTitles : un titre par zone (vide = zone sans titre) ; splitY : position du filet horizontal
    ParamPanel (const juce::StringArray& zoneTitles, int splitY);
    ~ParamPanel() override;

    // A fournir par l'ecran derive
    virtual Range getRange (ParamRef ref) const = 0;
    virtual int   getDefaultRaw (ParamRef ref) const = 0;
    virtual int   getRaw (ParamRef ref) const = 0;
    virtual void  setRaw (ParamRef ref, int rawValue, bool immediate) = 0;
    virtual juce::String getStatusPrefix() const    { return {}; }     // ex. "Part 1 "

    // Construction (a appeler dans le constructeur de l'ecran derive)
    void addSliderRow (int zone, const juce::String& labelText, ParamRef ref, ParamFormat format,
                       int gapBefore = 0, const juce::String& statusName = {});
    void addComboRow  (int zone, const juce::String& labelText, ParamRef ref,
                       const juce::StringArray& items, const juce::String& statusName = {});
    void addNote (int zone, const juce::String& text);

    // Texte en haut a droite, a gauche de la croix
    void setHeaderText (const juce::String& text);

private:
    struct Row;
    struct Note
    {
        int zone;
        juce::String text;
    };

    juce::Rectangle<int> zoneBounds (int zone) const;
    void onSliderChanged (size_t rowIndex, int displayValue, bool immediate);
    void onComboChanged (size_t rowIndex);

    juce::StringArray titles;
    int splitY;

    std::vector<std::unique_ptr<Row>> rows;
    std::vector<Note> notes;
    juce::Label headerLabel;
    juce::TextButton closeButton { "X" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamPanel)
};

// Formats et lecture des valeurs saisies (utilises par les ecrans derives si besoin)
juce::String formatParamValue (ParamFormat format, int displayValue);
int          parseParamValue  (ParamFormat format, const juce::String& text);
