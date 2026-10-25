#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Bouton "disquette" (enregistrer), dessine sans image.
class FloppyButton : public juce::Button
{
public:
    FloppyButton() : juce::Button ("Enregistrer le preset")
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool isHighlighted, bool isDown) override
    {
        const auto full = getLocalBounds().toFloat().reduced (1.0f);

        g.setColour (isDown ? juce::Colour (0xff3b4046) : isHighlighted ? juce::Colour (0xff30353a)
                                                                        : juce::Colour (0xff2a2e32));
        g.fillRoundedRectangle (full, 4.0f);
        g.setColour (juce::Colour (0xff5a6068));
        g.drawRoundedRectangle (full, 4.0f, 1.0f);

        const auto inner = full.reduced (6.0f);
        const float s = juce::jmin (inner.getWidth(), inner.getHeight());
        const auto r = juce::Rectangle<float> (s, s).withCentre (inner.getCentre());
        const float c = s * 0.22f;   // coin coupe

        // Corps de la disquette
        juce::Path body;
        body.startNewSubPath (r.getX(), r.getY());
        body.lineTo (r.getRight() - c, r.getY());
        body.lineTo (r.getRight(), r.getY() + c);
        body.lineTo (r.getRight(), r.getBottom());
        body.lineTo (r.getX(), r.getBottom());
        body.closeSubPath();
        g.setColour (juce::Colour (0xff3d7ab8));
        g.fillPath (body);

        // Volet metallique et fente
        const auto shutter = juce::Rectangle<float> (r.getX() + s * 0.2f, r.getY(), s * 0.55f, s * 0.32f);
        g.setColour (juce::Colour (0xffd8dce0));
        g.fillRect (shutter);
        g.setColour (juce::Colour (0xff23272a));
        g.fillRect (juce::Rectangle<float> (shutter.getX() + shutter.getWidth() * 0.62f, shutter.getY() + s * 0.05f,
                                            s * 0.1f, s * 0.2f));

        // Etiquette
        g.setColour (juce::Colour (0xfff0f0f0));
        g.fillRect (juce::Rectangle<float> (r.getX() + s * 0.15f, r.getY() + s * 0.52f, s * 0.7f, s * 0.48f));
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloppyButton)
};
