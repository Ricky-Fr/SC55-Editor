#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Bouton rond a bascule avec voyant lumineux et texte a droite (style ALL / GLOBAL).
//
// setEffectiveActive() : permet d'afficher un bouton "arme" mais non effectif
// (voyant allume en teinte attenuee), par exemple un bouton GLOBAL allume tant que
// le bouton ALL est eteint.
class RoundLedButton : public juce::Button
{
public:
    RoundLedButton (const juce::String& text, float fontSize = 11.0f, float ledDiameter = 14.0f)
        : juce::Button (text), textSize (fontSize), diameter (ledDiameter)
    {
        setClickingTogglesState (true);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setEffectiveActive (bool shouldBeBright)
    {
        if (effectiveActive != shouldBeBright)
        {
            effectiveActive = shouldBeBright;
            repaint();
        }
    }

    void paintButton (juce::Graphics& g, bool isHighlighted, bool isDown) override
    {
        const auto area = getLocalBounds().toFloat();
        const float d = juce::jmin (diameter, area.getHeight() - 2.0f);
        const auto led = juce::Rectangle<float> (area.getX() + 3.0f, area.getCentreY() - d * 0.5f, d, d);
        const bool on = getToggleState();

        const auto litColour = effectiveActive ? juce::Colour (0xffff9a2e) : juce::Colour (0xff8a5a1e);

        if (on)   // halo lumineux
        {
            g.setColour (litColour.withAlpha (effectiveActive ? 0.30f : 0.15f));
            g.fillEllipse (led.expanded (3.0f));
        }

        const float cx = led.getCentreX() - d * 0.15f;
        const float cy = led.getCentreY() - d * 0.15f;

        if (on)
            g.setGradientFill (juce::ColourGradient (litColour.brighter (0.6f), cx, cy,
                                                     litColour.darker (0.5f), led.getRight(), led.getBottom(), true));
        else
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3f45), cx, cy,
                                                     juce::Colour (0xff15181b), led.getRight(), led.getBottom(), true));

        g.fillEllipse (led);

        g.setColour ((isHighlighted || isDown) ? juce::Colour (0xffb0b6bd) : juce::Colour (0xff5a6068));
        g.drawEllipse (led, 1.2f);

        g.setColour (on ? juce::Colour (0xffe8e8e8) : juce::Colour (0xff9aa0a6));
        g.setFont (textSize);
        g.drawText (getButtonText(),
                    juce::Rectangle<float> (led.getRight() + 6.0f, area.getY(),
                                            area.getRight() - led.getRight() - 6.0f, area.getHeight()),
                    juce::Justification::centredLeft);
    }

private:
    float textSize;
    float diameter;
    bool effectiveActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoundLedButton)
};
