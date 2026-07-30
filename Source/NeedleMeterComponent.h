#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class NeedleMeterComponent final : public juce::Component
{
public:
    void setCents (double newCents, bool newSignalPresent);
    void paint (juce::Graphics& g) override;

private:
    static juce::Colour colourForCents (double centsValue, bool hasSignal);

    double cents = 0.0;
    bool signalPresent = false;

    static constexpr double displayRangeCents = 50.0;
    static constexpr double greenThresholdCents = 5.0;
    static constexpr double yellowThresholdCents = 20.0;
};
