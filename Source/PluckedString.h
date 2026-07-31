#pragma once

#include <array>

/*  Additive synthesis of a plucked steel string.

    Additive rather than Karplus-Strong because this is a tuning reference: partials sit at
    exact integer multiples of the target, whereas a delay-line model quantises pitch to whole
    samples and would drift sharp or flat depending on the note.

    Realism comes from two things — a harmonic profile taken from the ideal plucked-string
    model, and per-harmonic decay, since upper partials die away faster than the fundamental.
    That is what makes a real string sound bright at the attack and mellow as it rings out.
*/
class PluckedString
{
public:
    void prepare (double newSampleRate);

    /** Takes effect on the next pluck, so retuning never bends a ringing note. */
    void setFrequency (double newFrequencyHz);

    void pluck();
    void silence();

    bool isRinging() const noexcept   { return ringing; }

    float renderSample() noexcept;

private:
    void updateHarmonicProfile();

    static constexpr int numHarmonics = 10;

    // Plucking one fifth along the string is typical picking position, and gives the
    // characteristic missing fifth harmonic.
    static constexpr double pluckPosition = 0.2;

    // Rolloff gentler than the ideal string's 1/k^2, which alone sounds too dull —
    // real instruments carry more upper harmonic energy through the body.
    static constexpr double harmonicRolloff = 1.2;

    static constexpr double fundamentalDecaySeconds = 0.7;
    static constexpr double harmonicDecayExponent = 0.7; // higher partials decay faster
    static constexpr double attackSeconds = 0.004;
    static constexpr float silenceThreshold = 0.0005f;

    double sampleRate = 44100.0;
    double frequencyHz = 440.0;

    std::array<float, numHarmonics> harmonicAmplitude {};
    std::array<float, numHarmonics> decayPerSample {};
    std::array<float, numHarmonics> envelope {};
    std::array<double, numHarmonics> phase {};
    std::array<double, numHarmonics> phaseIncrement {};

    float attackGain = 0.0f;
    float attackRate = 1.0f;
    bool ringing = false;
};
