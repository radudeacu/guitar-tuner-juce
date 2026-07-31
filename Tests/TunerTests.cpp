#include "../Source/PitchDetector.h"
#include "../Source/ReferenceTonePlayer.h"

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

namespace
{
    constexpr int toneBlockSize = 512;

    std::vector<float> renderTone (ReferenceTonePlayer& player, int numBlocks)
    {
        juce::AudioBuffer<float> buffer (1, toneBlockSize);
        std::vector<float> rendered;

        for (int block = 0; block < numBlocks; ++block)
        {
            buffer.clear();
            player.renderNextBlock (buffer, 0, toneBlockSize);

            const float* data = buffer.getReadPointer (0);
            rendered.insert (rendered.end(), data, data + toneBlockSize);
        }

        return rendered;
    }

    float largestSampleToSampleJump (const std::vector<float>& samples)
    {
        float largest = 0.0f;

        for (size_t i = 1; i < samples.size(); ++i)
            largest = juce::jmax (largest, std::abs (samples[i] - samples[i - 1]));

        return largest;
    }

    void testReferenceTonePitch()
    {
        constexpr double toneFrequencyHz = 110.0;

        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (toneFrequencyHz);
        player.setPlaying (true);

        const auto rendered = renderTone (player, 40);

        // Analyse a window from the end, well clear of the gain ramp.
        PitchDetector detector;
        detector.prepare (sampleRate, windowSize);
        const auto result = detector.detectPitch (rendered.data() + rendered.size() - windowSize, windowSize);

        expectTrue ("Reference tone: pitch detected", result.pitchFound);

        if (result.pitchFound)
            expectNear ("Reference tone: frequency", result.frequencyHz, toneFrequencyHz, 0.5);
    }

    void testReferenceToneRampsWithoutClicks()
    {
        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (110.0);
        player.setPlaying (true);

        const auto rendered = renderTone (player, 20);

        // A 110Hz sine at this amplitude moves ~0.003 per sample; an ungated start would jump
        // by roughly the full amplitude, so anything above 0.01 means a click.
        expectTrue ("Reference tone: starts without a click", largestSampleToSampleJump (rendered) < 0.01f);
    }

    void testReferenceToneStopsSilently()
    {
        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (110.0);
        player.setPlaying (true);
        renderTone (player, 10);

        player.setPlaying (false);
        renderTone (player, 10); // let the gain ramp complete

        const auto afterStop = renderTone (player, 4);
        float loudest = 0.0f;

        for (const auto sample : afterStop)
            loudest = juce::jmax (loudest, std::abs (sample));

        expectTrue ("Reference tone: fully silent after stopping", loudest == 0.0f);
    }
}

int main()
{
    std::printf ("Running tuner tests...\n\n");

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

    testReferenceTonePitch();
    testReferenceToneRampsWithoutClicks();
    testReferenceToneStopsSilently();

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
