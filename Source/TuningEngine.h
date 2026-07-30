#pragma once

#include "Tuning.h"

struct TuningMatchResult
{
    juce::String noteName;
    double targetFrequencyHz = 0.0; // adjusted for the current reference A4
    double cents = 0.0;             // detected vs targetFrequencyHz; + sharp, - flat
    int stringIndex = -1;
    bool valid = false;
};

class TuningEngine
{
public:
    TuningEngine();

    void setTuning (Tuning newTuning);
    void setReferenceA4 (double newReferenceHz);

    TuningMatchResult evaluate (double detectedFrequencyHz) const;

private:
    int findNearestStringIndex (double frequencyHz) const;

    Tuning currentTuning;
    double referenceA4 = 440.0;
};
