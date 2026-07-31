#include "ReferenceToneBar.h"
#include "Theme.h"

ReferenceToneBar::ReferenceToneBar()
{
    for (int i = 0; i < (int) stringButtons.size(); ++i)
    {
        auto& button = stringButtons[(size_t) i];
        button.setClickingTogglesState (false); // toggle state mirrors what is sounding, not the click
        button.onClick = [this, i] { handleButtonClick (i); };
        addAndMakeVisible (button);
    }
}

void ReferenceToneBar::setTuning (const Tuning& tuning)
{
    const auto& strings = tuning.getStrings();

    for (size_t i = 0; i < stringButtons.size(); ++i)
        stringButtons[i].setButtonText (strings[i].noteName);
}

void ReferenceToneBar::handleButtonClick (int stringIndex)
{
    const int requested = (stringIndex == activeStringIndex) ? -1 : stringIndex;

    setActiveString (requested);

    if (onStringToggled != nullptr)
        onStringToggled (requested);
}

void ReferenceToneBar::setActiveString (int stringIndex)
{
    activeStringIndex = stringIndex;

    for (int i = 0; i < (int) stringButtons.size(); ++i)
        stringButtons[(size_t) i].setToggleState (i == activeStringIndex, juce::dontSendNotification);
}

int ReferenceToneBar::getActiveString() const noexcept
{
    return activeStringIndex;
}

void ReferenceToneBar::paint (juce::Graphics& g)
{
    g.setColour (Theme::Colours::textDim);
    g.setFont (Theme::labelFont (12.0f));
    g.drawText ("REFERENCE TONE",
                getLocalBounds().removeFromTop (16),
                juce::Justification::centred,
                false);
}

void ReferenceToneBar::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop (20); // caption

    const int gap = Theme::Spacing::xs;
    const int totalGaps = gap * ((int) stringButtons.size() - 1);
    const int buttonWidth = (bounds.getWidth() - totalGaps) / (int) stringButtons.size();

    for (auto& button : stringButtons)
    {
        button.setBounds (bounds.removeFromLeft (buttonWidth));
        bounds.removeFromLeft (gap);
    }
}
