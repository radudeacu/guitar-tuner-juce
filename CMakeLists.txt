cmake_minimum_required(VERSION 3.22)

project(GUITAR_TUNER VERSION 0.0.1)

add_subdirectory(JUCE)

juce_add_gui_app(GuitarTuner
    PRODUCT_NAME "Guitar Tuner")

target_sources(GuitarTuner
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/PitchDetector.cpp
        Source/Tuning.cpp
        Source/TuningPresets.cpp
        Source/TuningEngine.cpp
        Source/NeedleMeterComponent.cpp
        Source/TunerDisplayComponent.cpp)

target_compile_definitions(GuitarTuner
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        # Enables ASIO device support (MS Windows only) using JUCE's own bundled ASIO headers
        # (JUCE >= 8.0.11) — no separate Steinberg SDK download required. Real interfaces
        # (e.g. Focusrite Scarlett) expose their low-latency ASIO driver in the device selector.
        JUCE_ASIO=1
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:GuitarTuner,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:GuitarTuner,JUCE_VERSION>")

target_link_libraries(GuitarTuner
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# --- PitchDetector unit tests: standalone console app, no third-party test framework ---
juce_add_console_app(PitchDetectorTests
    PRODUCT_NAME "PitchDetectorTests")

target_sources(PitchDetectorTests
    PRIVATE
        Source/PitchDetector.cpp
        Tests/PitchDetectorTests.cpp)

target_link_libraries(PitchDetectorTests
    PRIVATE
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
