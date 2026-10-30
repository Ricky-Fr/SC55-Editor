#pragma once

#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>

// Bandeau du haut de l'editeur :
//
//   SOUNDCanvas              SC-55 Editor - <vue>              Ricky_Fr
//
// Police : Century Gothic (si elle est installee ; sinon Segoe UI, puis la police sans-serif par
// defaut). Les tailles sont en points. Tous les morceaux de texte d'un groupe partagent la meme
// ligne de base, comme dans la maquette.
class HeaderBar : public juce::Component
{
public:
    HeaderBar()
    {
        setInterceptsMouseClicks (false, false);
    }

    // Nom de la vue affichee apres "SC-55 Editor - " : Main, View, Advanced, System
    void setViewName (const juce::String& newName)
    {
        if (newName != viewName)
        {
            viewName = newName;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto white = juce::Colour (0xffeeeeee);

        const std::vector<Run> logo =
        {
            { "SOUND",  makeFont (18.0f, juce::Font::italic),                      white },
            { "Canvas", makeFont (24.0f, juce::Font::bold | juce::Font::italic),   white }
        };

        const std::vector<Run> title =
        {
            { "SC-55 Editor",           makeFont (18.0f, juce::Font::bold),  white },
            { " - " + viewName,         makeFont (18.0f, juce::Font::plain), white }
        };

        const std::vector<Run> signature =
        {
            { "Ricky_", makeFont (16.0f, juce::Font::italic),                    white },
            { "Fr",     makeFont (18.0f, juce::Font::bold | juce::Font::italic), white }
        };

        const float baseline = (float) getHeight() - 6.0f;

        drawRuns (g, logo,      0.0f, baseline);
        drawRuns (g, title,     ((float) getWidth() - measureRuns (title)) * 0.5f, baseline);
        drawRuns (g, signature, (float) getWidth() - measureRuns (signature), baseline);
    }

private:
    struct Run
    {
        juce::String text;
        juce::Font font;
        juce::Colour colour;
    };

    // Century Gothic si elle est installee (elle est livree avec Microsoft Office), sinon Segoe UI,
    // sinon la police sans-serif par defaut. Le resultat est memorise.
    static const juce::String& familyName()
    {
        static const juce::String name = []
        {
            const auto installed = juce::Font::findAllTypefaceNames();

            for (const char* candidate : { "Century Gothic", "Segoe UI" })
                if (installed.contains (candidate, true))
                    return juce::String (candidate);

            return juce::Font::getDefaultSansSerifFontName();
        }();

        return name;
    }

    static juce::Font makeFont (float pointHeight, int styleFlags)
    {
        return juce::Font (juce::FontOptions (familyName(), pointHeight, styleFlags).withPointHeight (pointHeight));
    }

    static float widthOf (const Run& run)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText (run.font, run.text, 0.0f, 0.0f);

        return glyphs.getNumGlyphs() > 0 ? glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth() : 0.0f;
    }

    static float measureRuns (const std::vector<Run>& runs)
    {
        float total = 0.0f;
        for (const auto& run : runs)
            total += widthOf (run);

        return total;
    }

    // Dessine les morceaux de texte les uns a la suite des autres sur une meme ligne de base
    static void drawRuns (juce::Graphics& g, const std::vector<Run>& runs, float x, float baseline)
    {
        x = std::round (x);                 // positions entieres : texte net

        for (const auto& run : runs)
        {
            juce::GlyphArrangement glyphs;
            glyphs.addLineOfText (run.font, run.text, x, std::round (baseline));

            g.setColour (run.colour);
            glyphs.draw (g);

            x += widthOf (run);
        }
    }

    juce::String viewName { "Main" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderBar)
};
