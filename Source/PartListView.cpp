#include "PartListView.h"
#include "InstrumentData.h"

namespace
{
    constexpr int kHeaderHeight = 30;
    constexpr int kTopPadding   = 6;
    constexpr int kRowHeight    = 26;
    constexpr int kDrumPart     = 9;     // part 10 (indice 9) : jeux de batterie

    // Colonnes (abscisses en pixels)
    constexpr int kNumberX = 10, kNumberW = 30;
    constexpr int kSoloX = 48, kMuteX = 78, kEfxX = 108, kButtonSize = 24;
    constexpr int kNameX = 142, kNameW = 176;
    constexpr int kLevelX = 330;
    constexpr int kEditW = 58, kRightMargin = 10;

    const juce::Colour kOrange (0xffff8c1a);      // ecran orange des noms et des valeurs
    const juce::Colour kTrackFill (0xff3a8fb0);   // partie remplie des faders de volume
}

// ---------------------------------------------------------------------------
// SpeakerButton
// ---------------------------------------------------------------------------

SpeakerButton::SpeakerButton (Kind buttonKind)
    : juce::Button (buttonKind == Kind::Solo ? "SOLO" : "MUTE"), kind (buttonKind)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void SpeakerButton::setLitState (int newState)
{
    if (litState != newState)
    {
        litState = newState;
        repaint();
    }
}

void SpeakerButton::paintButton (juce::Graphics& g, bool isHighlighted, bool isDown)
{
    const auto area = getLocalBounds().toFloat().reduced (1.0f);
    const float d = juce::jmin (area.getWidth(), area.getHeight());
    const auto face = juce::Rectangle<float> (d, d).withCentre (area.getCentre());

    // Couleur du bouton : gris, ou couleur du voyant quand il est allume
    juce::Colour base (0xffb4b4b4);
    if (litState > 0)
    {
        base = (kind == Kind::Solo) ? juce::Colour (0xffffd23a) : juce::Colour (0xffff4d3d);
        if (litState == 2)
            base = base.withMultipliedBrightness (0.6f);
    }

    g.setGradientFill (juce::ColourGradient (base.brighter (0.5f), face.getX() + d * 0.3f, face.getY() + d * 0.25f,
                                             base.darker (0.4f), face.getRight(), face.getBottom(), true));
    g.fillEllipse (face);

    g.setColour ((isHighlighted || isDown) ? juce::Colour (0xffe0e0e0) : juce::Colour (0xff4a4a4a));
    g.drawEllipse (face, 1.2f);

    // Pictogramme : haut-parleur
    const auto icon = face.reduced (d * 0.24f);
    const float x = icon.getX(), y = icon.getY(), w = icon.getWidth(), h = icon.getHeight();

    juce::Path speaker;
    speaker.startNewSubPath (x,            y + h * 0.35f);
    speaker.lineTo          (x + w * 0.28f, y + h * 0.35f);
    speaker.lineTo          (x + w * 0.55f, y + h * 0.10f);
    speaker.lineTo          (x + w * 0.55f, y + h * 0.90f);
    speaker.lineTo          (x + w * 0.28f, y + h * 0.65f);
    speaker.lineTo          (x,            y + h * 0.65f);
    speaker.closeSubPath();

    g.setColour (juce::Colour (0xff2a2a2a));
    g.fillPath (speaker);

    if (kind == Kind::Solo)
    {
        // Ondes sonores (arcs a droite du haut-parleur)
        juce::Path waves;
        const float cx = x + w * 0.55f, cy = y + h * 0.5f;
        waves.addCentredArc (cx, cy, w * 0.28f, h * 0.28f, 0.0f, 1.0f, 2.14f, true);
        waves.addCentredArc (cx, cy, w * 0.50f, h * 0.45f, 0.0f, 0.9f, 2.24f, true);
        g.strokePath (waves, juce::PathStrokeType (1.3f));
    }
    else
    {
        // Croix : son coupe
        const float cx = x + w * 0.76f, cy = y + h * 0.5f, r = w * 0.17f;
        g.drawLine (cx - r, cy - r, cx + r, cy + r, 1.6f);
        g.drawLine (cx - r, cy + r, cx + r, cy - r, 1.6f);
    }
}

