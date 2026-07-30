#pragma once

#include <vector>

class PitchDetector
{
public:
    struct Result
    {
        double frequencyHz = 0.0;
        double confidence  = 0.0;
        bool   pitchFound  = false;
    };

    void   prepare (double sampleRate, int windowSizeSamples);
    Result detectPitch (const float* samples, int numSamples);

private:
    void   computeDifferenceFunction (const float* samples, int numSamples);
    void   computeCumulativeMeanNormalizedDifference();
    int    findFirstMinimumBelowThreshold (float threshold) const;
    double parabolicInterpolationAroundTau (int tauEstimate) const;
    static float computeRms (const float* samples, int numSamples);

    static constexpr float yinThreshold = 0.15f;
    static constexpr float silenceRmsThreshold = 0.003f;
    static constexpr double minDetectableFrequencyHz = 70.0;   // below open low E (~82.4Hz), with margin
    static constexpr double maxDetectableFrequencyHz = 1400.0; // above high fret positions on high E

    double sampleRate = 44100.0;
    int yinBufferSize = 0;
    int minLag = 0;
    int maxLag = 0;

    std::vector<float> differenceBuffer;
    std::vector<float> cmndBuffer;
};
