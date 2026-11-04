#include "AdvancedView.h"

namespace
{
    constexpr int kHSplit = 276;    // filet horizontal entre les zones du haut et du bas
}

void AdsrGraph::setValues (float newAttack, float newDecay, float newRelease)
{
    attack = juce::jlimit (0.0f, 1.0f, newAttack);
    decay = juce::jlimit (0.0f, 1.0f, newDecay);
    release = juce::jlimit (0.0f, 1.0f, newRelease);
    repaint();
}

void AdsrGraph::setVelocityValues (float newDepth, float newOffset)
{
    velocityDepth = juce::jlimit (0.0f, 1.0f, newDepth);
    velocityOffset = juce::jlimit (0.0f, 1.0f, newOffset);
    repaint();
}

void AdsrGraph::setVelocityMode (bool shouldShowVelocity)
{
    if (showingVelocity != shouldShowVelocity)
    {
        showingVelocity = shouldShowVelocity;
        repaint();
    }
}

void AdsrGraph::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (16.0f, 12.0f);
    const auto graph = bounds.withTrimmedTop (24.0f).withTrimmedBottom (18.0f);

    g.setColour (juce::Colour (0xff15191b));
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (juce::Colour (0xffd8d8d8));
    g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
    g.drawText (showingVelocity ? "VELOCITY" : "ADSR",
                juce::roundToInt (bounds.getX() + 8.0f), juce::roundToInt (bounds.getY() + 4.0f),
                juce::roundToInt (bounds.getWidth() - 16.0f), 18, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff3d464a));
    g.drawHorizontalLine (juce::roundToInt (graph.getBottom()), graph.getX(), graph.getRight());
    g.drawVerticalLine (juce::roundToInt (graph.getX()), graph.getY(), graph.getBottom());

    if (showingVelocity)
    {
        juce::Path velocityCurve;
        constexpr int sampleCount = 64;

        for (int sample = 0; sample <= sampleCount; ++sample)
        {
            const float x = (float) sample / (float) sampleCount;
            const float output = juce::jlimit (0.0f, 1.0f,
                                               velocityOffset + velocityDepth * 2.0f * (x - 0.5f));
            const float px = graph.getX() + graph.getWidth() * x;
            const float py = graph.getBottom() - graph.getHeight() * output;

            if (sample == 0)
                velocityCurve.startNewSubPath (px, py);
            else
                velocityCurve.lineTo (px, py);
        }

        g.setColour (juce::Colour (0xff70c8d8));
        g.strokePath (velocityCurve, juce::PathStrokeType (2.0f));

        g.setFont (juce::Font (juce::FontOptions (10.0f)));
        g.setColour (juce::Colour (0xffaeb7ba));
        g.drawText ("1", juce::roundToInt (graph.getX() - 4.0f), juce::roundToInt (graph.getBottom() + 3.0f),
                    14, 14, juce::Justification::centred);
        g.drawText ("64", juce::roundToInt (graph.getCentreX() - 8.0f), juce::roundToInt (graph.getBottom() + 3.0f),
                    16, 14, juce::Justification::centred);
        g.drawText ("127", juce::roundToInt (graph.getRight() - 10.0f), juce::roundToInt (graph.getBottom() + 3.0f),
                    20, 14, juce::Justification::centred);
        return;
    }

    constexpr float sustainLevel = 0.75f;
    constexpr float keyOffTime = 0.5f;
    const float total = attack + decay + keyOffTime + release;
    const float attackWidth = graph.getWidth() * attack / total;
    const float decayWidth = graph.getWidth() * decay / total;
    const float sustainWidth = graph.getWidth() * keyOffTime / total;
    const float releaseWidth = graph.getWidth() * release / total;
    const float peakY = graph.getY() + 2.0f;
    const float baseY = graph.getBottom();
    const float sustainY = baseY - graph.getHeight() * sustainLevel;

    juce::Path envelope;
    envelope.startNewSubPath (graph.getX(), baseY);
    envelope.lineTo (graph.getX() + attackWidth, peakY);
    envelope.lineTo (graph.getX() + attackWidth + decayWidth, sustainY);
    envelope.lineTo (graph.getX() + attackWidth + decayWidth + sustainWidth, sustainY);
    envelope.lineTo (graph.getRight(), baseY);

    g.setColour (juce::Colour (0xff70c8d8));
    g.strokePath (envelope, juce::PathStrokeType (2.0f));

    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.setColour (juce::Colour (0xffaeb7ba));
    const int labelY = juce::roundToInt (graph.getBottom() + 3.0f);
    g.drawText ("A", juce::roundToInt (graph.getX()), labelY, juce::roundToInt (attackWidth), 14, juce::Justification::centred);
    g.drawText ("D", juce::roundToInt (graph.getX() + attackWidth), labelY, juce::roundToInt (decayWidth), 14, juce::Justification::centred);
    g.drawText ("S", juce::roundToInt (graph.getX() + attackWidth + decayWidth), labelY, juce::roundToInt (sustainWidth), 14, juce::Justification::centred);
    g.drawText ("R", juce::roundToInt (graph.getRight() - releaseWidth), labelY, juce::roundToInt (releaseWidth), 14, juce::Justification::centred);
}

