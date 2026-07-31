#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include <atomic>

/*  Generates the sine tone used for tuning by ear.

    Control methods are called from the message thread and the render method from the audio
    thread, so requested changes are handed over through atomics and applied at the start of
    the next block. Nothing here allocates or locks once prepare() has run.
*/
class ReferenceTonePlayer
{
public:
    ReferenceTonePlayer();

    /** Must be called from prepareToPlay — allocates the oscillator's ramp buffer. */
    void prepare (double sampleRate, int maximumBlockSize);

    // --- message thread ---
    void setFrequency (double frequencyHz);
    void setPlaying (bool shouldPlay);

    /** Adds the tone into the buffer. Audio thread only. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

private:
    void applyPendingChanges();

    // Quiet enough to sit under a plucked string rather than drown it.
    static constexpr float toneAmplitude = 0.2f;
    static constexpr double gainRampSeconds = 0.03;

    juce::dsp::Oscillator<float> oscillator;
    juce::SmoothedValue<float> gain;

    std::atomic<float> requestedFrequencyHz { 440.0f };
    std::atomic<bool> requestedPlaying { false };

    float appliedFrequencyHz = 0.0f; // audio thread only
};
