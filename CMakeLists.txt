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

# --- ASIO support: optional, requires the Steinberg ASIO SDK (license-restricted, not redistributable) ---
# See README.md for setup instructions. Without ASIO_SDK_PATH set, the app builds and runs using
# WASAPI/DirectSound only (both enabled by default in JUCE).
option(GUITAR_TUNER_ENABLE_ASIO "Enable ASIO support (requires Steinberg ASIO SDK)" ON)
set(ASIO_SDK_PATH "" CACHE PATH "Path to Steinberg ASIO SDK root (folder containing common/, host/, host/pc/)")

set(_asio_available FALSE)
if (GUITAR_TUNER_ENABLE_ASIO AND ASIO_SDK_PATH AND EXISTS "${ASIO_SDK_PATH}/common/iasiodrv.h")
    set(_asio_available TRUE)
elseif (GUITAR_TUNER_ENABLE_ASIO AND ASIO_SDK_PATH)
    message(WARNING "ASIO_SDK_PATH set but iasiodrv.h not found under ${ASIO_SDK_PATH}/common — building without ASIO.")
endif()

if (_asio_available)
    target_compile_definitions(GuitarTuner PRIVATE JUCE_ASIO=1)
    target_include_directories(GuitarTuner PRIVATE
        "${ASIO_SDK_PATH}/common"
        "${ASIO_SDK_PATH}/host"
        "${ASIO_SDK_PATH}/host/pc")
    message(STATUS "ASIO SDK found — ASIO support enabled.")
else()
    message(STATUS "ASIO SDK not configured — building without ASIO (WASAPI/DirectSound only). Set -DASIO_SDK_PATH=<path> to enable.")
endif()

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
