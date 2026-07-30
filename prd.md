# PRD: Guitar Tuner (JUCE Standalone)

## Goal
A standalone Windows desktop application that listens to a live guitar signal via an audio interface (ASIO), detects the pitch of a single played note in real time, and displays how in-tune it is against standard or alternate guitar tunings.

## Features

### Audio Input
- Input device selection via JUCE's built-in `AudioDeviceSelectorComponent` (device, sample rate, buffer size).
- ASIO driver support (tested against Focusrite Scarlett 2i2 / Focusrite USB ASIO driver).
- Single mono input channel (guitar signal).

### Pitch Detection
- Real-time monophonic pitch detection (single note at a time — no chord/polyphonic detection).
- Detection range: standard guitar range, approx. E2 (~82Hz) to E6 (~1300Hz, covering high fret positions).
- Algorithm: autocorrelation-based (YIN), chosen for accuracy and low latency [NEED: confirm exact buffer size during implementation — target lowest latency that keeps detection stable].

### Tunings
- Default tuning: Standard (E2 A2 D3 G3 B3 E4).
- Alternate tuning presets selectable from a list (e.g. Drop D, Open D, Open G, Open E, DADGAD, half-step down — exact preset list to be finalized during implementation).
- Custom tuning builder: user can set each of the 6 strings to any note independently.
- No persistence of custom tunings required for V1 (session-only).

### Display / UI
- Detected note name (e.g. "A2").
- Cents-off-from-pitch, numeric.
- Needle/meter showing tuning offset, range ±50 cents.
- Numeric frequency readout in Hz.
- Color in-tune indicator:
  - Green: within ±5 cents (in tune)
  - Yellow: moderately off
  - Red: far off
- Reference pitch control: default A4 = 440Hz, adjustable by the user.
- Reference tone playback: button to play back the target pitch (selected string/note) as an audible tone, so the user can tune by ear alongside the visual display.
- Detailed visual layout/styling is deferred to a later phase, after detection logic is working.

## Behaviour
- App must run standalone on Windows (no DAW/plugin host required).
- Must support ASIO drivers for low-latency input (specifically validated against Focusrite Scarlett 2i2).
- Pitch detection should update continuously while a note is sounding/decaying, not just once per pluck.
- If no signal / silence is detected, UI should indicate no-input state rather than showing stale or false readings.
- Reference tone playback must not interfere with live pitch detection (e.g. should not be picked up as input if monitoring is active).

## Out of Scope
- VST3 and AAX plugin formats (standalone only for V1; may be added later — AAX specifically requires an Avid developer agreement and PACE/iLok signing, which is significant additional scope).
- Bass guitar or other non-guitar instrument tuning ranges.
- Strobe-tuner display mode.
- Pitch stability/hold-before-confirming-in-tune logic (e.g. requiring a stable reading for X ms before showing "in tune").
- Saving/persisting custom tunings between sessions.
- MIDI input/output.
- Recording or audio export.
- Chord/polyphonic detection — single note only.
- Final visual design/UI polish (basic functional UI only for V1).
