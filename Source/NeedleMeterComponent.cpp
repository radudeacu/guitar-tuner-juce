#include "NeedleMeterComponent.h"
#include "GlassStyle.h"
#include "Theme.h"

namespace
{
    constexpr int animationHz = 60;

    float easeToward (float current, float target, float factor)
    {
        return current + (target - current) * factor;
    }
}

NeedleMeterComponent::NeedleMeterComponent()
{
    startTimerHz (animationHz);
}

void NeedleMeterComponent::setCents (double newCents, bool newSignalPresent)
{
    signalPresent = newSignalPresent;
    targetPresence = newSignalPresent ? 1.0f : 0.0f;

    // Returning to centre when the note dies away keeps the needle consistent with the
    // "no signal" readout; holding the last position reads as a frozen display.
    targetCents = newSignalPresent
                    ? juce::jlimit (-Theme::Tolerance::displayRangeCents,
                                     Theme::Tolerance::displayRangeCents,
                                     newCents)
                    : 0.0;
}

void NeedleMeterComponent::snapToTarget()
{
    displayedCents = targetCents;
    displayedPresence = targetPresence;
    repaint();
}

bool NeedleMeterComponent::advanceAnimation()
{
    const auto previousCents = displayedCents;
    const auto previousPresence = displayedPresence;

    displayedCents = easeToward ((float) displayedCents, (float) targetCents, easingFactor);
    displayedPresence = easeToward (displayedPresence, targetPresence, easingFactor * 0.6f);

    return std::abs (displayedCents - previousCents) > 0.01
        || std::abs (displayedPresence - previousPresence) > 0.002f;
}

void NeedleMeterComponent::timerCallback()
{
    // Only repaint while something is actually moving, so an idle tuner costs nothing.
    if (advanceAnimation())
        repaint();
}

float NeedleMeterComponent::angleForCents (double cents) const
{
    const double normalised = cents / Theme::Tolerance::displayRangeCents;
    return juce::degreesToRadians ((float) normalised * sweepDegrees);
}

juce::Colour NeedleMeterComponent::currentColour() const
{
    const auto active = Theme::tuneColour (displayedCents, signalPresent);
    return active.interpolatedWith (Theme::Colours::idle, 1.0f - displayedPresence);
}

void NeedleMeterComponent::paintTrack (juce::Graphics& g, juce::Point<float> centre, float radius) const
{
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f,
                          angleForCents (-Theme::Tolerance::displayRangeCents),
                          angleForCents (Theme::Tolerance::displayRangeCents),
                          true);

    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.strokePath (track, juce::PathStrokeType (trackThickness,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
}

void NeedleMeterComponent::paintInTuneZone (juce::Graphics& g, juce::Point<float> centre, float radius) const
{
    juce::Path zone;
    zone.addCentredArc (centre.x, centre.y, radius, radius, 0.0f,
                         angleForCents (-Theme::Tolerance::inTuneCents),
                         angleForCents (Theme::Tolerance::inTuneCents),
                         true);

    g.setColour (Theme::Colours::inTune.withAlpha (0.25f + 0.35f * displayedPresence));
    g.strokePath (zone, juce::PathStrokeType (trackThickness,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

void NeedleMeterComponent::paintTicks (juce::Graphics& g, juce::Point<float> centre, float radius) const
{
    for (int cents = -50; cents <= 50; cents += 25)
    {
        const float angle = angleForCents (cents);
        const bool isCentre = (cents == 0);
        const float inner = radius + trackThickness * 0.5f + 3.0f;
        const float outer = inner + (isCentre ? 11.0f : 6.0f);

        const juce::Point<float> from { centre.x + std::sin (angle) * inner,
                                        centre.y - std::cos (angle) * inner };
        const juce::Point<float> to   { centre.x + std::sin (angle) * outer,
                                        centre.y - std::cos (angle) * outer };

        g.setColour (isCentre ? Theme::Colours::textSecondary
                              : juce::Colours::white.withAlpha (0.22f));
        g.drawLine ({ from, to }, isCentre ? 2.5f : 1.5f);
    }
}

void NeedleMeterComponent::paintNeedle (juce::Graphics& g, juce::Point<float> centre, float radius) const
{
    const float angle = angleForCents (displayedCents);
    const auto colour = currentColour();

    const juce::Point<float> direction { std::sin (angle), -std::cos (angle) };
    const juce::Point<float> perpendicular { -direction.y, direction.x };

    // The needle runs the whole way from the hub to the track, tapering as it goes, so the
    // pivot and pointer read as one object rather than two floating marks.
    const auto baseCentre = centre + direction * hubRadius;
    const auto tipCentre = centre + direction * (radius + trackThickness * 0.5f - 2.0f);

    juce::Path needle;
    needle.startNewSubPath (baseCentre + perpendicular * 4.5f);
    needle.lineTo (tipCentre + perpendicular * 1.5f);
    needle.lineTo (tipCentre - perpendicular * 1.5f);
    needle.lineTo (baseCentre - perpendicular * 4.5f);
    needle.closeSubPath();

    // A soft wide stroke under the solid shape fakes a bloom without an image filter.
    g.setColour (colour.withAlpha (0.22f * displayedPresence));
    g.strokePath (needle, juce::PathStrokeType (10.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (colour.withAlpha (0.4f + 0.6f * displayedPresence));
    g.fillPath (needle);

    const auto hubBounds = juce::Rectangle<float> (hubRadius * 2.0f, hubRadius * 2.0f).withCentre (centre);
    g.setColour (colour.withAlpha (0.45f + 0.5f * displayedPresence));
    g.fillEllipse (hubBounds);

    g.setColour (juce::Colours::white.withAlpha (0.45f));
    g.drawEllipse (hubBounds, 1.0f);
}

void NeedleMeterComponent::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Pivot sits below the visible area so the arc reads as the top of a large dial.
    const float radius = juce::jmin (bounds.getWidth() * 0.42f, bounds.getHeight() * 0.92f);
    const juce::Point<float> centre { bounds.getCentreX(), bounds.getBottom() - 12.0f };

    GlassStyle::paintGlow (g,
                           juce::Rectangle<float> (radius * 1.6f, radius * 1.6f).withCentre (centre),
                           currentColour(),
                           0.16f * displayedPresence);

    paintTrack (g, centre, radius);
    paintInTuneZone (g, centre, radius);
    paintTicks (g, centre, radius);
    paintNeedle (g, centre, radius);
}
