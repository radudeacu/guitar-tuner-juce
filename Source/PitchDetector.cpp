#include "PitchDetector.h"

#include <algorithm>
#include <cassert>
#include <cmath>

void PitchDetector::prepare (double newSampleRate, int windowSizeSamples)
{
    sampleRate = newSampleRate;
    yinBufferSize = windowSizeSamples / 2;

    differenceBuffer.assign (static_cast<size_t> (yinBufferSize), 0.0f);
    cmndBuffer.assign (static_cast<size_t> (yinBufferSize), 0.0f);

    minLag = std::clamp ((int) std::floor (sampleRate / maxDetectableFrequencyHz), 1, yinBufferSize - 1);
    maxLag = std::clamp ((int) std::ceil (sampleRate / minDetectableFrequencyHz), minLag + 1, yinBufferSize - 1);
}

float PitchDetector::computeRms (const float* samples, int numSamples)
{
    double sumOfSquares = 0.0;
    for (int i = 0; i < numSamples; ++i)
        sumOfSquares += (double) samples[i] * (double) samples[i];

    return (float) std::sqrt (sumOfSquares / (double) numSamples);
}

void PitchDetector::computeDifferenceFunction (const float* samples, int numSamples)
{
    assert (numSamples >= yinBufferSize * 2);
    (void) numSamples;

    for (int tau = 0; tau < yinBufferSize; ++tau)
    {
        float sum = 0.0f;

        for (int j = 0; j < yinBufferSize; ++j)
        {
            const float delta = samples[j] - samples[j + tau];
            sum += delta * delta;
        }

        differenceBuffer[(size_t) tau] = sum;
    }
}

void PitchDetector::computeCumulativeMeanNormalizedDifference()
{
    cmndBuffer[0] = 1.0f;
    float runningSum = 0.0f;

    for (int tau = 1; tau < yinBufferSize; ++tau)
    {
        runningSum += differenceBuffer[(size_t) tau];
        cmndBuffer[(size_t) tau] = runningSum > 0.0f
                                        ? differenceBuffer[(size_t) tau] * (float) tau / runningSum
                                        : 1.0f;
    }
}

int PitchDetector::findFirstMinimumBelowThreshold (float threshold) const
{
    for (int tau = minLag; tau <= maxLag; ++tau)
    {
        if (cmndBuffer[(size_t) tau] < threshold)
        {
            while (tau + 1 <= maxLag && cmndBuffer[(size_t) (tau + 1)] < cmndBuffer[(size_t) tau])
                ++tau;

            return tau;
        }
    }

    return -1;
}

double PitchDetector::parabolicInterpolationAroundTau (int tauEstimate) const
{
    const int x0 = tauEstimate < 1 ? tauEstimate : tauEstimate - 1;
    const int x2 = tauEstimate + 1 < yinBufferSize ? tauEstimate + 1 : tauEstimate;

    if (x0 == tauEstimate)
        return cmndBuffer[(size_t) tauEstimate] <= cmndBuffer[(size_t) x2] ? tauEstimate : x2;

    if (x2 == tauEstimate)
        return cmndBuffer[(size_t) tauEstimate] <= cmndBuffer[(size_t) x0] ? tauEstimate : x0;

    const float s0 = cmndBuffer[(size_t) x0];
    const float s1 = cmndBuffer[(size_t) tauEstimate];
    const float s2 = cmndBuffer[(size_t) x2];
    const float denominator = 2.0f * s1 - s2 - s0;

    if (denominator == 0.0f)
        return tauEstimate;

    return tauEstimate + (s2 - s0) / (2.0f * denominator);
}

PitchDetector::Result PitchDetector::detectPitch (const float* samples, int numSamples)
{
    Result result;

    if (computeRms (samples, numSamples) < silenceRmsThreshold)
        return result;

    computeDifferenceFunction (samples, numSamples);
    computeCumulativeMeanNormalizedDifference();

    const int tau = findFirstMinimumBelowThreshold (yinThreshold);

    if (tau < 0)
        return result;

    const double betterTau = parabolicInterpolationAroundTau (tau);

    result.pitchFound = true;
    result.frequencyHz = sampleRate / betterTau;
    result.confidence = 1.0 - (double) cmndBuffer[(size_t) tau];

    return result;
}
