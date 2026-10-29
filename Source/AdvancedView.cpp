#include "AdvancedView.h"

namespace
{
    constexpr int kHSplit = 276;    // filet horizontal entre les zones du haut et du bas
}

AdvancedView::AdvancedView (SC55Processor& processorToUse)
    : ParamPanel ({ "Tone Modify", "Part Setup", "", "Pitch" }, kHSplit),
      processor (processorToUse)
{
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
}
