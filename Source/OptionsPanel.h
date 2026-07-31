#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <functional>

/*  Contents of the Options dialog: settings that are configured once and then left alone.

    Frequently changed settings — the tuning itself — deliberately live on the main screen
    instead, so switching tuning never costs a trip through a dialog.
*/
class OptionsPanel final : public juce::Component
{
public:
    explicit OptionsPanel (juce::AudioDeviceManager& deviceManagerToUse);

    /** Called on the message thread whenever the user moves the concert pitch control. */
    std::function<void (double)> onReferencePitchChanged;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    static constexpr double minReferencePitchHz = 415.0;
    static constexpr double maxReferencePitchHz = 466.0;
    static constexpr double defaultReferencePitchHz = 440.0;

    juce::Label referencePitchLabel;
    juce::Slider referencePitchSlider;
    juce::AudioDeviceSelectorComponent audioSettings;
};