// ---------------------------------------------------------------------------
// EfxIndicator
// ---------------------------------------------------------------------------

EfxIndicator::EfxIndicator()
{
    setInterceptsMouseClicks (false, false);
}

void EfxIndicator::setLit (bool shouldBeLit)
{
    if (lit != shouldBeLit)
    {
        lit = shouldBeLit;
        repaint();
    }
}

void EfxIndicator::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat().reduced (1.0f);
    const float d = juce::jmin (area.getWidth(), area.getHeight());
    const auto face = juce::Rectangle<float> (d, d).withCentre (area.getCentre());

    const juce::Colour base = lit ? juce::Colour (0xffff3b4a) : juce::Colour (0xffe9a3ae);

    g.setGradientFill (juce::ColourGradient (base.brighter (0.5f), face.getX() + d * 0.3f, face.getY() + d * 0.25f,
                                             base.darker (0.4f), face.getRight(), face.getBottom(), true));
    g.fillEllipse (face);

    g.setColour (juce::Colour (0xff4a4a4a));
    g.drawEllipse (face, 1.2f);

    g.setColour (juce::Colour (0xff3a1a1e));
    g.setFont (juce::Font (juce::FontOptions (9.0f, juce::Font::bold)));
    g.drawText ("EFX", face, juce::Justification::centred);
}

// ---------------------------------------------------------------------------
// PartListView
// ---------------------------------------------------------------------------

PartListView::PartListView (SC55Processor& processorToUse)
    : processor (processorToUse)
{
    setOpaque (true);

    for (int part = 0; part < 16; ++part)
    {
        auto& row = rows[(size_t) part];

        row.solo.onClick = [this, part]
        {
            // Comme sur l'ecran de part : solos cumules si ALL est allume, sinon solo exclusif
            const bool additive = isAllActive && isAllActive();

            processor.toggleSolo (part, additive);
            refresh();
            if (onStatus)
                onStatus ("Part " + juce::String (part + 1) + " SOLO "
                              + (processor.isPartSoloed (part) ? "ON" : "OFF")
                              + (additive ? " (multiple)" : " (exclusif)"));
        };

        row.mute.onClick = [this, part]
        {
            processor.setPartMuted (part, ! processor.isPartMuted (part));
            refresh();
            if (onStatus)
                onStatus ("Part " + juce::String (part + 1) + " MUTE "
                              + (processor.isPartMuted (part) ? "ON" : "OFF"));
        };

        // Volume : meme comportement que les sliders de l'ecran de part (file d'envoi)
        SysExParamSlider::Config config;
        config.minValue       = 0;
        config.maxValue       = 127;
        config.defaultValue   = 100;
        config.style         = juce::Slider::LinearHorizontal;
        config.textBoxPosition = juce::Slider::TextBoxRight;
        config.textBoxWidth   = 50;
        config.textBoxHeight  = 20;

        row.level = std::make_unique<SysExParamSlider> (config, [this, part] (int value, bool immediate)
        {
            processor.setPartParam (part, SC55::PartLevel, value, immediate);
            if (onStatus)
                onStatus ("Part " + juce::String (part + 1) + " LEVEL = " + juce::String (value));
        });
        // Piste sombre et partie remplie (du minimum a la valeur) bien visible
        row.level->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0e0e0e));
        row.level->setColour (juce::Slider::trackColourId, kTrackFill);
        row.level->setColour (juce::Slider::textBoxBackgroundColourId, kOrange);
        row.level->setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
        row.level->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff000000));
        row.level->setColour (juce::Slider::textBoxHighlightColourId, juce::Colour (0xffffd9a0));

        row.edit.onClick = [this, part]
        {
            if (onEditRequested)
                onEditRequested (part);
        };

        addAndMakeVisible (row.solo);
        addAndMakeVisible (row.mute);
        addAndMakeVisible (row.efx);
        addAndMakeVisible (*row.level);
        addAndMakeVisible (row.edit);
    }

    closeButton.onClick = [this]
    {
        if (onCloseRequested)
            onCloseRequested();
    };
    addAndMakeVisible (closeButton);

    refresh();
}

juce::String PartListView::instrumentNameForPart (int part) const
{
    const int pc  = processor.getPartTonePc (part) + 1;
    const int cc0 = processor.getPartToneCc0 (part);

    return part == kDrumPart ? SC55::getDrumSetName (pc) : SC55::getInstrumentName (pc, cc0);
}

