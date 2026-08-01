#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>
#include <vector>

#include "OptionsPanel.h"
#include "PitchDetector.h"
#include "ReferenceToneBar.h"
#include "ReferenceTonePlayer.h"
#include "TunerDisplayComponent.h"
#include "TunerSettings.h"
#include "TuningEngine.h"

class MainComponent final : public juce::AudioAppComponent,
                            private juce::Timer,
                            private juce::ChangeListener
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
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void appendToCaptureBuffer (const float* samples, int numSamples); // audio thread only
    void showOptions();
    void applySelectedTuning();
    void restoreSavedTuning();
    void updateReferenceToneFrequency();
    void refreshBackdropCacheIfNeeded();

    static constexpr int windowSize = 2048;
    static constexpr int hopSize = 512;

    PitchDetector pitchDetector;
    TuningEngine tuningEngine;
    ReferenceTonePlayer referenceTonePlayer;

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

    // Declared first so it outlives everything that writes to it during teardown.
    TunerSettings settings;

    // Read only from timerCallback and UI callbacks — both on the message thread — so the
    // engine needs no synchronisation despite the detector running on the audio thread.
    std::vector<Tuning> availableTunings;

    juce::TextButton optionsButton { "Options" };
    juce::ComboBox tuningSelector;
    OptionsPanel optionsPanel;
    TunerDisplayComponent tunerDisplay;
    ReferenceToneBar referenceToneBar;
};
