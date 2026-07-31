#include "PluckedString.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr double pi = 3.141592653589793;
    constexpr double twoPi = 6.283185307179586;

    // Keep partials clear of Nyquist; anything above would fold back as aliasing.
    constexpr double highestUsableFractionOfSampleRate = 0.45;
}

void PluckedString::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
    attackRate = (float) (1.0 / (attackSeconds * sampleRate));

    for (int k = 0; k < numHarmonics; ++k)
    {
        const double harmonic = k + 1.0;

        // Time constant shortens as the harmonic number rises.
        const double decaySeconds = fundamentalDecaySeconds / std::pow (harmonic, harmonicDecayExponent);
        decayPerSample[(size_t) k] = (float) std::exp (-1.0 / (decaySeconds * sampleRate));
    }

    updateHarmonicProfile();
    silence();
}

void PluckedString::setFrequency (double newFrequencyHz)
{
    if (newFrequencyHz <= 0.0)
        return;

    frequencyHz = newFrequencyHz;
    updateHarmonicProfile();
}

void PluckedString::updateHarmonicProfile()
{
    const double nyquistLimit = sampleRate * highestUsableFractionOfSampleRate;
    double totalAmplitude = 0.0;

    for (int k = 0; k < numHarmonics; ++k)
    {
        const double harmonic = k + 1.0;
        const double harmonicFrequency = frequencyHz * harmonic;

        if (harmonicFrequency >= nyquistLimit)
        {
            harmonicAmplitude[(size_t) k] = 0.0f;
            phaseIncrement[(size_t) k] = 0.0;
            continue;
        }

        // Ideal string plucked at pluckPosition, with a softened rolloff.
        const double shape = std::abs (std::sin (harmonic * pi * pluckPosition));
        const double amplitude = shape / std::pow (harmonic, harmonicRolloff);

        harmonicAmplitude[(size_t) k] = (float) amplitude;
        phaseIncrement[(size_t) k] = twoPi * harmonicFrequency / sampleRate;
        totalAmplitude += amplitude;
    }

    // Partials can momentarily align, so normalising against their sum bounds the peak at 1
    // and keeps the output level independent of the harmonic profile.
    if (totalAmplitude > 0.0)
        for (auto& amplitude : harmonicAmplitude)
            amplitude = (float) (amplitude / totalAmplitude);
}

void PluckedString::pluck()
{
    for (int k = 0; k < numHarmonics; ++k)
    {
        envelope[(size_t) k] = harmonicAmplitude[(size_t) k];
        phase[(size_t) k] = 0.0;
    }

    attackGain = 0.0f;
    ringing = true;
}

void PluckedString::silence()
{
    envelope.fill (0.0f);
    attackGain = 0.0f;
    ringing = false;
}

float PluckedString::renderSample() noexcept
{
    if (! ringing)
        return 0.0f;

    if (attackGain < 1.0f)
        attackGain = std::min (1.0f, attackGain + attackRate);

    float output = 0.0f;

    for (int k = 0; k < numHarmonics; ++k)
    {
        const auto index = (size_t) k;

        output += envelope[index] * (float) std::sin (phase[index]);

        envelope[index] *= decayPerSample[index];
        phase[index] += phaseIncrement[index];

        if (phase[index] >= twoPi)
            phase[index] -= twoPi;
    }

    // The fundamental outlasts every other partial, so it decides when the note is over.
    if (envelope[0] < silenceThreshold)
        ringing = false;

    return output * attackGain;
}
