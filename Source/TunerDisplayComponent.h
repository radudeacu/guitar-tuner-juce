#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "NeedleMeterComponent.h"
#include "TuningEngine.h"

/*  The main readout: a glass card carrying the arc meter, note name, cents and frequency.

    Values are painted directly rather than held in child Labels so type, colour and baseline
    can be tuned together as one composition.
*/
class TunerDisplayComponent final : public juce::Component
{
public:
    TunerDisplayComponent();

    void setResult (const TuningMatchResult& result, double detectedFrequencyHz, bool hasSignal);

    /** Skips the meter's easing animation so the display settles immediately. */
    void snapToCurrentResult();

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    /*  Where each element sits. Derived in one place so paint() and resized() can never
        disagree about the composition. */
    struct Layout
    {
        juce::Rectangle<int> meter, note, hint, cents, frequency;
    };

    Layout computeLayout() const;

    void paintNoteName (juce::Graphics& g, juce::Rectangle<int> area) const;
    void paintStringHint (juce::Graphics& g, juce::Rectangle<int> area) const;
    void paintCents (juce::Graphics& g, juce::Rectangle<int> area) const;
    void paintFrequency (juce::Graphics& g, juce::Rectangle<int> area) const;

    juce::String notePitchClass() const;
    juce::String noteOctave() const;

    NeedleMeterComponent needleMeter;

    juce::String noteName;
    double cents = 0.0;
    double frequencyHz = 0.0;
    bool showingResult = false;
};
