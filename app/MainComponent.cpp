#include "MainComponent.h"

namespace
{

void configureSlider(juce::Slider& slider, double minValue, double maxValue, double defaultValue,
                      const juce::String& suffix)
{
    slider.setRange(minValue, maxValue);
    slider.setValue(defaultValue, juce::dontSendNotification);
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    slider.setTextValueSuffix(suffix);
}

}

void MainComponent::LevelMeter::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    auto bounds = getLocalBounds().toFloat();
    const auto width = bounds.getWidth() * juce::jlimit(0.0f, 1.0f, level);
    g.setColour(juce::Colours::limegreen);
    g.fillRect(bounds.removeFromLeft(width));
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds());
}

MainComponent::MainComponent()
    : deviceSelector(deviceManager, 1, 1, 2, 2, false, false, true, false)
{
    volumeEffect = effectChain.addEffect(std::make_unique<voicemod::VolumeEffect>());
    eqEffect = effectChain.addEffect(std::make_unique<voicemod::EQEffect>());
    pitchEffect = effectChain.addEffect(std::make_unique<voicemod::PitchEffect>());
    echoEffect = effectChain.addEffect(std::make_unique<voicemod::EchoEffect>());
    distortionEffect = effectChain.addEffect(std::make_unique<voicemod::DistortionEffect>());
    reverbEffect = effectChain.addEffect(std::make_unique<voicemod::ReverbEffect>());

    addAndMakeVisible(deviceSelector);

    addAndMakeVisible(bypassButton);
    bypassButton.onClick = [this]
    {
        effectChain.bypassed = bypassButton.getToggleState();
        saveSettings();
    };

    addAndMakeVisible(inputLevelLabel);
    addAndMakeVisible(inputLevelMeter);

    addAndMakeVisible(volumeLabel);
    configureSlider(volumeGainSlider, -60.0, 12.0, 0.0, " dB");
    volumeGainSlider.onValueChange = [this]
    {
        volumeEffect->setGainDecibels((float) volumeGainSlider.getValue());
        saveSettings();
    };
    addAndMakeVisible(volumeGainSlider);
    addAndMakeVisible(volumeMuteButton);
    volumeMuteButton.onClick = [this]
    {
        volumeEffect->setMuted(volumeMuteButton.getToggleState());
        saveSettings();
    };
    addAndMakeVisible(volumeResetButton);
    volumeResetButton.onClick = [this]
    {
        volumeGainSlider.setValue(0.0, juce::dontSendNotification);
        volumeMuteButton.setToggleState(false, juce::dontSendNotification);
        volumeEffect->setGainDecibels(0.0f);
        volumeEffect->setMuted(false);
        saveSettings();
    };

    addAndMakeVisible(eqLabel);
    configureSlider(eqLowSlider, -12.0, 12.0, 0.0, " dB");
    configureSlider(eqMidSlider, -12.0, 12.0, 0.0, " dB");
    configureSlider(eqHighSlider, -12.0, 12.0, 0.0, " dB");
    eqLowSlider.onValueChange = [this]
    {
        eqEffect->setLowShelfGainDecibels((float) eqLowSlider.getValue());
        saveSettings();
    };
    eqMidSlider.onValueChange = [this]
    {
        eqEffect->setMidPeakGainDecibels((float) eqMidSlider.getValue());
        saveSettings();
    };
    eqHighSlider.onValueChange = [this]
    {
        eqEffect->setHighShelfGainDecibels((float) eqHighSlider.getValue());
        saveSettings();
    };
    addAndMakeVisible(eqLowSlider);
    addAndMakeVisible(eqMidSlider);
    addAndMakeVisible(eqHighSlider);
    addAndMakeVisible(eqResetButton);
    eqResetButton.onClick = [this]
    {
        eqLowSlider.setValue(0.0, juce::dontSendNotification);
        eqMidSlider.setValue(0.0, juce::dontSendNotification);
        eqHighSlider.setValue(0.0, juce::dontSendNotification);
        eqEffect->setLowShelfGainDecibels(0.0f);
        eqEffect->setMidPeakGainDecibels(0.0f);
        eqEffect->setHighShelfGainDecibels(0.0f);
        saveSettings();
    };

    addAndMakeVisible(pitchLabel);
    configureSlider(pitchSlider, -12.0, 12.0, 0.0, " st");
    pitchSlider.onValueChange = [this]
    {
        pitchEffect->setPitchSemitones((float) pitchSlider.getValue());
        saveSettings();
    };
    addAndMakeVisible(pitchSlider);
    addAndMakeVisible(pitchResetButton);
    pitchResetButton.onClick = [this]
    {
        pitchSlider.setValue(0.0, juce::dontSendNotification);
        pitchEffect->setPitchSemitones(0.0f);
        saveSettings();
    };

    addAndMakeVisible(echoLabel);
    configureSlider(echoDelaySlider, 0.01, 1.0, 0.3, " s");
    configureSlider(echoFeedbackSlider, 0.0, 0.95, 0.3, "");
    configureSlider(echoMixSlider, 0.0, 1.0, 0.3, "");
    echoDelaySlider.onValueChange = [this]
    {
        echoEffect->setDelayTimeSeconds((float) echoDelaySlider.getValue());
        saveSettings();
    };
    echoFeedbackSlider.onValueChange = [this]
    {
        echoEffect->setFeedback((float) echoFeedbackSlider.getValue());
        saveSettings();
    };
    echoMixSlider.onValueChange = [this]
    {
        echoEffect->setMix((float) echoMixSlider.getValue());
        saveSettings();
    };
    addAndMakeVisible(echoDelaySlider);
    addAndMakeVisible(echoFeedbackSlider);
    addAndMakeVisible(echoMixSlider);
    addAndMakeVisible(echoResetButton);
    echoResetButton.onClick = [this]
    {
        echoDelaySlider.setValue(0.3, juce::dontSendNotification);
        echoFeedbackSlider.setValue(0.3, juce::dontSendNotification);
        echoMixSlider.setValue(0.3, juce::dontSendNotification);
        echoEffect->setDelayTimeSeconds(0.3f);
        echoEffect->setFeedback(0.3f);
        echoEffect->setMix(0.3f);
        saveSettings();
    };

    addAndMakeVisible(distortionLabel);
    configureSlider(distortionDriveSlider, 0.0, 1.0, 0.0, "");
    distortionDriveSlider.onValueChange = [this]
    {
        distortionEffect->setDrive((float) distortionDriveSlider.getValue());
        saveSettings();
    };
    addAndMakeVisible(distortionDriveSlider);
    addAndMakeVisible(distortionResetButton);
    distortionResetButton.onClick = [this]
    {
        distortionDriveSlider.setValue(0.0, juce::dontSendNotification);
        distortionEffect->setDrive(0.0f);
        saveSettings();
    };

    addAndMakeVisible(reverbLabel);
    configureSlider(reverbRoomSlider, 0.0, 1.0, 0.5, "");
    configureSlider(reverbDampingSlider, 0.0, 1.0, 0.5, "");
    configureSlider(reverbMixSlider, 0.0, 1.0, 0.0, "");
    reverbRoomSlider.onValueChange = [this]
    {
        reverbEffect->setRoomSize((float) reverbRoomSlider.getValue());
        saveSettings();
    };
    reverbDampingSlider.onValueChange = [this]
    {
        reverbEffect->setDamping((float) reverbDampingSlider.getValue());
        saveSettings();
    };
    reverbMixSlider.onValueChange = [this]
    {
        reverbEffect->setWetDryMix((float) reverbMixSlider.getValue());
        saveSettings();
    };
    addAndMakeVisible(reverbRoomSlider);
    addAndMakeVisible(reverbDampingSlider);
    addAndMakeVisible(reverbMixSlider);
    addAndMakeVisible(reverbResetButton);
    reverbResetButton.onClick = [this]
    {
        reverbRoomSlider.setValue(0.5, juce::dontSendNotification);
        reverbDampingSlider.setValue(0.5, juce::dontSendNotification);
        reverbMixSlider.setValue(0.0, juce::dontSendNotification);
        reverbEffect->setRoomSize(0.5f);
        reverbEffect->setDamping(0.5f);
        reverbEffect->setWetDryMix(0.0f);
        saveSettings();
    };

    // Apply the slider defaults set above to the effects, then let loadSettings() override with any saved state.
    volumeEffect->setGainDecibels((float) volumeGainSlider.getValue());
    volumeEffect->setMuted(volumeMuteButton.getToggleState());
    eqEffect->setLowShelfGainDecibels((float) eqLowSlider.getValue());
    eqEffect->setMidPeakGainDecibels((float) eqMidSlider.getValue());
    eqEffect->setHighShelfGainDecibels((float) eqHighSlider.getValue());
    pitchEffect->setPitchSemitones((float) pitchSlider.getValue());
    echoEffect->setDelayTimeSeconds((float) echoDelaySlider.getValue());
    echoEffect->setFeedback((float) echoFeedbackSlider.getValue());
    echoEffect->setMix((float) echoMixSlider.getValue());
    distortionEffect->setDrive((float) distortionDriveSlider.getValue());
    reverbEffect->setRoomSize((float) reverbRoomSlider.getValue());
    reverbEffect->setDamping((float) reverbDampingSlider.getValue());
    reverbEffect->setWetDryMix((float) reverbMixSlider.getValue());

    loadSettings();
    deviceManager.addAudioCallback(this);

    setSize(700, 760);
    startTimerHz(20);
}

