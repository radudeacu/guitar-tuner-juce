#include "ReferenceTonePlayer.h"

void ReferenceTonePlayer::prepare (double sampleRate, int /*maximumBlockSize*/)
{
    const float startingFrequency = requestedFrequencyHz.load (std::memory_order_relaxed);

    string.prepare (sampleRate);
    string.setFrequency (startingFrequency);
    appliedFrequencyHz = startingFrequency;

    fadeGain.reset (sampleRate, retriggerFadeSeconds);
    fadeGain.setCurrentAndTargetValue (1.0f);
    fadingBeforeRetrigger = false;
}

void ReferenceTonePlayer::setFrequency (double frequencyHz)
{
    if (frequencyHz > 0.0)
        requestedFrequencyHz.store ((float) frequencyHz, std::memory_order_relaxed);
}

void ReferenceTonePlayer::pluck()
{
    pluckRequested.store (true, std::memory_order_relaxed);
}

void ReferenceTonePlayer::setLooping (bool shouldLoop)
{
    looping.store (shouldLoop, std::memory_order_relaxed);
}

void ReferenceTonePlayer::applyPendingChanges()
{
    const float frequency = requestedFrequencyHz.load (std::memory_order_relaxed);

    if (! juce::approximatelyEqual (frequency, appliedFrequencyHz))
    {
        appliedFrequencyHz = frequency;
        string.setFrequency (frequency);
    }

    if (! pluckRequested.exchange (false, std::memory_order_relaxed))
        return;

    if (string.isRinging())
    {
        // Interrupting a sounding note would step the output to zero, so duck away first.
        fadeGain.setTargetValue (0.0f);
        fadingBeforeRetrigger = true;
    }
    else
    {
        fadeGain.setCurrentAndTargetValue (1.0f);
        string.pluck();
    }
}

void ReferenceTonePlayer::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    applyPendingChanges();

    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        if (fadingBeforeRetrigger)
        {
            if (fadeGain.getCurrentValue() <= 0.0f)
            {
                string.pluck();
                fadeGain.setCurrentAndTargetValue (1.0f);
                fadingBeforeRetrigger = false;
            }
        }
        else if (looping.load (std::memory_order_relaxed) && ! string.isRinging())
        {
            // The previous note has fully decayed, so the repeat needs no fade.
            string.pluck();
        }

        const float sample = string.renderSample() * fadeGain.getNextValue() * outputGain;

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.addSample (channel, startSample + i, sample);
    }
}
