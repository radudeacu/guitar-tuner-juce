#include "TunerDisplayComponent.h"
#include "GlassStyle.h"
#include "Theme.h"

TunerDisplayComponent::TunerDisplayComponent()
{
    addAndMakeVisible (needleMeter);
}

void TunerDisplayComponent::setResult (const TuningMatchResult& result,
                                       double detectedFrequencyHz,
                                       bool hasSignal)
{
    showingResult = hasSignal && result.valid;

    if (showingResult)
    {
        noteName = result.noteName;
        cents = result.cents;
        frequencyHz = detectedFrequencyHz;
    }

    needleMeter.setCents (result.cents, showingResult);
    repaint();
}

void TunerDisplayComponent::snapToCurrentResult()
{
    needleMeter.snapToTarget();
}

juce::String TunerDisplayComponent::notePitchClass() const
{
    // A single en dash reads as a deliberate placeholder; a pair of hyphens looks like a glitch.
    if (! showingResult)
        return juce::String::charToString (static_cast<juce::juce_wchar> (0x2013));

    // Note names arrive as e.g. "C#3"; strip the trailing octave digits.
    return noteName.retainCharacters ("ABCDEFG#b");
}

juce::String TunerDisplayComponent::noteOctave() const
{
    if (! showingResult)
        return {};

    return noteName.retainCharacters ("0123456789");
}

void TunerDisplayComponent::paintNoteName (juce::Graphics& g, juce::Rectangle<int> area) const
{
    const auto colour = showingResult ? Theme::Colours::textPrimary : Theme::Colours::textDim;
    const auto pitchClass = notePitchClass();
    const auto octave = noteOctave();

    const auto nameFont = Theme::displayFont ((float) area.getHeight() * 0.82f);
    const auto octaveFont = Theme::displayFont ((float) area.getHeight() * 0.30f);

    const int nameWidth = juce::GlyphArrangement::getStringWidthInt (nameFont, pitchClass);
    const int octaveWidth = octave.isEmpty()
                              ? 0
                              : juce::GlyphArrangement::getStringWidthInt (octaveFont, octave) + Theme::Spacing::xs;

    // Centre the note and octave as one unit so the glyph block never drifts off-axis.
    auto block = area.withWidth (nameWidth + octaveWidth).withX (area.getCentreX() - (nameWidth + octaveWidth) / 2);

    g.setColour (colour);
    g.setFont (nameFont);
    g.drawText (pitchClass, block.removeFromLeft (nameWidth), juce::Justification::centred, false);

    if (octave.isNotEmpty())
    {
        g.setColour (Theme::Colours::textSecondary);
        g.setFont (octaveFont);
        g.drawText (octave, block, juce::Justification::centredBottom, false);
    }
}

void TunerDisplayComponent::paintCents (juce::Graphics& g, juce::Rectangle<int> area) const
{
    // With no signal the frequency line already says so; a second placeholder just adds noise.
    if (! showingResult)
        return;

    g.setFont (Theme::labelFont (22.0f));
    g.setColour (Theme::tuneColour (cents, true));
    g.drawText (juce::String::formatted ("%+.1f cents", cents), area, juce::Justification::centred, false);
}

void TunerDisplayComponent::paintFrequency (juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setFont (Theme::bodyFont (15.0f));
    g.setColour (Theme::Colours::textSecondary);
    g.drawText (showingResult ? juce::String::formatted ("%.2f Hz", frequencyHz) : juce::String ("no signal"),
                area, juce::Justification::centred, false);
}

void TunerDisplayComponent::paintStringHint (juce::Graphics& g, juce::Rectangle<int> area) const
{
    if (! showingResult)
        return;

    // Tells the player which way to turn the peg; the arc alone reads as direction-neutral.
    const bool isFlat = cents < -Theme::Tolerance::inTuneCents;
    const bool isSharp = cents > Theme::Tolerance::inTuneCents;

    if (! isFlat && ! isSharp)
        return;

    g.setFont (Theme::labelFont (13.0f));
    g.setColour (Theme::tuneColour (cents, true).withAlpha (0.85f));
    g.drawText (isFlat ? "TUNE UP" : "TUNE DOWN", area, juce::Justification::centred, false);
}

TunerDisplayComponent::Layout TunerDisplayComponent::computeLayout() const
{
    const auto inner = getLocalBounds().reduced (Theme::Spacing::lg, Theme::Spacing::md);

    // Elements keep a fixed relationship to each other and the whole stack is centred, so the
    // composition stays balanced instead of stretching apart in a tall window.
    const int meterHeight = juce::jlimit (120, 250, juce::roundToInt (inner.getHeight() * 0.34f));
    const int noteHeight  = juce::jlimit (90, 170, juce::roundToInt (inner.getHeight() * 0.24f));
    constexpr int hintHeight = 20;
    constexpr int centsHeight = 34;
    constexpr int frequencyHeight = 22;

    const int stackHeight = meterHeight + Theme::Spacing::md + noteHeight + Theme::Spacing::xs
                          + hintHeight + centsHeight + frequencyHeight;

    auto block = inner.withHeight (stackHeight)
                      .withY (inner.getY() + juce::jmax (0, (inner.getHeight() - stackHeight) / 2));

    Layout layout;
    layout.meter = block.removeFromTop (meterHeight);
    block.removeFromTop (Theme::Spacing::md);
    layout.note = block.removeFromTop (noteHeight);
    block.removeFromTop (Theme::Spacing::xs);
    layout.hint = block.removeFromTop (hintHeight);
    layout.cents = block.removeFromTop (centsHeight);
    layout.frequency = block.removeFromTop (frequencyHeight);

    return layout;
}

void TunerDisplayComponent::paint (juce::Graphics& g)
{
    GlassStyle::paintSurface (g, getLocalBounds().toFloat().reduced (0.5f), Theme::Radius::card);

    const auto layout = computeLayout();

    paintNoteName (g, layout.note);
    paintStringHint (g, layout.hint);
    paintCents (g, layout.cents);
    paintFrequency (g, layout.frequency);
}

void TunerDisplayComponent::resized()
{
    needleMeter.setBounds (computeLayout().meter);
}
