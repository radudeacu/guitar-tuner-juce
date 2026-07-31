#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>

#include "Tuning.h"

/*  A row of buttons, one per string, for auditioning target pitches.

    Purely a control surface: it reports which string the user wants to hear and reflects what
    is currently sounding, but owns no audio itself.
*/
class ReferenceToneBar final : public juce::Component
{
public:
    ReferenceToneBar();

    /** Relabels the buttons for a tuning. */
    void setTuning (const Tuning& tuning);

    /** Index of the string the user asked to hear. Clicking always sounds a note. */
    std::function<void (int)> onStringSelected;

    /** Whether repeated plucking was switched on or off. */
    std::function<void (bool)> onLoopToggled;

    void setActiveString (int stringIndex);
    int getActiveString() const noexcept;
    bool isLooping() const noexcept;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void handleButtonClick (int stringIndex);

    std::array<juce::TextButton, 6> stringButtons;
    juce::TextButton loopButton { "Loop" };
    int activeStringIndex = -1;
};
