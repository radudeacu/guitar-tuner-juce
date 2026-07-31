#include "ReferenceTonePlayer.h"

ReferenceTonePlayer::ReferenceTonePlayer()
{
    // Lookup table rather than a live std::sin call, so rendering stays cheap on the audio thread.
    oscillator.initialise ([] (float phase) { return std::sin (phase); }, 512);
}

void ReferenceTonePlayer::prepare (double sampleRate, int maximumBlockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax (1, maximumBlockSize);
    spec.numChannels = 1;

    oscillator.prepare (spec);
    oscillator.setFrequency (requestedFrequencyHz.load (std::memory_order_relaxed), true);
    appliedFrequencyHz = requestedFrequencyHz.load (std::memory_order_relaxed);

    gain.reset (sampleRate, gainRampSeconds);
    gain.setCurrentAndTargetValue (0.0f);
}

void ReferenceTonePlayer::setFrequency (double frequencyHz)
{
    if (frequencyHz > 0.0)
        requestedFrequencyHz.store ((float) frequencyHz, std::memory_order_relaxed);
}

void ReferenceTonePlayer::setPlaying (bool shouldPlay)
{
    requestedPlaying.store (shouldPlay, std::memory_order_relaxed);
}

void ReferenceTonePlayer::applyPendingChanges()
{
    const float frequency = requestedFrequencyHz.load (std::memory_order_relaxed);

    if (! juce::approximatelyEqual (frequency, appliedFrequencyHz))
    {
        appliedFrequencyHz = frequency;

        // Not forced: the oscillator glides to the new frequency, so switching strings
        // mid-tone bends rather than clicks.
        oscillator.setFrequency (frequency);
    }

    gain.setTargetValue (requestedPlaying.load (std::memory_order_relaxed) ? toneAmplitude : 0.0f);
}

void ReferenceTonePlayer::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    applyPendingChanges();

    // Silent and not ramping — skip the whole block rather than adding zeroes.
    if (! gain.isSmoothing() && gain.getCurrentValue() <= 0.0f)
        return;

    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        const float sample = oscillator.processSample (0.0f) * gain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.addSample (channel, startSample + i, sample);
    }
}
