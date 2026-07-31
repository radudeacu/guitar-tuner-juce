#include "../Source/PitchDetector.h"
#include "../Source/PluckedString.h"
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

    float loudestSample (const std::vector<float>& samples)
    {
        float loudest = 0.0f;

        for (const auto sample : samples)
            loudest = juce::jmax (loudest, std::abs (sample));

        return loudest;
    }

    void testReferenceTonePitch()
    {
        constexpr double toneFrequencyHz = 110.0;

        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (toneFrequencyHz);
        player.pluck();

        // Analyse just after the attack, while the note is still at full strength.
        const auto rendered = renderTone (player, 8);

        PitchDetector detector;
        detector.prepare (sampleRate, windowSize);
        const auto result = detector.detectPitch (rendered.data() + 512, windowSize);

        expectTrue ("Plucked tone: pitch detected", result.pitchFound);

        // A tuning reference has to be exactly on pitch despite the added harmonics.
        if (result.pitchFound)
            expectNear ("Plucked tone: fundamental is exact", result.frequencyHz, toneFrequencyHz, 0.2);
    }

    void testPluckIsHarmonicallyRich()
    {
        PluckedString string;
        string.prepare (sampleRate);
        string.setFrequency (110.0);
        string.pluck();

        // A pure sine never exceeds its own amplitude; summed partials reinforce, so a
        // crest factor well above a sine's 1.41 shows real harmonic content.
        double sumOfSquares = 0.0;
        float peak = 0.0f;
        constexpr int numSamples = 4096;

        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = string.renderSample();
            sumOfSquares += (double) sample * sample;
            peak = juce::jmax (peak, std::abs (sample));
        }

        const double rms = std::sqrt (sumOfSquares / numSamples);
        expectTrue ("Plucked tone: richer than a sine", rms > 0.0 && (peak / rms) > 1.8);
    }

    void testPluckDecaysToSilence()
    {
        PluckedString string;
        string.prepare (sampleRate);
        string.setFrequency (110.0);
        string.pluck();

        // Six seconds is comfortably longer than the note should ring for.
        for (int i = 0; i < (int) (sampleRate * 6.0); ++i)
            string.renderSample();

        expectTrue ("Plucked tone: stops ringing on its own", ! string.isRinging());
    }

    /*  A click is a step comparable to the signal's own amplitude. Harmonic content legitimately
        raises the sample-to-sample slope — the highest partial here moves about 14% of peak per
        sample — so the check is relative to peak rather than an absolute figure calibrated for
        a sine. A true discontinuity would approach 100%.
    */
    void expectNoClick (const char* testName, const std::vector<float>& samples)
    {
        const float peak = loudestSample (samples);
        const float jump = largestSampleToSampleJump (samples);
        const float ratio = peak > 0.0f ? jump / peak : 0.0f;

        std::printf ("      (peak %.3f, largest jump %.3f, ratio %.2f)\n", peak, jump, ratio);
        expectTrue (testName, peak > 0.0f && ratio < 0.30f);
    }

    void testReferenceToneStartsWithoutClick()
    {
        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (110.0);
        player.pluck();

        expectNoClick ("Plucked tone: starts without a click", renderTone (player, 20));
    }

    void testRetriggerDoesNotClick()
    {
        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (110.0);
        player.pluck();
        renderTone (player, 4); // let it ring

        player.pluck(); // interrupt mid-ring — the fade should absorb the discontinuity
        expectNoClick ("Plucked tone: re-plucking mid-ring does not click", renderTone (player, 8));
    }

    void testLoopingRepeatsAfterDecay()
    {
        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (110.0);
        player.setLooping (true);
        player.pluck();

        // Render well past a single note's decay; looping should have re-plucked by then.
        const int blocksForEightSeconds = (int) (sampleRate * 8.0) / toneBlockSize;
        renderTone (player, blocksForEightSeconds);

        const auto later = renderTone (player, 8);
        expectTrue ("Plucked tone: keeps sounding while looping", loudestSample (later) > 0.001f);
    }

    void testNotLoopingFallsSilent()
    {
        ReferenceTonePlayer player;
        player.prepare (sampleRate, toneBlockSize);
        player.setFrequency (110.0);
        player.setLooping (false);
        player.pluck();

        const int blocksForEightSeconds = (int) (sampleRate * 8.0) / toneBlockSize;
        renderTone (player, blocksForEightSeconds);

        const auto later = renderTone (player, 8);
        expectTrue ("Plucked tone: one-shot falls silent", loudestSample (later) == 0.0f);
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
    testPluckIsHarmonicallyRich();
    testPluckDecaysToSilence();
    testReferenceToneStartsWithoutClick();
    testRetriggerDoesNotClick();
    testLoopingRepeatsAfterDecay();
    testNotLoopingFallsSilent();

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
