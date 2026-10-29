#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SC55Config.h"

// Apparence de l'editeur : identique au style JUCE par defaut, sauf les sliders lineaires
// qui affichent trois reperes blancs discrets (valeur min, mediane, max).
//
//  - la position des reperes vient de Slider::getPositionOfValue : elle suit donc le sens
//    du slider (vertical : min en bas, max en haut ; horizontal : min a gauche, max a droite)
//    et la plage courante (qui change en mode global)
//  - les reperes sont perpendiculaires a la piste ; le repere median reste de part et
//    d'autre de la piste, sans la traverser
//  - reglages : SC55Config::kSliderMark... dans SC55Config.h
class SC55LookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearVertical)
        {
            juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                                    minSliderPos, maxSliderPos, style, slider);
            return;
        }

        const bool horizontal = (style == juce::Slider::LinearHorizontal);
        const float fx = (float) x, fy = (float) y, fw = (float) width, fh = (float) height;

        // Piste (meme geometrie que LookAndFeel_V4)
        const float trackWidth = juce::jmin (6.0f, horizontal ? fh * 0.25f : fw * 0.25f);
        const juce::Point<float> start (horizontal ? fx : fx + fw * 0.5f,
                                        horizontal ? fy + fh * 0.5f : fy + fh);
        const juce::Point<float> end (horizontal ? fx + fw : start.x,
                                      horizontal ? start.y : fy);
        const juce::Point<float> thumb (horizontal ? sliderPos : start.x,
                                        horizontal ? start.y : sliderPos);

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath (start);
        backgroundTrack.lineTo (end);
        g.setColour (slider.findColour (juce::Slider::backgroundColourId));
        g.strokePath (backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        // Partie "remplie" : du minimum jusqu'a la valeur courante
        juce::Path valueTrack;
        valueTrack.startNewSubPath (start);
        valueTrack.lineTo (thumb);
        g.setColour (slider.findColour (juce::Slider::trackColourId));
        g.strokePath (valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        // Reperes, dessines sous le curseur
        const float thumbSize = (float) getSliderThumbRadius (slider);
        drawMarks (g, slider, horizontal, start, thumbSize * 0.5f, horizontal ? fh : fw);

        // Curseur
        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        g.fillEllipse (juce::Rectangle<float> (thumbSize, thumbSize).withCentre (thumb));
    }

private:
    // trackAxisPoint : un point de l'axe de la piste ; thumbRadius : rayon du curseur ;
    // crossSize : dimension disponible perpendiculairement a la piste
    static void drawMarks (juce::Graphics& g, juce::Slider& slider, bool horizontal,
                           juce::Point<float> trackAxisPoint, float thumbRadius, float crossSize)
    {
        using namespace SC55Config;

        const double lo = slider.getMinimum();
        const double hi = slider.getMaximum();
        if (hi <= lo)
            return;

        // Les reperes ne doivent pas deborder du composant
        const float limit = crossSize * 0.5f - 0.5f;
        float outer = juce::jmin (thumbRadius + kSliderMarkGap + kSliderMarkLength, limit);
        float inner = juce::jmin (thumbRadius + kSliderMarkGap, outer - 2.0f);

        // Rognage : toutes les extremites exterieures reculent (les reperes restent alignes) et
        // le bord interieur des reperes lateraux se rapproche de la piste
        outer -= kSliderMarkOuterTrim;
        inner -= kSliderMarkInnerTrim;

        g.setColour (juce::Colours::white.withAlpha (kSliderMarkAlpha));

        auto drawMark = [&] (double value, bool crossTrack)
        {
            const float pos = slider.getPositionOfValue (value);

            // from / to : ecarts par rapport a l'axe de la piste, perpendiculairement a elle
            auto segment = [&] (float from, float to)
            {
                if (horizontal)
                    g.drawLine (pos, trackAxisPoint.y + from, pos, trackAxisPoint.y + to, kSliderMarkThickness);
                else
                    g.drawLine (trackAxisPoint.x + from, pos, trackAxisPoint.x + to, pos, kSliderMarkThickness);
            };

            if (crossTrack)
            {
                segment (-outer, outer);
            }
            else
            {
                segment (-outer, -inner);
                segment (inner, outer);
            }
        };

        drawMark (lo, kSliderEndMarksCrossTrack);
        drawMark (hi, kSliderEndMarksCrossTrack);
        drawMark ((lo + hi) * 0.5, false);       // repere median : jamais a travers la piste
    }
};
