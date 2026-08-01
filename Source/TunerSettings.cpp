#include "TunerSettings.h"

namespace
{
    const juce::String audioDeviceStateKey { "audioDeviceState" };
    const juce::String tuningNameKey       { "tuningName" };
    const juce::String referencePitchKey   { "referencePitchHz" };
}

TunerSettings::TunerSettings()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Guitar Tuner";
    options.filenameSuffix = ".settings";
    options.folderName = "GuitarTuner";
    options.osxLibrarySubFolder = "Application Support";
    options.millisecondsBeforeSaving = 2000;

    properties.setStorageParameters (options);
}

juce::PropertiesFile& TunerSettings::file() const
{
    return *properties.getUserSettings();
}

std::unique_ptr<juce::XmlElement> TunerSettings::getAudioDeviceState() const
{
    return file().getXmlValue (audioDeviceStateKey);
}

void TunerSettings::setAudioDeviceState (const juce::XmlElement* state)
{
    if (state != nullptr)
        file().setValue (audioDeviceStateKey, state);
    else
        file().removeValue (audioDeviceStateKey);
}

juce::String TunerSettings::getTuningName() const
{
    return file().getValue (tuningNameKey);
}

void TunerSettings::setTuningName (const juce::String& name)
{
    file().setValue (tuningNameKey, name);
}

double TunerSettings::getReferencePitchHz() const
{
    return file().getDoubleValue (referencePitchKey, defaultReferencePitchHz);
}

void TunerSettings::setReferencePitchHz (double frequencyHz)
{
    file().setValue (referencePitchKey, frequencyHz);
}

void TunerSettings::flush()
{
    properties.saveIfNeeded();
}
