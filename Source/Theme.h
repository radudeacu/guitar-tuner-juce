#pragma once

#include <juce_graphics/juce_graphics.h>

/*  Central design tokens for the whole app.

    Every colour, radius, spacing step and font used by a component should come from here
    rather than being written inline, so the visual language stays consistent and a restyle
    means editing one file.
*/
namespace Theme
{
    namespace Colours
    {
        // Backdrop — a deep, slightly blue-shifted charcoal so the coloured glow reads as light.
        const juce::Colour backdropTop    { 0xff0b0f1a };
        const juce::Colour backdropBottom { 0xff151c2e };

        // Aurora blobs sit behind the glass; heavily transparent so they bleed rather than block.
        const juce::Colour auroraTeal   { 0xff2dd4bf };
        const juce::Colour auroraIndigo { 0xff6366f1 };
        const juce::Colour auroraViolet { 0xffa855f7 };

        // Glass surfaces are white at very low alpha — the backdrop supplies the colour.
        const juce::Colour glassFill      = juce::Colours::white.withAlpha (0.055f);
        const juce::Colour glassFillRaised = juce::Colours::white.withAlpha (0.085f);
        const juce::Colour glassBorder    = juce::Colours::white.withAlpha (0.14f);
        const juce::Colour glassHighlight = juce::Colours::white.withAlpha (0.30f);

        const juce::Colour textPrimary   { 0xfff8fafc };
        const juce::Colour textSecondary { 0xff94a3b8 };
        const juce::Colour textDim       { 0xff64748b };

        // Tuning feedback. Desaturated slightly versus pure RGB so they sit calmly on dark glass.
        const juce::Colour inTune  { 0xff34d399 };
        const juce::Colour nearby  { 0xfffbbf24 };
        const juce::Colour farOff  { 0xfffb7185 };
        const juce::Colour idle    { 0xff475569 };
    }

    namespace Radius
    {
        constexpr float card = 28.0f;
        constexpr float panel = 18.0f;
        constexpr float control = 12.0f;
    }

    namespace Spacing
    {
        constexpr int xs = 6;
        constexpr int sm = 12;
        constexpr int md = 20;
        constexpr int lg = 32;
        constexpr int xl = 48;
    }

    namespace Tolerance
    {
        constexpr double inTuneCents = 5.0;
        constexpr double nearbyCents = 20.0;
        constexpr double displayRangeCents = 50.0;
    }

    inline juce::Font displayFont (float height)
    {
        return juce::Font (juce::FontOptions ("Segoe UI", "Light", height));
    }

    inline juce::Font bodyFont (float height)
    {
        return juce::Font (juce::FontOptions ("Segoe UI", "Regular", height));
    }

    inline juce::Font labelFont (float height)
    {
        return juce::Font (juce::FontOptions ("Segoe UI", "Semibold", height));
    }

    /** Maps how far off pitch we are onto the feedback palette. */
    inline juce::Colour tuneColour (double cents, bool hasSignal)
    {
        if (! hasSignal)
            return Colours::idle;

        const double absCents = std::abs (cents);

        if (absCents <= Tolerance::inTuneCents)
            return Colours::inTune;

        if (absCents <= Tolerance::nearbyCents)
            return Colours::nearby;

        return Colours::farOff;
    }
}
