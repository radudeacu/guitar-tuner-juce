#include "TunerLookAndFeel.h"
#include "GlassStyle.h"
#include "Theme.h"

TunerLookAndFeel::TunerLookAndFeel()
{
    setColourScheme (juce::LookAndFeel_V4::getDarkColourScheme());

    setColour (juce::ResizableWindow::backgroundColourId, Theme::Colours::backdropTop);
    setColour (juce::DocumentWindow::textColourId,        Theme::Colours::textPrimary);

    setColour (juce::Label::textColourId,                 Theme::Colours::textSecondary);

    setColour (juce::TextButton::textColourOffId,         Theme::Colours::textPrimary);
    setColour (juce::TextButton::textColourOnId,          Theme::Colours::textPrimary);

    setColour (juce::ComboBox::textColourId,              Theme::Colours::textPrimary);
    setColour (juce::ComboBox::backgroundColourId,        juce::Colours::transparentBlack);
    setColour (juce::ComboBox::outlineColourId,           Theme::Colours::glassBorder);
    setColour (juce::ComboBox::arrowColourId,             Theme::Colours::textSecondary);

    setColour (juce::PopupMenu::backgroundColourId,       Theme::Colours::backdropBottom);
    setColour (juce::PopupMenu::textColourId,             Theme::Colours::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::Colours::auroraIndigo.withAlpha (0.35f));

    setColour (juce::ToggleButton::textColourId,          Theme::Colours::textSecondary);
    setColour (juce::ToggleButton::tickColourId,          Theme::Colours::inTune);
    setColour (juce::ToggleButton::tickDisabledColourId,  Theme::Colours::glassBorder);
}

void TunerLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                             juce::Button& button,
                                             const juce::Colour& /*backgroundColour*/,
                                             bool shouldDrawAsHighlighted,
                                             bool shouldDrawAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

    GlassStyle::paintSurface (g, bounds, Theme::Radius::control, shouldDrawAsHighlighted);

    // Pressing dims rather than shifts hue, keeping the glass reading intact.
    if (shouldDrawAsDown)
    {
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.fillRoundedRectangle (bounds, Theme::Radius::control);
    }
}

void TunerLookAndFeel::drawComboBox (juce::Graphics& g,
                                     int width,
                                     int height,
                                     bool /*isButtonDown*/,
                                     int /*buttonX*/,
                                     int /*buttonY*/,
                                     int /*buttonW*/,
                                     int /*buttonH*/,
                                     juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);

    GlassStyle::paintSurface (g, bounds, Theme::Radius::control, box.isMouseOver());

    juce::Path arrow;
    const float arrowX = bounds.getRight() - 22.0f;
    const float arrowY = bounds.getCentreY() - 2.0f;
    arrow.startNewSubPath (arrowX, arrowY);
    arrow.lineTo (arrowX + 5.0f, arrowY + 5.0f);
    arrow.lineTo (arrowX + 10.0f, arrowY);

    g.setColour (findColour (juce::ComboBox::arrowColourId));
    g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void TunerLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);

    g.setColour (Theme::Colours::backdropBottom.withAlpha (0.97f));
    g.fillRoundedRectangle (bounds, Theme::Radius::panel);

    g.setColour (Theme::Colours::glassBorder);
    g.drawRoundedRectangle (bounds.reduced (0.5f), Theme::Radius::panel, 1.0f);
}

juce::Font TunerLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return Theme::labelFont (juce::jmin (15.0f, (float) buttonHeight * 0.45f));
}

juce::Font TunerLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return Theme::bodyFont (juce::jmin (15.0f, (float) box.getHeight() * 0.5f));
}

juce::Font TunerLookAndFeel::getLabelFont (juce::Label& label)
{
    return Theme::bodyFont (label.getFont().getHeight());
}
