#include "Tuning.h"

Tuning::Tuning (juce::String nameIn, std::array<TuningString, 6> stringsIn)
    : name (std::move (nameIn)), strings (std::move (stringsIn))
{
}

const juce::String& Tuning::getName() const noexcept
{
    return name;
}

const std::array<TuningString, 6>& Tuning::getStrings() const noexcept
{
    return strings;
}