MainComponent::~MainComponent()
{
    deviceManager.removeAudioCallback(this);
    saveSettings();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    deviceSelector.setBounds(area.removeFromTop(150));
    area.removeFromTop(10);

    auto topRow = area.removeFromTop(24);
    bypassButton.setBounds(topRow.removeFromLeft(100));
    inputLevelLabel.setBounds(topRow.removeFromLeft(80));
    inputLevelMeter.setBounds(topRow);
    area.removeFromTop(10);

    auto layoutRow = [&area](std::initializer_list<juce::Component*> components)
    {
        auto row = area.removeFromTop(28);
        const auto widthEach = row.getWidth() / (int) components.size();
        for (auto* component : components)
            component->setBounds(row.removeFromLeft(widthEach).reduced(2));
        area.removeFromTop(6);
    };

    layoutRow({ &volumeLabel, &volumeGainSlider, &volumeMuteButton, &volumeResetButton });
    layoutRow({ &eqLabel, &eqLowSlider, &eqMidSlider, &eqHighSlider, &eqResetButton });
    layoutRow({ &pitchLabel, &pitchSlider, &pitchResetButton });
    layoutRow({ &echoLabel, &echoDelaySlider, &echoFeedbackSlider, &echoMixSlider, &echoResetButton });
    layoutRow({ &distortionLabel, &distortionDriveSlider, &distortionResetButton });
    layoutRow({ &reverbLabel, &reverbRoomSlider, &reverbDampingSlider, &reverbMixSlider, &reverbResetButton });
}

