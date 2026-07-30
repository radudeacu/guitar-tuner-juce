# Guitar Tuner (JUCE)

## What This Is
A standalone Windows guitar tuner built with JUCE. Detects live pitch from an ASIO audio interface and shows note name, cents offset, needle, Hz, and an in-tune color indicator. Standard and alternate tunings supported. See [prd.md](prd.md) for full feature spec.

## Coding Principles
- **Single responsibility.** Every function does one thing. If you need an "and" to describe what it does, split it.
- **Small functions.** If a function exceeds ~30 lines, it's probably doing too much. Extract sub-operations into their own functions with descriptive names.
- **No god classes.** Each class should represent one clear concept or responsibility. If a class is accumulating unrelated methods, decompose it.
- **Composition over deep inheritance.** Prefer building complex behaviour by combining small, focused objects rather than creating deep class hierarchies. One or two levels of inheritance is fine — five is a design smell.
- **Descriptive names over comments.** A function called `calculateCentsOffset()` doesn't need a comment. A function called `calc()` needs a refactor, not a comment.
- **Comment why, not what.** The code shows what's happening. Comments explain the reasoning behind non-obvious decisions.
- **Fail early and clearly.** Validate inputs at function boundaries. Don't let bad state propagate silently through the system.

## Tech Stack
- **Framework:** JUCE 8.0.4, installed locally at `C:\Users\yo_ra\Documents\JUCE\JUCE`
- **Language:** C++17
- **Build system:** CMake (JUCE's official CMake API, via `add_subdirectory` pointing at the local JUCE checkout)
- **Target format:** Standalone app only for now (Windows). AAX is a planned future target — do not scaffold plugin wrapper code until asked.
- **IDE/toolchain:** Visual Studio 2022 as the CMake generator/compiler
- **Key JUCE modules:** `juce_audio_utils` (`AudioAppComponent`, `AudioDeviceSelectorComponent`), `juce_audio_devices` (ASIO), `juce_dsp` (FFT/windowing if needed for pitch detection), `juce_gui_basics`

## ASIO Setup Note
JUCE's ASIO support requires the Steinberg ASIO SDK, which cannot be bundled due to licensing. It must be downloaded separately and its path set via `JUCE_ASIO_SDK_PATH`/CMake option, with `JUCE_ASIO=1` defined. Don't assume ASIO "just works" out of the box — flag this setup step during initial build configuration.

## Architecture Preferences
- Separate DSP from UI, even in a standalone app: a `PitchDetector` class (YIN algorithm) takes an audio buffer and returns detected frequency + confidence — it has no knowledge of UI or JUCE components beyond `juce::AudioBuffer`.
- A `TuningEngine` (or similarly named) class owns the current `Tuning` (list of target string notes/frequencies), compares a detected frequency against the nearest target, and computes note name + cents offset. Keep this separate from `PitchDetector`.
- A `Tuning` class/struct represents one tuning (6 string targets). Presets and the custom tuning builder both produce a `Tuning` instance — the rest of the system shouldn't care which.
- Structure the audio/DSP layer so it could later be lifted into a `PluginProcessor` with minimal change if AAX support is added — i.e., don't couple pitch detection logic to `AudioAppComponent` directly; have `AudioAppComponent` own and drive it.
- Audio thread (`getNextAudioBlock`/audio callback) must never touch UI components directly. Pass detection results to the message thread via `std::atomic` fields or a lock-free FIFO; update UI on a `juce::Timer` callback.
- Reference tone playback is a separate audio source (e.g. `juce::ToneGeneratorAudioSource` or custom oscillator) mixed only to output, never fed back into the pitch detector's input path.

## Code Style
- Follow JUCE conventions: camelCase for variables/methods, PascalCase for classes.
- Use the `juce::` namespace prefix explicitly (don't use `using namespace juce;`).
- Prefer RAII — acquire resources in constructors, release in destructors.

## DSP Constraints
- No memory allocations in the audio callback — pre-allocate buffers in `prepareToPlay()`/`prepareToPlay`-equivalent setup.
- No locks or blocking operations on the audio thread.
- Use `juce::SmoothedValue` for any audio-rate value changes (e.g. reference tone amplitude/frequency ramps) to avoid zipper noise.
- Use `juce::dsp` module utilities where they fit before writing custom DSP math.

## What NOT To Do
- Don't scaffold VST3/AU/AAX plugin wrapper code yet — standalone only until explicitly asked to add a plugin format.
- Don't add third-party dependencies without asking first.
- Don't hardcode behavior specific to the Focusrite Scarlett 2i2 — target any ASIO-compatible device, test against the 2i2.
- Don't put DSP/pitch-detection logic in UI component code.
- Don't use raw owning pointers — prefer `std::unique_ptr` or JUCE equivalents.
- Don't over-engineer — keep the architecture simple and flat. No deep inheritance hierarchies or speculative abstraction layers.
- Don't add features not specified in [prd.md](prd.md) (e.g. no chord detection, no strobe mode, no tuning persistence) without checking in first.
