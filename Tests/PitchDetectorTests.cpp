#include "../Source/PitchDetector.h"

#include <juce_core/juce_core.h>

#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
    constexpr double sampleRate = 44100.0;
    constexpr int windowSize = 2048;
    constexpr double twoPi = 6.283185307179586;

    std::vector<float> generateSine (double frequencyHz, int numSamples, float amplitude = 0.5f)
    {
        std::vector<float> samples ((size_t) numSamples);

        for (int i = 0; i < numSamples; ++i)
            samples[(size_t) i] = amplitude * (float) std::sin (twoPi * frequencyHz * (double) i / sampleRate);

        return samples;
    }

    int failureCount = 0;

    void expectNear (const char* testName, double actual, double expected, double tolerance)
    {
        if (std::abs (actual - expected) > tolerance)
        {
            std::printf ("FAIL: %s — expected %.3f, got %.3f (tolerance %.3f)\n", testName, expected, actual, tolerance);
            ++failureCount;
        }
        else
        {
            std::printf ("PASS: %s — got %.3f (expected %.3f)\n", testName, actual, expected);
        }
    }

    void expectTrue (const char* testName, bool condition)
    {
        if (! condition)
        {
            std::printf ("FAIL: %s\n", testName);
            ++failureCount;
        }
        else
        {
            std::printf ("PASS: %s\n", testName);
        }
    }

    void testKnownFrequency (const char* stringName, double frequencyHz)
    {
        PitchDetector detector;
        detector.prepare (sampleRate, windowSize);

        const auto samples = generateSine (frequencyHz, windowSize);
        const auto result = detector.detectPitch (samples.data(), windowSize);

        expectTrue ((juce::String (stringName) + ": pitch found").toRawUTF8(), result.pitchFound);

        if (result.pitchFound)
        {
            // tolerance: within 1 cent -> ratio tolerance of 2^(1/1200)
            const double toleranceHz = frequencyHz * (std::pow (2.0, 1.0 / 1200.0) - 1.0);
            expectNear ((juce::String (stringName) + ": frequency").toRawUTF8(), result.frequencyHz, frequencyHz, toleranceHz);
        }
    }
}

int main()
{
    std::printf ("Running PitchDetector tests...\n\n");

    testKnownFrequency ("E2 (low E)", 82.41);
    testKnownFrequency ("A2", 110.00);
    testKnownFrequency ("D3", 146.83);
    testKnownFrequency ("G3", 196.00);
    testKnownFrequency ("B3", 246.94);
    testKnownFrequency ("E4 (high E)", 329.63);

    {
        PitchDetector detector;
        detector.prepare (sampleRate, windowSize);

        std::vector<float> silence ((size_t) windowSize, 0.0f);
        const auto result = detector.detectPitch (silence.data(), windowSize);

        expectTrue ("Silence: no pitch found", ! result.pitchFound);
    }

    {
        // Sanity check that the test harness itself can fail: a deliberately wrong expectation must be reported as FAIL.
        const int before = failureCount;
        expectNear ("Sanity check (expected to fail)", 100.0, 200.0, 1.0);
        expectTrue ("Sanity check produced a failure", failureCount == before + 1);
        failureCount = before; // don't let the deliberate sanity failure affect the real exit code
    }

    std::printf ("\n%s\n", failureCount == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return failureCount == 0 ? 0 : 1;
}
