#include "SysExQueue.h"

SysExQueue::SysExQueue (Sink sinkFunction)
    : sink (std::move (sinkFunction))
{
}

SysExQueue::~SysExQueue()
{
    stopTimer();
}

void SysExQueue::clear()
{
    entries.clear();
    stopTimer();
}

void SysExQueue::enqueue (juce::uint32 key, const juce::MidiMessage& message,
                          bool immediate, int quietAfterMs)
{
    const auto now = juce::Time::getMillisecondCounter();

    Entry* e = nullptr;
    for (auto& it : entries)
        if (it.key == key) { e = &it; break; }

    if (e == nullptr)
    {
        entries.emplace_back();
        e = &entries.back();
        e->key = key;
    }

    if (! e->dirty)               // nouvelle valeur apres un envoi : nouvelle place dans la file
        e->order = ++sequence;

    e->message      = message;    // ecrase la valeur precedente non envoyee
    e->lastChangeMs = now;
    e->dirty        = true;
    e->urgent       = e->urgent || immediate;
    e->quietAfterMs = quietAfterMs;

    if (! isTimerRunning())
        startTimer (SC55Config::kQueueTickMs);
}

void SysExQueue::timerCallback()
{
    using namespace SC55Config;

    const auto now = juce::Time::getMillisecondCounter();

    bool anyDirty = false;
    Entry* best = nullptr;

    for (auto& e : entries)
    {
        if (! e.dirty)
            continue;

        anyDirty = true;

        const bool idle   = (now - e.lastChangeMs) >= (juce::uint32) kDragEndTimeoutMs;
        const bool rateOk = ! e.hasSent || (now - e.lastSentMs) >= (juce::uint32) kDragSendIntervalMs;

        if (! (e.urgent || idle || rateOk))
            continue;

        if (best == nullptr || (juce::int32) (e.order - best->order) < 0)
            best = &e;
    }

    if (! anyDirty)
    {
        stopTimer();
        return;
    }

    // Espacement entre deux messages quelconques
    if (anySent && (now - lastSendMs) < gapMs)
        return;

    if (best == nullptr)
        return;

    if (sink)
        sink (best->message);

    best->dirty      = false;
    best->urgent     = false;
    best->hasSent    = true;
    best->lastSentMs = now;

    anySent    = true;
    lastSendMs = now;
    gapMs      = (juce::uint32) juce::jmax (kMinIntervalMs, best->quietAfterMs);
}
