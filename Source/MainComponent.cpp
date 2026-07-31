#include "MainComponent.h"
#include "GlassStyle.h"
#include "Theme.h"
#include "TuningPresets.h"

MainComponent::MainComponent()
    : availableTunings (TuningPresets::all()),
      optionsPanel (deviceManager)
{
    setOpaque (true);

    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this] { showOptions(); };

    for (int i = 0; i < (int) availableTunings.size(); ++i)
        tuningSelector.addItem (availableTunings[(size_t) i].getName(), i + 1);

    tuningSelector.setSelectedId (1, juce::dontSendNotification);
    tuningSelector.onChange = [this] { applySelectedTuning(); };
    addAndMakeVisible (tuningSelector);

    optionsPanel.onReferencePitchChanged = [this] (double referencePitchHz)
    {
        tuningEngine.setReferenceA4 (referencePitchHz);
        updateReferenceToneFrequency();
    };

    referenceToneBar.setTuning (tuningEngine.getTuning());
    referenceToneBar.onStringToggled = [this] (int stringIndex)
    {
        updateReferenceToneFrequency();
        referenceTonePlayer.setPlaying (stringIndex >= 0);
    };
    addAndMakeVisible (referenceToneBar);

    addAndMakeVisible (tunerDisplay);

    // Two output channels so the reference tone has somewhere to go; the input passthrough
    // this enables is overwritten every block in getNextAudioBlock.
    setAudioChannels (1, 2);
    setSize (820, 780);

    startTimerHz (30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    pitchDetector.prepare (sampleRate, windowSize);
    referenceTonePlayer.prepare (sampleRate, samplesPerBlockExpected);

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
    /*  Ordering here is load-bearing. JUCE copies the live input into the same buffer it will
        send to the output before this callback runs, so the guitar signal must be captured
        first, then the buffer overwritten — never mixed into.

        Overwriting does two jobs at once: it stops the raw input passing through to the
        speakers, and it guarantees the reference tone cannot reach the detector's input.
    */
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        const float* inputData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
        appendToCaptureBuffer (inputData, bufferToFill.numSamples);
    }

    bufferToFill.clearActiveBufferRegion();

    referenceTonePlayer.renderNextBlock (*bufferToFill.buffer,
                                          bufferToFill.startSample,
                                          bufferToFill.numSamples);
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

void MainComponent::applySelectedTuning()
{
    const int index = tuningSelector.getSelectedId() - 1;

    if (! juce::isPositiveAndBelow (index, (int) availableTunings.size()))
        return;

    tuningEngine.setTuning (availableTunings[(size_t) index]);
    referenceToneBar.setTuning (tuningEngine.getTuning());

    // A sounding tone follows the new tuning rather than being left on the old pitch.
    updateReferenceToneFrequency();
}

void MainComponent::updateReferenceToneFrequency()
{
    const int activeString = referenceToneBar.getActiveString();

    if (activeString >= 0)
        referenceTonePlayer.setFrequency (tuningEngine.getStringFrequencyHz (activeString));
}

void MainComponent::showOptions()
{
    juce::DialogWindow::LaunchOptions options;
    options.content.setNonOwned (&optionsPanel);
    options.dialogTitle = "Options";
    options.dialogBackgroundColour = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    options.launchAsync();
}

void MainComponent::refreshBackdropCacheIfNeeded()
{
    if (backdropCache.isValid()
        && backdropCache.getWidth() == getWidth()
        && backdropCache.getHeight() == getHeight())
        return;

    if (getWidth() <= 0 || getHeight() <= 0)
        return;

    backdropCache = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), false);
    juce::Graphics cacheGraphics (backdropCache);
    GlassStyle::paintBackdrop (cacheGraphics, getLocalBounds().toFloat());
}

void MainComponent::paint (juce::Graphics& g)
{
    refreshBackdropCacheIfNeeded();

    if (backdropCache.isValid())
        g.drawImageAt (backdropCache, 0, 0);
}

void MainComponent::resized()
{
    backdropCache = {};

    auto bounds = getLocalBounds().reduced (Theme::Spacing::md);

    auto header = bounds.removeFromTop (44);
    optionsButton.setBounds (header.removeFromLeft (108).withSizeKeepingCentre (108, 36));
    tuningSelector.setBounds (header.removeFromRight (190).withSizeKeepingCentre (190, 36));

    bounds.removeFromTop (Theme::Spacing::sm);

    referenceToneBar.setBounds (bounds.removeFromBottom (76));
    bounds.removeFromBottom (Theme::Spacing::sm);

    tunerDisplay.setBounds (bounds);
}
