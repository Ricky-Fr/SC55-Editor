#pragma once

#include "ParamPanel.h"
#include "PluginProcessor.h"

class SystemView : public ParamPanel,
                   private juce::Timer
{
public:
    explicit SystemView (SC55Processor& processorToUse);
    ~SystemView() override;

    void resized() override;
    void refresh();

protected:
    Range getRange (ParamRef ref) const override;
    int   getDefaultRaw (ParamRef ref) const override;
    int   getRaw (ParamRef ref) const override;
    void  setRaw (ParamRef ref, int rawValue, bool immediate) override;

private:
    enum Source { GlobalSource = 0, SysSource = 1 };

    void timerCallback() override;

    SC55Processor& processor;
    juce::Label midiOutputLabel { "midiOutputLabel", "MIDI Output:" };
    juce::ComboBox midiOutputBox;
    juce::Label lastProgramChangeLabel;
    juce::Label versionLabel { "versionLabel", "" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemView)
};
