#include "GlassStyle.h"
#include "Theme.h"

namespace
{
    /*  Blobs are placed proportionally so the composition survives any window size.
        Radius is expressed as a fraction of the larger window dimension.
    */
    struct AuroraBlob
    {
        float relativeX, relativeY, relativeRadius;
        juce::Colour colour;
        float alpha;
    };

    const AuroraBlob auroraBlobs[] =
    {
        { 0.16f, 0.20f, 0.62f, Theme::Colours::auroraIndigo, 0.30f },
        { 0.88f, 0.12f, 0.50f, Theme::Colours::auroraTeal,   0.22f },
        { 0.72f, 0.92f, 0.66f, Theme::Colours::auroraViolet, 0.26f }
    };

    void paintRadialBlob (juce::Graphics& g,
                          juce::Point<float> centre,
                          float radius,
                          juce::Colour colour,
                          float alpha)
    {
        juce::ColourGradient gradient (colour.withAlpha (alpha), centre,
                                        colour.withAlpha (0.0f), centre.translated (radius, 0.0f),
                                        true);

        // A mid stop keeps the falloff soft instead of a hard-edged disc.
        gradient.addColour (0.55, colour.withAlpha (alpha * 0.35f));

        g.setGradientFill (gradient);
        g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre));
    }
}

namespace GlassStyle
{
    void paintBackdrop (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setGradientFill (juce::ColourGradient (Theme::Colours::backdropTop, bounds.getTopLeft(),
                                                  Theme::Colours::backdropBottom, bounds.getBottomLeft(),
                                                  false));
        g.fillRect (bounds);

        const float reference = juce::jmax (bounds.getWidth(), bounds.getHeight());

        for (const auto& blob : auroraBlobs)
            paintRadialBlob (g,
                             { bounds.getX() + bounds.getWidth() * blob.relativeX,
                               bounds.getY() + bounds.getHeight() * blob.relativeY },
                             reference * blob.relativeRadius,
                             blob.colour,
                             blob.alpha);
    }

    void paintSurface (juce::Graphics& g,
                       juce::Rectangle<float> bounds,
                       float cornerRadius,
                       bool raised)
    {
        juce::Path shape;
        shape.addRoundedRectangle (bounds, cornerRadius);

        // Vertical tint: slightly brighter at the top, mimicking light falling from above.
        const auto base = raised ? Theme::Colours::glassFillRaised : Theme::Colours::glassFill;
        g.setGradientFill (juce::ColourGradient (base.withMultipliedAlpha (1.35f), bounds.getTopLeft(),
                                                  base.withMultipliedAlpha (0.75f), bounds.getBottomLeft(),
                                                  false));
        g.fillPath (shape);

        g.setColour (Theme::Colours::glassBorder);
        g.strokePath (shape, juce::PathStrokeType (1.0f));

        // A short bright arc across the top edge reads as a specular catch on a bevelled edge.
        juce::Path highlight;
        const float inset = cornerRadius * 0.6f;
        highlight.startNewSubPath (bounds.getX() + inset, bounds.getY() + 0.75f);
        highlight.lineTo (bounds.getRight() - inset, bounds.getY() + 0.75f);

        g.setGradientFill (juce::ColourGradient (Theme::Colours::glassHighlight.withAlpha (0.0f),
                                                  bounds.getX() + inset, 0.0f,
                                                  Theme::Colours::glassHighlight.withAlpha (0.0f),
                                                  bounds.getRight() - inset, 0.0f,
                                                  false));
        g.setColour (Theme::Colours::glassHighlight);
        g.strokePath (highlight, juce::PathStrokeType (1.5f));
    }

    void paintGlow (juce::Graphics& g,
                    juce::Rectangle<float> bounds,
                    juce::Colour colour,
                    float intensity)
    {
        if (intensity <= 0.001f)
            return;

        const auto centre = bounds.getCentre();
        const float radius = juce::jmax (bounds.getWidth(), bounds.getHeight()) * 0.5f;

        paintRadialBlob (g, centre, radius, colour, juce::jlimit (0.0f, 1.0f, intensity));
    }
}
