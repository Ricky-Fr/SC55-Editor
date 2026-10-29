#pragma once

#include <functional>
#include <vector>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include "SC55Config.h"

// File d'envoi de messages SysEx.
//  - espace les messages (SC55Config::kMinIntervalMs)
//  - fusionne les messages portant sur le meme parametre : seule la derniere
//    valeur est envoyee
//  - limite le debit d'un meme parametre pendant un glissement
//
// A utiliser depuis le thread de messages uniquement (interface graphique).
class SysExQueue : private juce::Timer
{
public:
    // Appelee pour chaque message a envoyer reellement.
    using Sink = std::function<void (const juce::MidiMessage&)>;

    explicit SysExQueue (Sink sinkFunction);
    ~SysExQueue() override;

    // key         : identifie le parametre (ex. son adresse sur 3 octets)
    // immediate   : true = ne pas attendre la limite de debit du parametre
    //               (fin de glissement, GS Reset, renvoi de l'etat...)
    // quietAfterMs: pause supplementaire apres l'envoi de ce message
    void enqueue (juce::uint32 key, const juce::MidiMessage& message,
                  bool immediate = false, int quietAfterMs = 0);

    // Appelle le callback une seule fois, quand tous les messages en attente ont ete envoyes
    void runWhenEmpty (std::function<void()> callback);

    void clear();

private:
    struct Entry
    {
        juce::uint32 key = 0;
        juce::MidiMessage message;
        juce::uint32 order = 0;          // ordre d'arrivee (FIFO)
        juce::uint32 lastChangeMs = 0;   // dernier changement de valeur
        juce::uint32 lastSentMs = 0;     // dernier envoi de ce parametre
        int quietAfterMs = 0;
        bool hasSent = false;
        bool dirty = false;              // valeur en attente d'envoi
        bool urgent = false;
    };

    void timerCallback() override;

    Sink sink;
    std::function<void()> emptyCallback;
    std::vector<Entry> entries;
    juce::uint32 sequence = 0;

    bool anySent = false;
    juce::uint32 lastSendMs = 0;
    juce::uint32 gapMs = (juce::uint32) SC55Config::kMinIntervalMs;
};