void MainComponent::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                                      float* const* outputChannelData, int numOutputChannels,
                                                      int numSamples,
                                                      const juce::AudioIODeviceCallbackContext&)
{
    if (numOutputChannels == 0)
        return;

    monoScratchBuffer.setSize(1, numSamples, false, false, true);
    auto* mono = monoScratchBuffer.getWritePointer(0);

    if (numInputChannels > 0 && inputChannelData[0] != nullptr)
    {
        for (int i = 0; i < numSamples; ++i)
            mono[i] = inputChannelData[0][i];
    }
    else
    {
        monoScratchBuffer.clear();
    }

    effectChain.processBlock(monoScratchBuffer);

    currentInputLevel.store(monoScratchBuffer.getMagnitude(0, numSamples));

    for (int channel = 0; channel < numOutputChannels; ++channel)
        if (outputChannelData[channel] != nullptr)
            juce::FloatVectorOperations::copy(outputChannelData[channel], mono, numSamples);
}

void MainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    effectChain.prepare(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
}

void MainComponent::audioDeviceStopped()
{
}

void MainComponent::timerCallback()
{
    inputLevelMeter.setLevel(currentInputLevel.load());
}

juce::PropertiesFile::Options MainComponent::makePropertiesOptions()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Voicemod-ish";
    options.filenameSuffix = ".settings";
    options.folderName = "Voicemod-ish";
    options.osxLibrarySubFolder = "Application Support";
    return options;
}

