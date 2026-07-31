#include "TuningEngine.h"
#include "PitchMath.h"
#include "TuningPresets.h"

#include <cmath>
#include <limits>

TuningEngine::TuningEngine()
    : currentTuning (TuningPresets::standard())
{
}

void TuningEngine::setTuning (Tuning newTuning)
{
    currentTuning = std::move (newTuning);
}

void TuningEngine::setReferenceA4 (double newReferenceHz)
{
    referenceA4 = newReferenceHz;
}

const Tuning& TuningEngine::getTuning() const noexcept
{
    return currentTuning;
}

double TuningEngine::getStringFrequencyHz (int stringIndex) const
{
    const auto& strings = currentTuning.getStrings();

    if (! juce::isPositiveAndBelow (stringIndex, (int) strings.size()))
        return 0.0;

    // Preset frequencies are baked at A4 = 440Hz, so the whole tuning scales with the reference.
    return strings[(size_t) stringIndex].frequencyHz * (referenceA4 / 440.0);
}

int TuningEngine::findNearestStringIndex (double frequencyHz) const
{
    int bestIndex = 0;
    double bestAbsCents = std::numeric_limits<double>::max();

    const auto& strings = currentTuning.getStrings();

    for (int i = 0; i < (int) strings.size(); ++i)
    {
        const double targetFrequencyHz = getStringFrequencyHz (i);
        const double absCents = std::abs (PitchMath::frequencyToCents (frequencyHz, targetFrequencyHz));

        if (absCents < bestAbsCents)
        {
            bestAbsCents = absCents;
            bestIndex = i;
        }
    }

    return bestIndex;
}

TuningMatchResult TuningEngine::evaluate (double detectedFrequencyHz) const
{
    TuningMatchResult result;

    if (detectedFrequencyHz <= 0.0)
        return result;

    const int index = findNearestStringIndex (detectedFrequencyHz);
    const auto& target = currentTuning.getStrings()[(size_t) index];
    const double targetFrequencyHz = getStringFrequencyHz (index);

    result.noteName = target.noteName;
    result.targetFrequencyHz = targetFrequencyHz;
    result.cents = PitchMath::frequencyToCents (detectedFrequencyHz, targetFrequencyHz);
    result.stringIndex = index;
    result.valid = true;

    return result;
}
