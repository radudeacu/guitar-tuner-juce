#pragma once

#include <array>
#include <juce_core/juce_core.h>

struct TuningString
{
    juce::String noteName;
    double frequencyHz = 0.0; // baked at reference A4 = 440Hz
};

class Tuning
{
public:
    Tuning (juce::String name, std::array<TuningString, 6> stringsLowToHigh);

    const juce::String& getName() const noexcept;
    const std::array<TuningString, 6>& getStrings() const noexcept;

private:
    juce::String name;
    std::array<TuningString, 6> strings;
};
