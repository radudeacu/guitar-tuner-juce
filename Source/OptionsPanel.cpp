#include "OptionsPanel.h"
#include "GlassStyle.h"
#include "Theme.h"

OptionsPanel::OptionsPanel (juce::AudioDeviceManager& deviceManagerToUse)
    : audioSettings (deviceManagerToUse,
                     1, 1,     // min/max audio input channels
                     0, 0,     // min/max audio output channels
                     false,    // showMidiInputOptions
                     false,    // showMidiOutputSelector
                     true,     // showChannelsAsStereoPairs
                     false)    // hideAdvancedOptionsWithButton
{
    referencePitchLabel.setText ("Reference pitch (A4)", juce::dontSendNotification);
    referencePitchLabel.setFont (Theme::labelFont (14.0f));
    referencePitchLabel.setColour (juce::Label::textColourId, Theme::Colours::textSecondary);
    addAndMakeVisible (referencePitchLabel);

    // The range covers baroque pitch through to the sharper orchestral tunings, so players
    // matching a fixed-pitch instrument can follow it rather than fighting it.
    referencePitchSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    referencePitchSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 76, 24);
    referencePitchSlider.setRange (minReferencePitchHz, maxReferencePitchHz, 0.5);
    referencePitchSlider.setValue (defaultReferencePitchHz, juce::dontSendNotification);
    referencePitchSlider.setTextValueSuffix (" Hz");
    referencePitchSlider.setDoubleClickReturnValue (true, defaultReferencePitchHz);

    referencePitchSlider.onValueChange = [this]
    {
        if (onReferencePitchChanged != nullptr)
            onReferencePitchChanged (referencePitchSlider.getValue());
    };

    addAndMakeVisible (referencePitchSlider);
    addAndMakeVisible (audioSettings);

    setSize (540, 540);
}

void OptionsPanel::paint (juce::Graphics& g)
{
    GlassStyle::paintBackdrop (g, getLocalBounds().toFloat());
}

void OptionsPanel::resized()
{
    auto bounds = getLocalBounds().reduced (Theme::Spacing::md);

    referencePitchLabel.setBounds (bounds.removeFromTop (22));
    referencePitchSlider.setBounds (bounds.removeFromTop (32));

    bounds.removeFromTop (Theme::Spacing::md);
    audioSettings.setBounds (bounds);
}
