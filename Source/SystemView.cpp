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

    // Zone 0 : System
    addSliderRow (0, "Pan",       { GlobalSource, PartPan },      ParamFormat::Pan,       0, "System Pan");
    addSliderRow (0, "Key Shift", { GlobalSource, PartKeyShift }, ParamFormat::Signed,    0, "System Key Shift");
    addSliderRow (0, "Tune",      { SysSource,    SysMasterTune }, ParamFormat::TuneCents, 0, "System Tune");

    // Zone 1 : Reverb
    addComboRow  (1, "Type", { SysSource, SysReverbType },
                  { "Room 1", "Room 2", "Room 3", "Hall 1", "Hall 2", "Plate", "Delay", "Panning Delay" },
                  "Reverb Type");
    addComboRow  (1, "Character", { SysSource, SysReverbCharacter },
                  { "Room 1", "Room 2", "Room 3", "Hall 1", "Hall 2", "Plate", "Delay", "Panning Delay" },
                  "Reverb Character");
    addSliderRow (1, "Level",          { GlobalSource, PartReverb },           ParamFormat::Plain, 0, "Reverb Level");
    addSliderRow (1, "Time",           { SysSource,    SysReverbTime },        ParamFormat::Plain, 0, "Reverb Time");
    addSliderRow (1, "Delay Feedback", { SysSource,    SysReverbDelayFeedback }, ParamFormat::Plain, 0, "Reverb Delay Feedback");
    addSliderRow (1, "Send to chorus", { SysSource,    SysReverbSendToChorus }, ParamFormat::Plain, 0, "Reverb Send to chorus");

    // Zone 2 : Chorus
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

    // Zone 3 : Transmission (Quadrant inférieur droit)
    midiOutputLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd0d0d0));
    midiOutputLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    addAndMakeVisible (midiOutputLabel);

    lastProgramChangeLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd0d0d0));
    lastProgramChangeLabel.setFont (juce::Font (juce::FontOptions (12.0f)));
    addAndMakeVisible (lastProgramChangeLabel);

    versionLabel.setColour (juce::Label::textColourId, juce::Colour (0xffa0a0a0));
    versionLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    versionLabel.setText ("Version " + juce::String (JucePlugin_VersionString),
                          juce::dontSendNotification);
    addAndMakeVisible (versionLabel);
    startTimerHz (10);

    midiOutputBox.onChange = [this]
    {
        const int id = midiOutputBox.getSelectedId();
        if (id == 1)
        {
            processor.setMidiOutputDeviceName ({});
        }
        else if (id > 1)
        {
            const auto deviceName = midiOutputBox.getItemText (midiOutputBox.getSelectedItemIndex());
            processor.setMidiOutputDeviceName (deviceName);
        }
    };
    addAndMakeVisible (midiOutputBox);

    addNote (3, "Bypass Ableton SysEx Filter: select the virtual MIDI output port "
                "(e.g., loopMIDI) used to send SysEx directly to the hardware/emulator.");

    refresh();
}

SystemView::~SystemView() = default;

void SystemView::timerCallback()
{
    const int note = processor.getLastKeySwitchNote();
    const int presetIdx = processor.getLastPresetIndex();

    juce::String text = "Last key : aucune";

    if (note >= 0)
    {
        const auto noteName = juce::MidiMessage::getMidiNoteName (note, true, true, 3);
        text = "Last key : Note " + juce::String (note) + " (" + noteName + ")";

        if (presetIdx >= 0)
        {
            const auto names = processor.getPresetManager().getPresetNames();
            if (presetIdx < names.size())
                text += " -> Preset " + juce::String (presetIdx) + " (" + names[presetIdx] + ")";
            else
                text += " -> no preset";
        }
        else
        {
            text += " -> no preset";
        }
    }

    lastProgramChangeLabel.setText (text, juce::dontSendNotification);
}

void SystemView::resized()
{
    ParamPanel::resized();

    const int zoneX = getWidth() / 2;
    const int rowY = kHSplit + 108;
    const int labelX = zoneX + 16;
    const int labelWidth = 110;
    const int boxX = labelX + labelWidth + 8;

    midiOutputLabel.setBounds (labelX, rowY, labelWidth, 24);
    midiOutputBox.setBounds (boxX, rowY, getWidth() - boxX - 16, 24);
    lastProgramChangeLabel.setBounds (labelX, rowY + 30, getWidth() - labelX - 16, 20);
    versionLabel.setBounds (labelX, rowY + 54, getWidth() - labelX - 16, 18);
}

void SystemView::refresh()
{
    ParamPanel::refresh();

    midiOutputBox.clear (juce::dontSendNotification);
    midiOutputBox.addItem ("-- Aucun (Defaut DAW) --", 1);

    const auto devices = juce::MidiOutput::getAvailableDevices();
    int id = 2;
    for (const auto& dev : devices)
    {
        midiOutputBox.addItem (dev.name, id++);
    }

    const auto activeDevice = processor.getMidiOutputDeviceName();
    if (activeDevice.isEmpty())
    {
        midiOutputBox.setSelectedId (1, juce::dontSendNotification);
    }
    else
    {
        int selectedId = 1;
        int deviceId = 2;
        for (const auto& dev : devices)
        {
            if (dev.name == activeDevice)
            {
                selectedId = deviceId;
                break;
            }
            ++deviceId;
        }

        midiOutputBox.setSelectedId (selectedId, juce::dontSendNotification);
    }
}

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