#include "NeedleMeterComponent.h"

void NeedleMeterComponent::setCents (double newCents, bool newSignalPresent)
{
    signalPresent = newSignalPresent;
    cents = signalPresent ? juce::jlimit (-displayRangeCents, displayRangeCents, newCents) : 0.0;
    repaint();
}

juce::Colour NeedleMeterComponent::colourForCents (double centsValue, bool hasSignal)
{
    if (! hasSignal)
        return juce::Colours::grey;

    const double absCents = std::abs (centsValue);

    if (absCents <= greenThresholdCents)
        return juce::Colours::limegreen;

    if (absCents <= yellowThresholdCents)
        return juce::Colours::gold;

    return juce::Colours::red;
}

void NeedleMeterComponent::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (10.0f);
    const float centreY = bounds.getCentreY();
    const float trackLeft = bounds.getX();
    const float trackRight = bounds.getRight();
    const float trackWidth = trackRight - trackLeft;

    g.setColour (juce::Colours::darkgrey);
    g.drawLine (trackLeft, centreY, trackRight, centreY, 2.0f);

    for (int tick = -50; tick <= 50; tick += 25)
    {
        const float x = trackLeft + trackWidth * (float) (tick + 50) / 100.0f;
        g.drawLine (x, centreY - 8.0f, x, centreY + 8.0f, 2.0f);
    }

    const float greenHalfWidth = trackWidth * (float) (greenThresholdCents / (displayRangeCents * 2.0));
    const float centreX = trackLeft + trackWidth * 0.5f;
    g.setColour (juce::Colours::limegreen.withAlpha (0.25f));
    g.fillRect (juce::Rectangle<float> (centreX - greenHalfWidth, centreY - 4.0f, greenHalfWidth * 2.0f, 8.0f));

    const float indicatorX = trackLeft + trackWidth * (float) ((cents + displayRangeCents) / (displayRangeCents * 2.0));
    const auto indicatorColour = colourForCents (cents, signalPresent);

    g.setColour (indicatorColour);
    juce::Path indicator;
    indicator.addTriangle (indicatorX, centreY - 20.0f,
                            indicatorX - 8.0f, centreY - 4.0f,
                            indicatorX + 8.0f, centreY - 4.0f);
    g.fillPath (indicator);
    g.drawLine (indicatorX, centreY - 4.0f, indicatorX, centreY + 4.0f, 3.0f);
}
