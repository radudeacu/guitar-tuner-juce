#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <atomic>

#include "PluckedString.h"

/*  Plays the reference pitch as a plucked guitar note.

    Control methods are called from the message thread and the render method from the audio
    thread, so requests are handed over through atomics and applied on the audio thread.
    Nothing here allocates or locks once prepare() has run.
*/
class ReferenceTonePlayer
{
public:
    void prepare (double sampleRate, int maximumBlockSize);

    // --- message thread ---

    /** Takes effect on the next pluck, so retuning never bends a ringing note. */
    void setFrequency (double frequencyHz);

    /** Sounds a note at the most recently set frequency. */
    void pluck();

    /** When looping, a note re-plucks itself as soon as the previous one has decayed. */
    void setLooping (bool shouldLoop);

    /** Adds the note into the buffer. Audio thread only. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

private:
    void applyPendingChanges();

    // Loud enough to hear against a plucked string without dominating it.
    static constexpr float outputGain = 0.45f;

    // Long enough to swallow the discontinuity of interrupting a ringing note, short enough
    // that the delay before the new note is imperceptible.
    static constexpr double retriggerFadeSeconds = 0.008;

    PluckedString string;
    juce::SmoothedValue<float> fadeGain;

    std::atomic<float> requestedFrequencyHz { 440.0f };
    std::atomic<bool> pluckRequested { false };
    std::atomic<bool> looping { false };

    // audio thread only
    float appliedFrequencyHz = 0.0f;
    bool fadingBeforeRetrigger = false;
};
