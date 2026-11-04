#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "SysExQueue.h"
#include "PartParams.h"
#include "MixState.h"
#include "PresetManager.h"

class SC55Processor : public juce::AudioProcessor
{
public:
    SC55Processor();
    ~SC55Processor() override;

    void setPartParam (int part, int paramId, int rawValue, bool immediate = false);
    int  getPartParam (int part, int paramId) const;

    void setPartAdvParam (int part, int advId, int rawValue, bool immediate = false);
    int  getPartAdvParam (int part, int advId) const;
    bool isDrumPart (int part) const;            

    void setPartCcParam (int part, int ccId, int rawValue, bool immediate = false);
    int  getPartCcParam (int part, int ccId) const;

    void setSysParam (int id, int rawValue, bool immediate = false);
    int  getSysParam (int id) const;

    void setPartInstrument (int part, int pc, int cc0, bool immediate = false);
    int  getPartTonePc  (int part) const;
    int  getPartToneCc0 (int part) const;

    void setGlobalParam (int paramId, int rawValue, bool immediate = false);
    int  getGlobalParam (int paramId) const;

    void sendGsReset();
    void resendAll (bool onlyChanged = false);

    void setMidiOutputDeviceName (const juce::String& deviceName);
    juce::String getMidiOutputDeviceName() const { return midiOutputDeviceName; }
    
    // --- Lecture du dernier Key Switch / Preset (Thread-safe) ---
    int getLastKeySwitchNote() const { return lastKeySwitchNote.load (std::memory_order_relaxed); }
    int getLastPresetIndex() const   { return lastPresetIndex.load (std::memory_order_relaxed); }
    juce::uint64 getKeySwitchSequence() const { return keySwitchSequence.load (std::memory_order_relaxed); }
    juce::uint64 getPresetLoadSequence() const { return presetLoadSequence.load (std::memory_order_relaxed); }
    
    void updateDirectMidiOutput();

    void setPartMuted (int part, bool shouldBeMuted);
    bool isPartMuted (int part) const;
    void setAllMuted (bool shouldBeMuted);       
    bool areAllMuted() const;
    bool isPartAudible (int part) const;         

    void toggleSolo (int part, bool additive);
    bool isPartSoloed (int part) const;

    bool isPartEfxModified (int part) const;

    void runWhenSendingFinished (std::function<void()> callback);

    PresetManager& getPresetManager()                  { return presetManager; }
    const juce::String& getCurrentPresetName() const   { return currentPresetName; }

    bool loadPreset (const juce::String& name, bool sendToDevice);
    bool saveCurrentAsPreset (const juce::String& name, juce::String& error);
    bool deletePreset (const juce::String& name);

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    struct Tone
    {
        int cc0 = 0;   
        int pc  = 0;   
    };

    bool queueDt1 (juce::uint8 a1, juce::uint8 a2, juce::uint8 a3,
                   const std::vector<juce::uint8>& data,
                   bool immediate = false, int quietAfterMs = 0);

    bool sendPartCc (int part, int ccId, bool immediate);
    bool queueControlChange (int channel, int controller, int value, bool immediate);
    bool sendSysParam (int id, bool immediate);
    void resendEffectDependents (bool reverb);

    void pushToAudioThread (const juce::MidiMessage& message);

    static int partToBlock (int part);

    void applyMixState (bool force = false);       
    void sendPartRxNote (int part, bool audible);
    void silenceHangingNotes (int part);

    std::unique_ptr<juce::XmlElement> createStateXml (bool includeDeviceId) const;
    void applyStateXml (const juce::XmlElement& xml, bool includeDeviceId);
    static std::unique_ptr<juce::XmlElement> createDefaultStateXml();
    bool applyPresetByName (const juce::String& name);

    juce::uint8 deviceId = 0x10;                    

    std::array<std::array<int, SC55::NumPartParams>, 16> partValues;
    std::array<std::array<int, SC55::NumAdvParams>, 16> partAdvValues;
    std::array<std::array<int, SC55::NumCcParams>, 16> partCcValues;
    std::array<Tone, 16> partTones;
    std::array<int, SC55::NumPartParams> globalValues;
    std::array<int, SC55::NumSysParams> sysValues;

    SC55::MixState mix;

    bool skipUnchanged = false;
    std::unordered_map<juce::uint32, std::vector<juce::uint8>> sentShadow;

    PresetManager presetManager;
    juce::String currentPresetName;

    juce::String midiOutputDeviceName;
    std::unique_ptr<juce::MidiOutput> directMidiOutput;

    juce::MidiBuffer pending;                       
    juce::SpinLock pendingLock;

    // --- Suivi du Key Switch (Canal 16, C-2 = Note 0) ---
    std::atomic<int> lastKeySwitchNote { -1 };
    std::atomic<int> lastPresetIndex   { -1 };
    std::atomic<juce::uint64> keySwitchSequence { 0 };
    std::atomic<juce::uint64> presetLoadSequence { 0 };

    SysExQueue sysexQueue;                          

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SC55Processor)
};