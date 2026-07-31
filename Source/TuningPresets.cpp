#include "TuningPresets.h"
#include "PitchMath.h"

#include <array>

namespace
{
    using MidiNotes = std::array<int, 6>;

    /*  Presets are defined as MIDI note numbers, low string to high, because that keeps the
        interval relationships obvious and leaves name/frequency derivation in one place.
        Reference points: E2 = 40, A4 = 69.

        Accidentals are always named as sharps, so half-step-down reads D#2 rather than the
        Eb2 a guitarist might write. Enharmonically identical, and consistent across presets.
    */
    const MidiNotes standardNotes      { 40, 45, 50, 55, 59, 64 }; // E  A  D  G  B  E
    const MidiNotes dropDNotes         { 38, 45, 50, 55, 59, 64 }; // D  A  D  G  B  E
    const MidiNotes openDNotes         { 38, 45, 50, 54, 57, 62 }; // D  A  D  F# A  D
    const MidiNotes openGNotes         { 38, 43, 50, 55, 59, 62 }; // D  G  D  G  B  D
    const MidiNotes openENotes         { 40, 47, 52, 56, 59, 64 }; // E  B  E  G# B  E
    const MidiNotes dadgadNotes        { 38, 45, 50, 55, 57, 62 }; // D  A  D  G  A  D
    const MidiNotes halfStepDownNotes  { 39, 44, 49, 54, 58, 63 }; // D# G# C# F# A# D#

    TuningString stringFromMidiNote (int midiNote)
    {
        return { PitchMath::midiNoteToName (midiNote), PitchMath::midiNoteToFrequency (midiNote) };
    }

    Tuning fromMidiNotes (juce::String name, const MidiNotes& midiNotes)
    {
        std::array<TuningString, 6> strings;

        for (size_t i = 0; i < strings.size(); ++i)
            strings[i] = stringFromMidiNote (midiNotes[i]);

        return Tuning (std::move (name), strings);
    }
}

namespace TuningPresets
{
    Tuning standard()
    {
        return fromMidiNotes ("Standard", standardNotes);
    }

    std::vector<Tuning> all()
    {
        return { standard(),
                 fromMidiNotes ("Drop D",         dropDNotes),
                 fromMidiNotes ("Open D",         openDNotes),
                 fromMidiNotes ("Open G",         openGNotes),
                 fromMidiNotes ("Open E",         openENotes),
                 fromMidiNotes ("DADGAD",         dadgadNotes),
                 fromMidiNotes ("Half Step Down", halfStepDownNotes) };
    }
}
