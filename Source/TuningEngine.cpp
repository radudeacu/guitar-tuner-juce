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

int TuningEngine::findNearestStringIndex (double frequencyHz) const
{
    int bestIndex = 0;
    double bestAbsCents = std::numeric_limits<double>::max();

    const auto& strings = currentTuning.getStrings();

    for (int i = 0; i < (int) strings.size(); ++i)
    {
        const double targetFrequencyHz = strings[(size_t) i].frequencyHz * (referenceA4 / 440.0);
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
    const double targetFrequencyHz = target.frequencyHz * (referenceA4 / 440.0);

    result.noteName = target.noteName;
    result.targetFrequencyHz = targetFrequencyHz;
    result.cents = PitchMath::frequencyToCents (detectedFrequencyHz, targetFrequencyHz);
    result.stringIndex = index;
    result.valid = true;

    return result;
}
