#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>
#include <vector>

#include "PitchDetector.h"
#include "TunerDisplayComponent.h"
#include "TuningEngine.h"

class MainComponent final : public juce::AudioAppComponent, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void appendToCaptureBuffer (const float* samples, int numSamples); // audio thread only
    void showAudioSettings();
    void refreshBackdropCacheIfNeeded();

    static constexpr int windowSize = 2048;
    static constexpr int hopSize = 512;

    PitchDetector pitchDetector;
    TuningEngine tuningEngine;

    std::vector<float> ringBuffer;
    std::vector<float> analysisScratch;
    int ringWriteIndex = 0;
    int samplesSinceLastAnalysis = 0;

    std::atomic<float> latestFrequencyHz { 0.0f };
    std::atomic<float> latestConfidence  { 0.0f };
    std::atomic<bool>  signalPresent     { false };

    // The translucent card forces the backdrop underneath it to repaint on every timer tick,
    // so the gradient/blob composition is rendered once and blitted rather than recomputed.
    juce::Image backdropCache;

    juce::TextButton optionsButton { "Options" };
    juce::AudioDeviceSelectorComponent audioSettings;
    TunerDisplayComponent tunerDisplay;
};