void PartListView::refresh()
{
    for (int part = 0; part < 16; ++part)
    {
        auto& row = rows[(size_t) part];

        row.level->setValueSilently (processor.getPartParam (part, SC55::PartLevel));
        row.solo.setLitState (processor.isPartSoloed (part) ? 1 : 0);

        // Mute en propre = allume ; part rendue muette par un solo = allume attenue
        row.mute.setLitState (processor.isPartMuted (part) ? 1 : (processor.isPartAudible (part) ? 0 : 2));

        row.efx.setLit (processor.isPartEfxModified (part));
        row.instrumentName = instrumentNameForPart (part);
    }

    repaint();
}

void PartListView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e1e));
    g.setColour (juce::Colour (0xff6a6a6a));
    g.drawRect (getLocalBounds(), 1);

    // En-tetes de colonnes
    g.setColour (juce::Colour (0xffe6e6e6));
    g.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::bold)));
    g.drawText ("PA",   kNumberX, 6, kNumberW, 18, juce::Justification::centred);
    g.drawText ("SOLO", kSoloX - 3, 6, kButtonSize + 6, 18, juce::Justification::centred);
    g.drawText ("MUTE", kMuteX - 3, 6, kButtonSize + 6, 18, juce::Justification::centred);
    g.drawText ("EFX",  kEfxX - 3,  6, kButtonSize + 6, 18, juce::Justification::centred);
    g.drawText ("INSTRUMENT", kNameX, 6, kNameW, 18, juce::Justification::centred);

    const int levelWidth = getWidth() - kRightMargin - kEditW - 8 - kLevelX;
    const auto levelHeader = juce::Rectangle<int> (kLevelX + (levelWidth - 60) / 2, 6, 60, 18);
    g.setColour (juce::Colour (0xff8a8a8a));
    g.fillRoundedRectangle (levelHeader.toFloat(), 3.0f);
    g.setColour (juce::Colour (0xff202020));
    g.drawText ("LEVEL", levelHeader, juce::Justification::centred);

    // Lignes
    const int top = kHeaderHeight + kTopPadding;
    const juce::Font nameFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::bold));

    for (int part = 0; part < 16; ++part)
    {
        const int y = top + part * kRowHeight;

        g.setColour (juce::Colour (0xff2e2e2e));
        g.drawHorizontalLine (y + kRowHeight - 1, 4.0f, (float) getWidth() - 4.0f);

        // Numero de part
        g.setColour (juce::Colours::black);
        g.fillRect (kNumberX, y + 2, kNumberW, kRowHeight - 4);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
        g.drawText (juce::String (part + 1).paddedLeft ('0', 2), kNumberX, y + 2, kNumberW, kRowHeight - 4,
                    juce::Justification::centred);

        // Nom de l'instrument (ecran orange)
        g.setColour (kOrange);
        g.fillRect (kNameX, y + 3, kNameW, kRowHeight - 6);
        g.setColour (juce::Colours::black);
        g.setFont (nameFont);
        g.drawText (rows[(size_t) part].instrumentName, kNameX + 4, y + 3, kNameW - 8, kRowHeight - 6,
                    juce::Justification::centredLeft, true);
    }
}

void PartListView::resized()
{
    const int top = kHeaderHeight + kTopPadding;
    const int editX = getWidth() - kRightMargin - kEditW;
    const int levelWidth = editX - 8 - kLevelX;

    for (int part = 0; part < 16; ++part)
    {
        auto& row = rows[(size_t) part];
        const int y = top + part * kRowHeight;
        const int buttonY = y + (kRowHeight - kButtonSize) / 2;

        row.solo.setBounds (kSoloX, buttonY, kButtonSize, kButtonSize);
        row.mute.setBounds (kMuteX, buttonY, kButtonSize, kButtonSize);
        row.efx.setBounds  (kEfxX,  buttonY, kButtonSize, kButtonSize);
        row.level->setBounds (kLevelX, y, levelWidth, kRowHeight);
        row.edit.setBounds (editX, y + 2, kEditW, kRowHeight - 4);
    }

    closeButton.setBounds (getWidth() - 28, 4, 22, 22);
}
