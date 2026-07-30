#pragma once

#include <cmath>
#include <juce_core/juce_core.h>

namespace PitchMath
{
    inline double midiNoteToFrequency (int midiNote, double referenceA4 = 440.0)
    {
        return referenceA4 * std::pow (2.0, (midiNote - 69) / 12.0);
    }

    inline double frequencyToCents (double frequencyHz, double referenceFrequencyHz)
    {
        if (frequencyHz <= 0.0 || referenceFrequencyHz <= 0.0)
            return 0.0;

        return 1200.0 * std::log2 (frequencyHz / referenceFrequencyHz);
    }

    inline juce::String midiNoteToName (int midiNote)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

        const int octave = midiNote / 12 - 1;
        const int pitchClass = ((midiNote % 12) + 12) % 12;

        return juce::String (names[pitchClass]) + juce::String (octave);
    }
}
