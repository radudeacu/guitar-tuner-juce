#include "MainComponent.h"

MainComponent::MainComponent()
    : audioSettings (deviceManager,
                     1, 1,     // min/max audio input channels
                     0, 0,     // min/max audio output channels
                     false,    // showMidiInputOptions
                     false,    // showMidiOutputSelector
                     true,     // showChannelsAsStereoPairs
                     false)    // hideAdvancedOptionsWithButton
{
    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this] { showAudioSettings(); };

    addAndMakeVisible (tunerDisplay);

    setAudioChannels (1, 0);
    setSize (800, 700);

    startTimerHz (30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
}

void MainComponent::prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate)
{
    pitchDetector.prepare (sampleRate, windowSize);

    ringBuffer.assign ((size_t) windowSize, 0.0f);
    analysisScratch.assign ((size_t) windowSize, 0.0f);
    ringWriteIndex = 0;
    samplesSinceLastAnalysis = 0;
}

void MainComponent::appendToCaptureBuffer (const float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        ringBuffer[(size_t) ringWriteIndex] = samples[i];
        ringWriteIndex = (ringWriteIndex + 1) % windowSize;
        ++samplesSinceLastAnalysis;

        if (samplesSinceLastAnalysis >= hopSize)
        {
            samplesSinceLastAnalysis = 0;

            for (int j = 0; j < windowSize; ++j)
                analysisScratch[(size_t) j] = ringBuffer[(size_t) ((ringWriteIndex + j) % windowSize)];

            const auto result = pitchDetector.detectPitch (analysisScratch.data(), windowSize);

            latestFrequencyHz.store ((float) result.frequencyHz, std::memory_order_relaxed);
            latestConfidence.store ((float) result.confidence, std::memory_order_relaxed);
            signalPresent.store (result.pitchFound, std::memory_order_relaxed);
        }
    }
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        const float* inputData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
        appendToCaptureBuffer (inputData, bufferToFill.numSamples);
    }

    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
}

void MainComponent::timerCallback()
{
    const bool hasSignal = signalPresent.load (std::memory_order_relaxed);
    const double frequencyHz = (double) latestFrequencyHz.load (std::memory_order_relaxed);
    const auto match = hasSignal ? tuningEngine.evaluate (frequencyHz) : TuningMatchResult {};

    tunerDisplay.setResult (match, frequencyHz, hasSignal);
}

void MainComponent::showAudioSettings()
{
    audioSettings.setSize (500, 450);

    juce::DialogWindow::LaunchOptions options;
    options.content.setNonOwned (&audioSettings);
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    options.launchAsync();
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop (40);
    optionsButton.setBounds (headerBounds.removeFromLeft (100).reduced (5));
    tunerDisplay.setBounds (bounds);
}
