#pragma once

#include <juce_data_structures/juce_data_structures.h>

#include <memory>

/*  Settings that survive between launches.

    Backed by a properties file in the user's application data folder. Writes are marked dirty
    and flushed on a short delay rather than immediately, so dragging a control does not hit
    the disk on every value change.
*/
class TunerSettings
{
public:
    TunerSettings();

    /** Null when nothing has been saved yet, which is the signal to use system defaults. */
    std::unique_ptr<juce::XmlElement> getAudioDeviceState() const;
    void setAudioDeviceState (const juce::XmlElement* state);

    juce::String getTuningName() const;
    void setTuningName (const juce::String& name);

    double getReferencePitchHz() const;
    void setReferencePitchHz (double frequencyHz);

    /** Flushes anything still pending. Call before the app exits. */
    void flush();

private:
    juce::PropertiesFile& file() const;

    static constexpr double defaultReferencePitchHz = 440.0;

    mutable juce::ApplicationProperties properties;
};
