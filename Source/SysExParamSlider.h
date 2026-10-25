#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

// Slider reutilisable pour un parametre SysEx a valeur entiere.
//
// Comportement commun a tous les sliders :
//  - chaque changement de valeur appelle sendFn (value, false) ;
//    la file d'envoi se charge de limiter le debit pendant le glissement
//  - a la fin du glissement, sendFn (value, true) force l'envoi de la valeur finale
//  - double-clic : retour a la valeur par defaut
//
// Les valeurs manipulees ici sont les valeurs AFFICHEES ; la conversion vers la
// valeur envoyee au SC-55 est faite par l'appelant.
class SysExParamSlider : public juce::Slider
{
public:
    struct Config
    {
        int minValue     = 0;
        int maxValue     = 127;
        int defaultValue = 0;
        juce::Slider::SliderStyle style = juce::Slider::LinearVertical;
        juce::Slider::TextEntryBoxPosition textBoxPosition = juce::Slider::TextBoxBelow;
        int textBoxWidth  = 60;
        int textBoxHeight = 20;

        // Optionnels : format d'affichage et lecture d'une valeur saisie
        std::function<juce::String (int)>              formatter;
        std::function<int (const juce::String&)>       parser;
    };

    // value : nouvelle valeur ; immediate : true en fin de glissement
    using SendFn = std::function<void (int value, bool immediate)>;

    SysExParamSlider (const Config& config, SendFn sendFunction)
        : sendFn (std::move (sendFunction))
    {
        setSliderStyle (config.style);
        setTextBoxStyle (config.textBoxPosition, false, config.textBoxWidth, config.textBoxHeight);
        setRange ((double) config.minValue, (double) config.maxValue, 1.0);

        if (config.formatter)
        {
            auto fmt = config.formatter;
            textFromValueFunction = [fmt] (double v) { return fmt (juce::roundToInt (v)); };
        }

        if (config.parser)
        {
            auto parse = config.parser;
            valueFromTextFunction = [parse] (const juce::String& text) { return (double) parse (text); };
        }

        setDefaultValue (config.defaultValue);
        setValue ((double) config.defaultValue, juce::dontSendNotification);
        updateText();

        onValueChange = [this] { if (sendFn) sendFn (getIntValue(), false); };
        onDragEnd     = [this] { if (sendFn) sendFn (getIntValue(), true);  };
    }

    int getIntValue() const                { return juce::roundToInt (getValue()); }

    // Met a jour l'affichage sans envoyer de message
    void setValueSilently (int newValue)   { setValue ((double) newValue, juce::dontSendNotification); }

    // Valeur utilisee par le double-clic
    void setDefaultValue (int newDefault)  { setDoubleClickReturnValue (true, (double) newDefault); }

    // Change la plage affichee (sans envoyer de message)
    void setValueRange (int newMin, int newMax)
    {
        setRange ((double) newMin, (double) newMax, 1.0);
    }

    // A appeler quand la fonction de formatage a change de comportement
    void refreshDisplayedText()            { updateText(); }

private:
    SendFn sendFn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SysExParamSlider)
};