AdvancedView::AdvancedView (SC55Processor& processorToUse)
    : ParamPanel ({ "Tone Modify", "Part Setup", "", "Pitch" }, kHSplit),
      processor (processorToUse)
{
    addAndMakeVisible (adsrGraph);

    envelopeButton.setClickingTogglesState (true);
    envelopeButton.setToggleState (true, juce::dontSendNotification);
    envelopeButton.setTooltip ("Envelope");
    envelopeButton.onClick = [this]
    {
        envelopeButton.setToggleState (true, juce::dontSendNotification);
        velocityButton.setToggleState (false, juce::dontSendNotification);
        adsrGraph.setVelocityMode (false);
    };

    velocityButton.setClickingTogglesState (true);
    velocityButton.setTooltip ("Velocity");
    velocityButton.onClick = [this]
    {
        velocityButton.setToggleState (true, juce::dontSendNotification);
        envelopeButton.setToggleState (false, juce::dontSendNotification);
        adsrGraph.setVelocityMode (true);
    };

    addAndMakeVisible (envelopeButton);
    addAndMakeVisible (velocityButton);

    using namespace SC55;

    // Zone 0 : Tone Modify
    addSliderRow (0, "Vibrato Rate",     { AdvSource, AdvVibratoRate },     ParamFormat::Signed);
    addSliderRow (0, "Vibrato Depth",    { AdvSource, AdvVibratoDepth },    ParamFormat::Signed);
    addSliderRow (0, "Vibrato Delay",    { AdvSource, AdvVibratoDelay },    ParamFormat::Signed);
    addSliderRow (0, "Modulation Depth", { AdvSource, AdvModulationDepth }, ParamFormat::Plain,  kGroupGap);
    addSliderRow (0, "Cutoff",           { AdvSource, AdvCutoff },          ParamFormat::Signed, kGroupGap);
    addSliderRow (0, "Resonance",        { AdvSource, AdvResonance },       ParamFormat::Signed);
    addSliderRow (0, "Attack",           { AdvSource, AdvAttack },          ParamFormat::Signed, kGroupGap);
    addSliderRow (0, "Decay",            { AdvSource, AdvDecay },           ParamFormat::Signed);
    addSliderRow (0, "Release",          { AdvSource, AdvRelease },         ParamFormat::Signed);

    // Zone 1 : Part Setup
    addComboRow  (1, "Part Mode",       { AdvSource, AdvPartMode }, { "Normal", "Drum Map 1", "Drum Map 2" });
    addSliderRow (1, "Velocity Depth",  { AdvSource, AdvVelocityDepth },  ParamFormat::Plain);
    addSliderRow (1, "Velocity Offset", { AdvSource, AdvVelocityOffset }, ParamFormat::Plain);
    addSliderRow (1, "Key Range Low",   { AdvSource, AdvKeyRangeLow },    ParamFormat::Note);
    addSliderRow (1, "Key Range High",  { AdvSource, AdvKeyRangeHigh },   ParamFormat::Note);
    addComboRow  (1, "Rx RPN",          { AdvSource, AdvRxRpn },  { "Off", "On" });
    addComboRow  (1, "Rx NRPN",         { AdvSource, AdvRxNrpn }, { "Off", "On" });

    // Portamento : envoye en Control Change (CC#65 et CC#5) sur le canal de la part
    addComboRow  (1, "Portamento",      { CcSource, CcPortamentoSwitch }, { "Off", "On" });
    addSliderRow (1, "Portamento Time", { CcSource, CcPortamentoTime },   ParamFormat::Plain);

    // Zone 3 : Pitch (Key Shift et Bend Range sont les memes reglages que sur l'ecran de part)
    addSliderRow (3, "Key Shift",   { MainSource, PartKeyShift },  ParamFormat::Signed);
    addSliderRow (3, "Fine Tune",   { AdvSource,  AdvFineTune },   ParamFormat::FineHz);
    addSliderRow (3, "Bend Range",  { MainSource, PartBendRange }, ParamFormat::Semitones);
    addComboRow  (3, "Mono / Poly", { AdvSource,  AdvMonoPoly },   { "Mono", "Poly" });

    setPart (0);
}

