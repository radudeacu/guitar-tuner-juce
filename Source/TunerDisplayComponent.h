#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "NeedleMeterComponent.h"
#include "TuningEngine.h"

class TunerDisplayComponent final : public juce::Component
{
public:
    TunerDisplayComponent();

    void setResult (const TuningMatchResult& result, double detectedFrequencyHz, bool hasSignal);

    void resized() override;

private:
    juce::Label noteNameLabel;
    juce::Label centsLabel;
    juce::Label hzLabel;
    NeedleMeterComponent needleMeter;
};
