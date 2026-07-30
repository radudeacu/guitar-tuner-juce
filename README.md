# Guitar Tuner (JUCE)

A standalone Windows guitar tuner built with JUCE. See [prd.md](prd.md) for the feature spec and [CLAUDE.md](CLAUDE.md) for architecture/coding conventions.

## Building

Requires Visual Studio 2022 and CMake 3.22+.

```bash
git clone --recurse-submodules https://github.com/radudeacu/guitar-tuner-juce.git
cd guitar-tuner-juce
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

If you already cloned without `--recurse-submodules`, run `git submodule update --init` first.

## ASIO support (optional)

JUCE's ASIO support requires the Steinberg ASIO SDK, which cannot be redistributed due to licensing and isn't bundled here. Without it, the app builds and runs using WASAPI/DirectSound.

To enable ASIO (e.g. for a Focusrite Scarlett interface):

1. Download the ASIO SDK from Steinberg (https://www.steinberg.net/developers/) — requires agreeing to their license.
2. Unzip it somewhere, e.g. `C:/asiosdk`.
3. Reconfigure with the SDK path:
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -DASIO_SDK_PATH="C:/asiosdk"
   cmake --build build --config Debug
   ```

Once configured, ASIO-compatible drivers (e.g. "Focusrite USB ASIO") will appear in the app's audio device selector automatically — no source changes needed.
