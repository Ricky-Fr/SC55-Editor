#pragma once

#include <array>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include "SysExQueue.h"
#include "PartParams.h"

// Plugin sans audio : il sert uniquement de telecommande MIDI/SysEx pour le Roland SC-55.
class SC55Processor : public juce::AudioProcessor
{
public:
    SC55Processor();
    ~SC55Processor() override = default;

    // --- API utilisee par l'interface (thread de messages uniquement) ---
    // immediate = true : envoi sans limite de debit (fin de glissement)

    // Parametre de part (rawValue = valeur envoyee au SC-55)
    void setPartParam (int part, int paramId, int rawValue, bool immediate = false);
    int  getPartParam (int part, int paramId) const;

    // Instrument d'une part : pc = Program Change (0..127), cc0 = variation (0..127)
    void setPartInstrument (int part, int pc, int cc0, bool immediate = false);
    int  getPartTonePc  (int part) const;
    int  getPartToneCc0 (int part) const;

    // Commande globale (voir SC55::GlobalKind)
    void setGlobalParam (int paramId, int rawValue, bool immediate = false);
    int  getGlobalParam (int paramId) const;

    void sendGsReset();
    void resendAll();                            // renvoie l'etat interne complet

    // --- AudioProcessor ---
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // Etat sauvegarde dans le projet de la DAW
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    struct Tone
    {
        int cc0 = 0;   // variation
        int pc  = 0;   // Program Change
    };

    // Met en file un message Roland DT1 (Data Set 1) : adresse sur 3 octets + donnees
    void queueDt1 (juce::uint8 a1, juce::uint8 a2, juce::uint8 a3,
                   const std::vector<juce::uint8>& data,
                   bool immediate = false, int quietAfterMs = 0);

    // Appelee par la file d'envoi : transmet le message au thread audio
    void pushToAudioThread (const juce::MidiMessage& message);

    static int partToBlock (int part);

    juce::uint8 deviceId = 0x10;                    // Device ID Roland (17 en facade par defaut)

    // Modele d'etat interne : valeurs envoyees au SC-55
    std::array<std::array<int, SC55::NumPartParams>, 16> partValues;
    std::array<Tone, 16> partTones;
    std::array<int, SC55::NumPartParams> globalValues;

    juce::MidiBuffer pending;                       // messages en attente pour le thread audio
    juce::SpinLock pendingLock;

    SysExQueue sysexQueue;                          // declaree apres pending : detruite avant

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SC55Processor)
};
