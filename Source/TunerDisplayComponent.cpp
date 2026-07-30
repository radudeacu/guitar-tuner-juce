#include "TunerDisplayComponent.h"

namespace
{
    juce::Colour colourForCents (double cents, bool hasSignal)
    {
        if (! hasSignal)
            return juce::Colours::grey;

        const double absCents = std::abs (cents);

        if (absCents <= 5.0)
            return juce::Colours::limegreen;

        if (absCents <= 20.0)
            return juce::Colours::gold;

        return juce::Colours::red;
    }
}

TunerDisplayComponent::TunerDisplayComponent()
{
    noteNameLabel.setJustificationType (juce::Justification::centred);
    noteNameLabel.setFont (juce::FontOptions (64.0f, juce::Font::bold));
    noteNameLabel.setText ("--", juce::dontSendNotification);
    addAndMakeVisible (noteNameLabel);

    centsLabel.setJustificationType (juce::Justification::centred);
    centsLabel.setFont (juce::FontOptions (20.0f));
    addAndMakeVisible (centsLabel);

    hzLabel.setJustificationType (juce::Justification::centred);
    hzLabel.setFont (juce::FontOptions (16.0f));
    addAndMakeVisible (hzLabel);

    addAndMakeVisible (needleMeter);
}

void TunerDisplayComponent::setResult (const TuningMatchResult& result, double detectedFrequencyHz, bool hasSignal)
{
    const bool showResult = hasSignal && result.valid;
    const auto colour = colourForCents (result.cents, showResult);

    if (! showResult)
    {
        noteNameLabel.setText ("--", juce::dontSendNotification);
        centsLabel.setText ("", juce::dontSendNotification);
        hzLabel.setText ("", juce::dontSendNotification);
    }
    else
    {
        noteNameLabel.setText (result.noteName, juce::dontSendNotification);
        centsLabel.setText (juce::String::formatted ("%+.1f cents", result.cents), juce::dontSendNotification);
        hzLabel.setText (juce::String::formatted ("%.2f Hz", detectedFrequencyHz), juce::dontSendNotification);
    }

    noteNameLabel.setColour (juce::Label::textColourId, colour);
    needleMeter.setCents (result.cents, showResult);
}

void TunerDisplayComponent::resized()
{
    auto bounds = getLocalBounds();
    noteNameLabel.setBounds (bounds.removeFromTop (120));
    centsLabel.setBounds (bounds.removeFromTop (40));
    hzLabel.setBounds (bounds.removeFromTop (30));
    needleMeter.setBounds (bounds.removeFromTop (100).reduced (20));
}
