# Guitar Tuner (JUCE)

A standalone Windows guitar tuner built with JUCE. See [prd.md](prd.md) for the feature spec and [CLAUDE.md](CLAUDE.md) for architecture/coding conventions.
<br>
<img width="64" height="64" alt="icon" src="https://github.com/user-attachments/assets/01190320-0ac4-440a-bce7-1ebcba285386" />

## Building

Requires Visual Studio 2022 and CMake 3.22+.

```bash
git clone --recurse-submodules https://github.com/radudeacu/guitar-tuner-juce.git
cd guitar-tuner-juce
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

If you already cloned without `--recurse-submodules`, run `git submodule update --init` first.

## ASIO support

Enabled by default (`JUCE_ASIO=1`). JUCE 8.0.11+ bundles the minimal ASIO SDK headers it needs internally, so no separate Steinberg SDK download is required. ASIO-compatible drivers (e.g. "Focusrite USB ASIO") appear automatically in the app's audio device selector.
