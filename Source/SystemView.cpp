#include "SystemView.h"

namespace
{
    constexpr int kHSplit = 196;    // filet horizontal entre les zones du haut et du bas
}

SystemView::SystemView (SC55Processor& processorToUse)
    : ParamPanel ({ "System", "Reverb", "Chorus", "Transmission" }, kHSplit),
      processor (processorToUse)
{
    using namespace SC55;

    // Zone 0 : System (pan et transposition generaux : memes reglages que les commandes GLOBAL)
    addSliderRow (0, "Pan",       { GlobalSource, PartPan },      ParamFormat::Pan,       0, "System Pan");
    addSliderRow (0, "Key Shift", { GlobalSource, PartKeyShift }, ParamFormat::Signed,    0, "System Key Shift");
    addSliderRow (0, "Tune",      { SysSource,    SysMasterTune }, ParamFormat::TuneCents, 0, "System Tune");

    // Zone 1 : Reverb (le niveau est le meme reglage que REVERB en mode GLOBAL)
    addComboRow  (1, "Type", { SysSource, SysReverbType },
                  { "Room 1", "Room 2", "Room 3", "Hall 1", "Hall 2", "Plate", "Delay", "Panning Delay" },
                  "Reverb Type");
    addComboRow  (1, "Character", { SysSource, SysReverbCharacter },
                  { "0", "1", "2", "3", "4", "5", "6", "7" }, "Reverb Character");
    addSliderRow (1, "Level",          { GlobalSource, PartReverb },           ParamFormat::Plain, 0, "Reverb Level");
    addSliderRow (1, "Time",           { SysSource,    SysReverbTime },        ParamFormat::Plain, 0, "Reverb Time");
    addSliderRow (1, "Delay Feedback", { SysSource,    SysReverbDelayFeedback }, ParamFormat::Plain, 0, "Reverb Delay Feedback");
    addSliderRow (1, "Send to chorus", { SysSource,    SysReverbSendToChorus }, ParamFormat::Plain, 0, "Reverb Send to chorus");

    // Zone 2 : Chorus (le niveau est le meme reglage que CHORUS en mode GLOBAL)
    addComboRow  (2, "Type", { SysSource, SysChorusType },
                  { "Chorus 1", "Chorus 2", "Chorus 3", "Chorus 4", "Feedback Chorus", "Flanger",
                    "Short Delay", "Short Delay (FB)" },
                  "Chorus Type");
    addSliderRow (2, "Pre-LPF",        { SysSource,    SysChorusPreLpf },      ParamFormat::Plain, 0, "Chorus Pre-LPF");
    addSliderRow (2, "Level",          { GlobalSource, PartChorus },           ParamFormat::Plain, 0, "Chorus Level");
    addSliderRow (2, "Feedback",       { SysSource,    SysChorusFeedback },    ParamFormat::Plain, 0, "Chorus Feedback");
    addSliderRow (2, "Delay",          { SysSource,    SysChorusDelay },       ParamFormat::Plain, 0, "Chorus Delay");
    addSliderRow (2, "Rate",           { SysSource,    SysChorusRate },        ParamFormat::Plain, 0, "Chorus Rate");
    addSliderRow (2, "Depth",          { SysSource,    SysChorusDepth },       ParamFormat::Plain, 0, "Chorus Depth");
    addSliderRow (2, "Send to reverb", { SysSource,    SysChorusSendToReverb }, ParamFormat::Plain, 0, "Chorus Send to reverb");

    // Zone 3 : Transmission. Ces reglages existent sur la facade du SC-55 (touche ALL), mais la
    // documentation ne leur donne aucune adresse SysEx : l'editeur ne peut pas les envoyer.
    addNote (3, "Front panel only: the SC-55 documentation gives no SysEx address for "
                "Rx Inst Chg, Rx GS Reset, Rx SysEx, Display, Peak Hold, LCD Contrast, Back Up "
                "and Rx Remote, so they cannot be set from the editor.");

    refresh();
}

SystemView::~SystemView() = default;

ParamPanel::Range SystemView::getRange (ParamRef ref) const
{
    if (ref.source == GlobalSource)
    {
        const auto& info = SC55::getPartParamInfo (ref.id);
        return { info.gRawMin, info.gRawMax, info.displayOffset };
    }

    const auto& info = SC55::getSysParamInfo (ref.id);
    return { info.rawMin, info.rawMax, info.displayOffset };
}

int SystemView::getDefaultRaw (ParamRef ref) const
{
    return ref.source == GlobalSource ? SC55::getPartParamInfo (ref.id).gRawDefault
                                      : SC55::getSysParamInfo (ref.id).rawDefault;
}

int SystemView::getRaw (ParamRef ref) const
{
    return ref.source == GlobalSource ? processor.getGlobalParam (ref.id)
                                      : processor.getSysParam (ref.id);
}

void SystemView::setRaw (ParamRef ref, int rawValue, bool immediate)
{
    if (ref.source == GlobalSource)
        processor.setGlobalParam (ref.id, rawValue, immediate);
    else
        processor.setSysParam (ref.id, rawValue, immediate);
}
