/*  Development tool — not part of the shipped app.

    Renders the tuner display in several tuning states to a single PNG so the visual design can
    be reviewed without a guitar plugged in. Build the DesignPreview target and run it; it writes
    design-preview.png next to the executable.
*/

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Source/GlassStyle.h"
#include "../Source/OptionsPanel.h"
#include "../Source/Theme.h"
#include "../Source/TunerDisplayComponent.h"
#include "../Source/TunerLookAndFeel.h"

namespace
{
    struct PreviewState
    {
        const char* label;
        const char* noteName;
        double cents;
        double frequencyHz;
        bool hasSignal;
    };

    const PreviewState previewStates[] =
    {
        { "In tune",        "A2",  0.8,  110.05, true  },
        { "Slightly sharp", "D3",  13.5, 148.00, true  },
        { "Very flat",      "E2", -37.0,  80.60, true  },
        { "No signal",      "",     0.0,   0.00, false }
    };

    constexpr int cellWidth = 440;
    constexpr int cellHeight = 520;
    constexpr int labelHeight = 30;
    constexpr int columns = 2;

    void paintCellLabel (juce::Graphics& g, juce::Rectangle<int> area, const char* label)
    {
        g.setColour (Theme::Colours::textSecondary);
        g.setFont (Theme::labelFont (14.0f));
        g.drawText (label, area, juce::Justification::centredLeft, false);
    }

    void writePng (const juce::Image& image, const juce::String& fileName)
    {
        const auto file = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                              .getSiblingFile (fileName);
        file.deleteFile();

        if (auto stream = file.createOutputStream())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream (image, *stream);
        }

        std::printf ("Wrote %s\n", file.getFullPathName().toRawUTF8());
    }
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    TunerLookAndFeel lookAndFeel;
    juce::Desktop::getInstance().setDefaultLookAndFeel (&lookAndFeel);

    const int numStates = juce::numElementsInArray (previewStates);
    const int rows = (numStates + columns - 1) / columns;

    juce::OwnedArray<TunerDisplayComponent> displays;

    for (const auto& state : previewStates)
    {
        auto* display = displays.add (new TunerDisplayComponent());
        display->setBounds (0, 0, cellWidth, cellHeight);
        display->setVisible (true); // child components are skipped when painting an invisible parent

        TuningMatchResult result;
        result.noteName = state.noteName;
        result.cents = state.cents;
        result.valid = state.hasSignal;

        display->setResult (result, state.frequencyHz, state.hasSignal);

        // Skip the easing animation so the needle renders at its resting position.
        display->snapToCurrentResult();
    }

    const int imageWidth = cellWidth * columns;
    const int imageHeight = (cellHeight + labelHeight) * rows;

    juce::Image image (juce::Image::ARGB, imageWidth, imageHeight, true);

    // Scoped so the graphics context is released — and any buffered drawing flushed to the
    // image — before the file is written.
    {
        juce::Graphics g (image);

        GlassStyle::paintBackdrop (g, juce::Rectangle<float> (0.0f, 0.0f, (float) imageWidth, (float) imageHeight));

        for (int i = 0; i < numStates; ++i)
        {
            const int column = i % columns;
            const int row = i / columns;
            const int x = column * cellWidth;
            const int y = row * (cellHeight + labelHeight);

            paintCellLabel (g, { x + 16, y, cellWidth - 32, labelHeight }, previewStates[i].label);

            g.saveState();
            g.setOrigin (x, y + labelHeight);
            displays[i]->paintEntireComponent (g, false);
            g.restoreState();
        }
    }

    writePng (image, "design-preview.png");

    // The options panel is rendered separately. Its device manager is deliberately left
    // uninitialised — this only needs to verify layout, not enumerate real hardware.
    {
        juce::AudioDeviceManager deviceManager;
        OptionsPanel optionsPanel (deviceManager);
        optionsPanel.setVisible (true);

        juce::Image optionsImage (juce::Image::ARGB, optionsPanel.getWidth(), optionsPanel.getHeight(), true);

        {
            juce::Graphics optionsGraphics (optionsImage);
            optionsPanel.paintEntireComponent (optionsGraphics, false);
        }

        writePng (optionsImage, "options-preview.png");
    }

    juce::Desktop::getInstance().setDefaultLookAndFeel (nullptr);
    return 0;
}
