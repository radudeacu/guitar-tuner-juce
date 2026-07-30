#include "TuningPresets.h"
#include "PitchMath.h"

namespace
{
    TuningString stringFromMidiNote (int midiNote)
    {
        return { PitchMath::midiNoteToName (midiNote), PitchMath::midiNoteToFrequency (midiNote) };
    }
}

namespace TuningPresets
{
    Tuning standard()
    {
        return Tuning ("Standard",
                        { stringFromMidiNote (40),  // E2
                          stringFromMidiNote (45),  // A2
                          stringFromMidiNote (50),  // D3
                          stringFromMidiNote (55),  // G3
                          stringFromMidiNote (59),  // B3
                          stringFromMidiNote (64) }); // E4
    }

    Tuning dropD()
    {
        return Tuning ("Drop D",
                        { stringFromMidiNote (38),  // D2
                          stringFromMidiNote (45),  // A2
                          stringFromMidiNote (50),  // D3
                          stringFromMidiNote (55),  // G3
                          stringFromMidiNote (59),  // B3
                          stringFromMidiNote (64) }); // E4
    }
}