void MainComponent::loadSettings()
{
    propertiesFile = std::make_unique<juce::PropertiesFile>(makePropertiesOptions());

    volumeGainSlider.setValue(propertiesFile->getDoubleValue("volumeGainDb", 0.0), juce::dontSendNotification);
    volumeMuteButton.setToggleState(propertiesFile->getBoolValue("volumeMuted", false), juce::dontSendNotification);
    eqLowSlider.setValue(propertiesFile->getDoubleValue("eqLowDb", 0.0), juce::dontSendNotification);
    eqMidSlider.setValue(propertiesFile->getDoubleValue("eqMidDb", 0.0), juce::dontSendNotification);
    eqHighSlider.setValue(propertiesFile->getDoubleValue("eqHighDb", 0.0), juce::dontSendNotification);
    pitchSlider.setValue(propertiesFile->getDoubleValue("pitchSemitones", 0.0), juce::dontSendNotification);
    echoDelaySlider.setValue(propertiesFile->getDoubleValue("echoDelaySeconds", 0.3), juce::dontSendNotification);
    echoFeedbackSlider.setValue(propertiesFile->getDoubleValue("echoFeedback", 0.3), juce::dontSendNotification);
    echoMixSlider.setValue(propertiesFile->getDoubleValue("echoMix", 0.3), juce::dontSendNotification);
    distortionDriveSlider.setValue(propertiesFile->getDoubleValue("distortionDrive", 0.0), juce::dontSendNotification);
    reverbRoomSlider.setValue(propertiesFile->getDoubleValue("reverbRoomSize", 0.5), juce::dontSendNotification);
    reverbDampingSlider.setValue(propertiesFile->getDoubleValue("reverbDamping", 0.5), juce::dontSendNotification);
    reverbMixSlider.setValue(propertiesFile->getDoubleValue("reverbMix", 0.0), juce::dontSendNotification);
    bypassButton.setToggleState(propertiesFile->getBoolValue("bypass", false), juce::dontSendNotification);

    volumeEffect->setGainDecibels((float) volumeGainSlider.getValue());
    volumeEffect->setMuted(volumeMuteButton.getToggleState());
    eqEffect->setLowShelfGainDecibels((float) eqLowSlider.getValue());
    eqEffect->setMidPeakGainDecibels((float) eqMidSlider.getValue());
    eqEffect->setHighShelfGainDecibels((float) eqHighSlider.getValue());
    pitchEffect->setPitchSemitones((float) pitchSlider.getValue());
    echoEffect->setDelayTimeSeconds((float) echoDelaySlider.getValue());
    echoEffect->setFeedback((float) echoFeedbackSlider.getValue());
    echoEffect->setMix((float) echoMixSlider.getValue());
    distortionEffect->setDrive((float) distortionDriveSlider.getValue());
    reverbEffect->setRoomSize((float) reverbRoomSlider.getValue());
    reverbEffect->setDamping((float) reverbDampingSlider.getValue());
    reverbEffect->setWetDryMix((float) reverbMixSlider.getValue());
    effectChain.bypassed = bypassButton.getToggleState();

    if (auto savedDeviceState = std::unique_ptr<juce::XmlElement>(propertiesFile->getXmlValue("audioDeviceState")))
        deviceManager.initialise(1, 2, savedDeviceState.get(), true);
    else
        deviceManager.initialise(1, 2, nullptr, true);
}

void MainComponent::saveSettings()
{
    if (propertiesFile == nullptr)
        return;

    propertiesFile->setValue("volumeGainDb", volumeGainSlider.getValue());
    propertiesFile->setValue("volumeMuted", volumeMuteButton.getToggleState());
    propertiesFile->setValue("eqLowDb", eqLowSlider.getValue());
    propertiesFile->setValue("eqMidDb", eqMidSlider.getValue());
    propertiesFile->setValue("eqHighDb", eqHighSlider.getValue());
    propertiesFile->setValue("pitchSemitones", pitchSlider.getValue());
    propertiesFile->setValue("echoDelaySeconds", echoDelaySlider.getValue());
    propertiesFile->setValue("echoFeedback", echoFeedbackSlider.getValue());
    propertiesFile->setValue("echoMix", echoMixSlider.getValue());
    propertiesFile->setValue("distortionDrive", distortionDriveSlider.getValue());
    propertiesFile->setValue("reverbRoomSize", reverbRoomSlider.getValue());
    propertiesFile->setValue("reverbDamping", reverbDampingSlider.getValue());
    propertiesFile->setValue("reverbMix", reverbMixSlider.getValue());
    propertiesFile->setValue("bypass", bypassButton.getToggleState());

    if (auto deviceState = std::unique_ptr<juce::XmlElement>(deviceManager.createStateXml()))
        propertiesFile->setValue("audioDeviceState", deviceState.get());

    propertiesFile->saveIfNeeded();
}
