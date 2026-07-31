#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*  Applies the glass design language to stock JUCE controls.

    Installed as the default LookAndFeel so the audio settings dialog — which is built from
    JUCE's own components — matches the main screen without being restyled by hand.
*/
class TunerLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    TunerLookAndFeel();

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawAsHighlighted,
                               bool shouldDrawAsDown) override;

    void drawComboBox (juce::Graphics& g,
                       int width,
                       int height,
                       bool isButtonDown,
                       int buttonX,
                       int buttonY,
                       int buttonW,
                       int buttonH,
                       juce::ComboBox& box) override;

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;

    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;
    juce::Font getLabelFont (juce::Label& label) override;
};
