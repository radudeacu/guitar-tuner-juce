#pragma once

#include <juce_graphics/juce_graphics.h>

/*  Painting primitives for the glass-morphism look.

    These are free functions rather than a base Component so any component can opt into the
    surface style inside its own paint() without inheriting from anything.
*/
namespace GlassStyle
{
    /** Paints the app backdrop: a vertical gradient with soft colour blobs behind it.

        Glass only reads as glass when there is something worth seeing through it, so the
        backdrop deliberately carries colour rather than being flat.
    */
    void paintBackdrop (juce::Graphics& g, juce::Rectangle<float> bounds);

    /** Paints a translucent glass surface: fill, hairline border and a top-edge light catch. */
    void paintSurface (juce::Graphics& g,
                       juce::Rectangle<float> bounds,
                       float cornerRadius,
                       bool raised = false);

    /** Paints a soft coloured halo, used to make active elements glow through the glass. */
    void paintGlow (juce::Graphics& g,
                    juce::Rectangle<float> bounds,
                    juce::Colour colour,
                    float intensity);
}
