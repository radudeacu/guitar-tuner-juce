#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*  Arc gauge showing how far off pitch the detected note is.

    Owns an animation timer so the needle eases toward each new reading rather than snapping —
    a plucked string wavers, and easing keeps that from reading as jitter.
*/
class NeedleMeterComponent final : public juce::Component, private juce::Timer
{
public:
    NeedleMeterComponent();

    void setCents (double newCents, bool newSignalPresent);

    /** Finishes the easing immediately, so the needle appears at the current reading rather
        than sweeping to it. Used when the reading jumps for a non-musical reason. */
    void snapToTarget();

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;
    bool advanceAnimation();

    void paintTrack (juce::Graphics& g, juce::Point<float> centre, float radius) const;
    void paintInTuneZone (juce::Graphics& g, juce::Point<float> centre, float radius) const;
    void paintTicks (juce::Graphics& g, juce::Point<float> centre, float radius) const;
    void paintNeedle (juce::Graphics& g, juce::Point<float> centre, float radius) const;

    float angleForCents (double cents) const;
    juce::Colour currentColour() const;

    static constexpr float sweepDegrees = 60.0f;   // half-sweep either side of centre
    static constexpr float trackThickness = 10.0f;
    static constexpr float easingFactor = 0.22f;
    static constexpr float hubRadius = 8.0f;

    double targetCents = 0.0;
    double displayedCents = 0.0;
    float targetPresence = 0.0f;   // 1 when a pitch is being tracked, fades out on silence
    float displayedPresence = 0.0f;
    bool signalPresent = false;
};