AdvancedView::~AdvancedView() = default;

void AdvancedView::setPart (int newPart)
{
    part = juce::jlimit (0, 15, newPart);
    setHeaderText ("Part " + juce::String (part + 1));
    refresh();
    updateAdsrGraph();
}

juce::String AdvancedView::getStatusPrefix() const
{
    return "Part " + juce::String (part + 1) + " ";
}

ParamPanel::Range AdvancedView::getRange (ParamRef ref) const
{
    if (ref.source == MainSource)
    {
        const auto& info = SC55::getPartParamInfo (ref.id);
        return { info.rawMin, info.rawMax, info.displayOffset };
    }

    if (ref.source == CcSource)
    {
        const auto& info = SC55::getCcParamInfo (ref.id);
        return { info.rawMin, info.rawMax, 0 };
    }

    const auto& info = SC55::getAdvParamInfo (ref.id);
    return { info.rawMin, info.rawMax, info.displayOffset };
}

int AdvancedView::getDefaultRaw (ParamRef ref) const
{
    if (ref.source == CcSource)
        return SC55::getCcParamInfo (ref.id).rawDefault;

    return ref.source == MainSource ? SC55::getDefaultRaw (SC55::getPartParamInfo (ref.id), part)
                                    : SC55::getAdvDefaultRaw (SC55::getAdvParamInfo (ref.id), part);
}

int AdvancedView::getRaw (ParamRef ref) const
{
    if (ref.source == CcSource)
        return processor.getPartCcParam (part, ref.id);

    return ref.source == MainSource ? processor.getPartParam (part, ref.id)
                                    : processor.getPartAdvParam (part, ref.id);
}

void AdvancedView::setRaw (ParamRef ref, int rawValue, bool immediate)
{
    if (ref.source == MainSource)
        processor.setPartParam (part, ref.id, rawValue, immediate);
    else if (ref.source == CcSource)
        processor.setPartCcParam (part, ref.id, rawValue, immediate);
    else
        processor.setPartAdvParam (part, ref.id, rawValue, immediate);

    updateAdsrGraph();
}

void AdvancedView::updateAdsrGraph()
{
    const auto normalise = [this] (int id)
    {
        const auto info = SC55::getAdvParamInfo (id);
        const float range = (float) (info.rawMax - info.rawMin);
        return range > 0.0f ? (float) (processor.getPartAdvParam (part, id) - info.rawMin) / range : 0.0f;
    };

    adsrGraph.setValues (normalise (SC55::AdvAttack),
                         normalise (SC55::AdvDecay),
                         normalise (SC55::AdvRelease));

    adsrGraph.setVelocityValues (juce::jlimit (0.0f, 1.0f,
                                               processor.getPartAdvParam (part, SC55::AdvVelocityDepth) / 127.0f),
                                 juce::jlimit (0.0f, 1.0f,
                                               processor.getPartAdvParam (part, SC55::AdvVelocityOffset) / 127.0f));
}

void AdvancedView::resized()
{
    ParamPanel::resized();

    const int leftWidth = getWidth() / 2;
    adsrGraph.setBounds (10, kHSplit + 10, leftWidth - 52, getHeight() - kHSplit - 20);
    envelopeButton.setBounds (leftWidth - 36, kHSplit + 10, 24, 24);
    velocityButton.setBounds (leftWidth - 36, kHSplit + 40, 24, 24);
}
