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

void MainComponent::VirtualMicOutput::pushBlock(const float* data, int numSamples)
{
    int start1, size1, start2, size2;
    fifo.prepareToWrite(numSamples, start1, size1, start2, size2);
    if (size1 > 0) ringBuffer.copyFrom(0, start1, data, size1);
    if (size2 > 0) ringBuffer.copyFrom(0, start2, data + size1, size2);
    fifo.finishedWrite(size1 + size2);
}

void MainComponent::VirtualMicOutput::audioDeviceIOCallbackWithContext(const float* const*, int,
                                                                       float* const* outputChannelData,
                                                                       int numOutputChannels, int numSamples,
                                                                       const juce::AudioIODeviceCallbackContext&)
{
    const auto available = juce::jmin(numSamples, fifo.getNumReady());

    int start1, size1, start2, size2;
    fifo.prepareToRead(available, start1, size1, start2, size2);

    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (outputChannelData[channel] == nullptr)
            continue;

        int written = 0;
        if (size1 > 0) { juce::FloatVectorOperations::copy(outputChannelData[channel], ringBuffer.getReadPointer(0, start1), size1); written += size1; }
        if (size2 > 0) { juce::FloatVectorOperations::copy(outputChannelData[channel] + written, ringBuffer.getReadPointer(0, start2), size2); written += size2; }
        if (written < numSamples)
            juce::FloatVectorOperations::clear(outputChannelData[channel] + written, numSamples - written);
    }

    fifo.finishedRead(size1 + size2);
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

    addAndMakeVisible(virtualMicOutputLabel);
    addAndMakeVisible(virtualMicOutputBox);
    virtualMicOutputBox.onChange = [this]
    {
        const auto id = virtualMicOutputBox.getSelectedId();
        setVirtualMicOutputDevice(id <= 1 ? juce::String() : virtualMicOutputBox.getText());
        saveSettings();
    };

    addAndMakeVisible(effectsViewport);
    effectsViewport.setViewedComponent(&effectsViewportContent, false);

    effectsViewportContent.addAndMakeVisible(volumeBox);
    effectsViewportContent.addAndMakeVisible(volumeEnabledButton);
    volumeEnabledButton.setToggleState(true, juce::dontSendNotification);
    volumeEnabledButton.onClick = [this]
    {
        volumeEffect->enabled = volumeEnabledButton.getToggleState();
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(volumeGainNameLabel);
    configureSlider(volumeGainSlider, -60.0, 12.0, 0.0, " dB");
    volumeGainSlider.onValueChange = [this]
    {
        volumeEffect->setGainDecibels((float) volumeGainSlider.getValue());
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(volumeGainSlider);
    effectsViewportContent.addAndMakeVisible(volumeMuteButton);
    volumeMuteButton.onClick = [this]
    {
        volumeEffect->setMuted(volumeMuteButton.getToggleState());
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(volumeResetButton);
    volumeResetButton.onClick = [this]
    {
        volumeGainSlider.setValue(0.0, juce::dontSendNotification);
        volumeMuteButton.setToggleState(false, juce::dontSendNotification);
        volumeEnabledButton.setToggleState(true, juce::dontSendNotification);
        volumeEffect->setGainDecibels(0.0f);
        volumeEffect->setMuted(false);
        volumeEffect->enabled = true;
        saveSettings();
    };

    effectsViewportContent.addAndMakeVisible(eqBox);
    effectsViewportContent.addAndMakeVisible(eqEnabledButton);
    eqEnabledButton.setToggleState(true, juce::dontSendNotification);
    eqEnabledButton.onClick = [this]
    {
        eqEffect->enabled = eqEnabledButton.getToggleState();
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(eqLowNameLabel);
    effectsViewportContent.addAndMakeVisible(eqMidNameLabel);
    effectsViewportContent.addAndMakeVisible(eqHighNameLabel);
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
    effectsViewportContent.addAndMakeVisible(eqLowSlider);
    effectsViewportContent.addAndMakeVisible(eqMidSlider);
    effectsViewportContent.addAndMakeVisible(eqHighSlider);
    effectsViewportContent.addAndMakeVisible(eqResetButton);
    eqResetButton.onClick = [this]
    {
        eqLowSlider.setValue(0.0, juce::dontSendNotification);
        eqMidSlider.setValue(0.0, juce::dontSendNotification);
        eqHighSlider.setValue(0.0, juce::dontSendNotification);
        eqEnabledButton.setToggleState(true, juce::dontSendNotification);
        eqEffect->setLowShelfGainDecibels(0.0f);
        eqEffect->setMidPeakGainDecibels(0.0f);
        eqEffect->setHighShelfGainDecibels(0.0f);
        eqEffect->enabled = true;
        saveSettings();
    };

    effectsViewportContent.addAndMakeVisible(pitchBox);
    effectsViewportContent.addAndMakeVisible(pitchEnabledButton);
    pitchEnabledButton.setToggleState(true, juce::dontSendNotification);
    pitchEnabledButton.onClick = [this]
    {
        pitchEffect->enabled = pitchEnabledButton.getToggleState();
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(pitchNameLabel);
    configureSlider(pitchSlider, -12.0, 12.0, 0.0, " st");
    pitchSlider.onValueChange = [this]
    {
        pitchEffect->setPitchSemitones((float) pitchSlider.getValue());
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(pitchSlider);
    effectsViewportContent.addAndMakeVisible(pitchResetButton);
    pitchResetButton.onClick = [this]
    {
        pitchSlider.setValue(0.0, juce::dontSendNotification);
        pitchEnabledButton.setToggleState(true, juce::dontSendNotification);
        pitchEffect->setPitchSemitones(0.0f);
        pitchEffect->enabled = true;
        saveSettings();
    };

    effectsViewportContent.addAndMakeVisible(echoBox);
    effectsViewportContent.addAndMakeVisible(echoEnabledButton);
    echoEnabledButton.setToggleState(true, juce::dontSendNotification);
    echoEnabledButton.onClick = [this]
    {
        echoEffect->enabled = echoEnabledButton.getToggleState();
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(echoDelayNameLabel);
    effectsViewportContent.addAndMakeVisible(echoFeedbackNameLabel);
    effectsViewportContent.addAndMakeVisible(echoMixNameLabel);
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
    effectsViewportContent.addAndMakeVisible(echoDelaySlider);
    effectsViewportContent.addAndMakeVisible(echoFeedbackSlider);
    effectsViewportContent.addAndMakeVisible(echoMixSlider);
    effectsViewportContent.addAndMakeVisible(echoResetButton);
    echoResetButton.onClick = [this]
    {
        echoDelaySlider.setValue(0.3, juce::dontSendNotification);
        echoFeedbackSlider.setValue(0.3, juce::dontSendNotification);
        echoMixSlider.setValue(0.3, juce::dontSendNotification);
        echoEnabledButton.setToggleState(true, juce::dontSendNotification);
        echoEffect->setDelayTimeSeconds(0.3f);
        echoEffect->setFeedback(0.3f);
        echoEffect->setMix(0.3f);
        echoEffect->enabled = true;
        saveSettings();
    };

    effectsViewportContent.addAndMakeVisible(distortionBox);
    effectsViewportContent.addAndMakeVisible(distortionEnabledButton);
    distortionEnabledButton.setToggleState(true, juce::dontSendNotification);
    distortionEnabledButton.onClick = [this]
    {
        distortionEffect->enabled = distortionEnabledButton.getToggleState();
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(distortionDriveNameLabel);
    configureSlider(distortionDriveSlider, 0.0, 1.0, 0.0, "");
    distortionDriveSlider.onValueChange = [this]
    {
        distortionEffect->setDrive((float) distortionDriveSlider.getValue());
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(distortionDriveSlider);
    effectsViewportContent.addAndMakeVisible(distortionResetButton);
    distortionResetButton.onClick = [this]
    {
        distortionDriveSlider.setValue(0.0, juce::dontSendNotification);
        distortionEnabledButton.setToggleState(true, juce::dontSendNotification);
        distortionEffect->setDrive(0.0f);
        distortionEffect->enabled = true;
        saveSettings();
    };

    effectsViewportContent.addAndMakeVisible(reverbBox);
    effectsViewportContent.addAndMakeVisible(reverbEnabledButton);
    reverbEnabledButton.setToggleState(true, juce::dontSendNotification);
    reverbEnabledButton.onClick = [this]
    {
        reverbEffect->enabled = reverbEnabledButton.getToggleState();
        saveSettings();
    };
    effectsViewportContent.addAndMakeVisible(reverbRoomNameLabel);
    effectsViewportContent.addAndMakeVisible(reverbDampingNameLabel);
    effectsViewportContent.addAndMakeVisible(reverbMixNameLabel);
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
    effectsViewportContent.addAndMakeVisible(reverbRoomSlider);
    effectsViewportContent.addAndMakeVisible(reverbDampingSlider);
    effectsViewportContent.addAndMakeVisible(reverbMixSlider);
    effectsViewportContent.addAndMakeVisible(reverbResetButton);
    reverbResetButton.onClick = [this]
    {
        reverbRoomSlider.setValue(0.5, juce::dontSendNotification);
        reverbDampingSlider.setValue(0.5, juce::dontSendNotification);
        reverbMixSlider.setValue(0.0, juce::dontSendNotification);
        reverbEnabledButton.setToggleState(true, juce::dontSendNotification);
        reverbEffect->setRoomSize(0.5f);
        reverbEffect->setDamping(0.5f);
        reverbEffect->setWetDryMix(0.0f);
        reverbEffect->enabled = true;
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

    setSize(820, 800);
    startTimerHz(20);
}

MainComponent::~MainComponent()
{
    deviceManager.removeAudioCallback(this);
    if (virtualMicDevice != nullptr)
        virtualMicDevice->stop();
    saveSettings();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    // AudioDeviceSelectorComponent renders channel checkboxes, sample rate, and buffer size below
    // the device dropdowns, so it needs much more than a token allowance or it overlaps the rows below it.
    deviceSelector.setBounds(area.removeFromTop(330));
    area.removeFromTop(10);

    auto topRow = area.removeFromTop(24);
    bypassButton.setBounds(topRow.removeFromLeft(100));
    inputLevelLabel.setBounds(topRow.removeFromLeft(80));
    inputLevelMeter.setBounds(topRow);
    area.removeFromTop(10);

    auto virtualMicRow = area.removeFromTop(24);
    virtualMicOutputLabel.setBounds(virtualMicRow.removeFromLeft(120));
    virtualMicOutputBox.setBounds(virtualMicRow.removeFromLeft(250));
    area.removeFromTop(10);

    effectsViewport.setBounds(area);

    const auto contentWidth = area.getWidth() - effectsViewport.getScrollBarThickness();
    int y = 8;
    y = layoutEffectBox(volumeBox, volumeEnabledButton, volumeResetButton, contentWidth, y,
                         { { &volumeGainNameLabel, &volumeGainSlider, &volumeMuteButton } });
    y = layoutEffectBox(eqBox, eqEnabledButton, eqResetButton, contentWidth, y,
                         { { &eqLowNameLabel, &eqLowSlider }, { &eqMidNameLabel, &eqMidSlider }, { &eqHighNameLabel, &eqHighSlider } });
    y = layoutEffectBox(pitchBox, pitchEnabledButton, pitchResetButton, contentWidth, y,
                         { { &pitchNameLabel, &pitchSlider } });
    y = layoutEffectBox(echoBox, echoEnabledButton, echoResetButton, contentWidth, y,
                         { { &echoDelayNameLabel, &echoDelaySlider }, { &echoFeedbackNameLabel, &echoFeedbackSlider },
                           { &echoMixNameLabel, &echoMixSlider } });
    y = layoutEffectBox(distortionBox, distortionEnabledButton, distortionResetButton, contentWidth, y,
                         { { &distortionDriveNameLabel, &distortionDriveSlider } });
    y = layoutEffectBox(reverbBox, reverbEnabledButton, reverbResetButton, contentWidth, y,
                         { { &reverbRoomNameLabel, &reverbRoomSlider }, { &reverbDampingNameLabel, &reverbDampingSlider },
                           { &reverbMixNameLabel, &reverbMixSlider } });

    effectsViewportContent.setSize(contentWidth, y);
}

int MainComponent::layoutEffectBox(juce::GroupComponent& box, juce::ToggleButton& enabledButton,
                                    juce::TextButton& resetButton, int contentWidth, int contentY,
                                    std::initializer_list<ParamRow> rows)
{
    constexpr int titleBarHeight = 28;
    constexpr int rowHeight = 32;
    constexpr int bottomPadding = 12;
    constexpr int sidePadding = 12;
    constexpr int nameLabelWidth = 80;
    constexpr int extraControlWidth = 70; // e.g. Volume's Mute toggle

    const auto height = titleBarHeight + (int) rows.size() * rowHeight + bottomPadding;
    juce::Rectangle<int> boxArea(0, contentY, contentWidth, height);
    box.setBounds(boxArea);

    // On/Reset sit in the box's title bar, to the right of the title text.
    auto headerRow = boxArea.removeFromTop(titleBarHeight).reduced(sidePadding, 4);
    resetButton.setBounds(headerRow.removeFromRight(70));
    headerRow.removeFromRight(8);
    enabledButton.setBounds(headerRow.removeFromRight(60));

    auto inner = boxArea.reduced(sidePadding, 0);
    inner.removeFromBottom(bottomPadding);
    for (auto& row : rows)
    {
        auto r = inner.removeFromTop(rowHeight).reduced(0, 3);
        row.nameLabel->setBounds(r.removeFromLeft(nameLabelWidth));
        if (row.extra != nullptr)
            row.extra->setBounds(r.removeFromRight(extraControlWidth));
        r.removeFromLeft(8);
        row.slider->setBounds(r);
    }

    return contentY + height + 10; // gap before the next box
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

    if (virtualMicDevice != nullptr)
        virtualMicCallback.pushBlock(mono, numSamples);

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

    volumeEnabledButton.setToggleState(propertiesFile->getBoolValue("volumeEnabled", true), juce::dontSendNotification);
    eqEnabledButton.setToggleState(propertiesFile->getBoolValue("eqEnabled", true), juce::dontSendNotification);
    pitchEnabledButton.setToggleState(propertiesFile->getBoolValue("pitchEnabled", true), juce::dontSendNotification);
    echoEnabledButton.setToggleState(propertiesFile->getBoolValue("echoEnabled", true), juce::dontSendNotification);
    distortionEnabledButton.setToggleState(propertiesFile->getBoolValue("distortionEnabled", true), juce::dontSendNotification);
    reverbEnabledButton.setToggleState(propertiesFile->getBoolValue("reverbEnabled", true), juce::dontSendNotification);

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

    volumeEffect->enabled = volumeEnabledButton.getToggleState();
    eqEffect->enabled = eqEnabledButton.getToggleState();
    pitchEffect->enabled = pitchEnabledButton.getToggleState();
    echoEffect->enabled = echoEnabledButton.getToggleState();
    distortionEffect->enabled = distortionEnabledButton.getToggleState();
    reverbEffect->enabled = reverbEnabledButton.getToggleState();

    if (auto savedDeviceState = std::unique_ptr<juce::XmlElement>(propertiesFile->getXmlValue("audioDeviceState")))
        deviceManager.initialise(1, 2, savedDeviceState.get(), true);
    else
        deviceManager.initialise(1, 2, nullptr, true);

    refreshVirtualMicOutputChoices();
    const auto savedVirtualMicDevice = propertiesFile->getValue("virtualMicOutputDevice");
    if (savedVirtualMicDevice.isNotEmpty())
    {
        virtualMicOutputBox.setText(savedVirtualMicDevice, juce::dontSendNotification);
        setVirtualMicOutputDevice(savedVirtualMicDevice);
    }
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

    propertiesFile->setValue("volumeEnabled", volumeEnabledButton.getToggleState());
    propertiesFile->setValue("eqEnabled", eqEnabledButton.getToggleState());
    propertiesFile->setValue("pitchEnabled", pitchEnabledButton.getToggleState());
    propertiesFile->setValue("echoEnabled", echoEnabledButton.getToggleState());
    propertiesFile->setValue("distortionEnabled", distortionEnabledButton.getToggleState());
    propertiesFile->setValue("reverbEnabled", reverbEnabledButton.getToggleState());

    propertiesFile->setValue("virtualMicOutputDevice",
                              virtualMicOutputBox.getSelectedId() <= 1 ? juce::String() : virtualMicOutputBox.getText());

    if (auto deviceState = std::unique_ptr<juce::XmlElement>(deviceManager.createStateXml()))
        propertiesFile->setValue("audioDeviceState", deviceState.get());

    propertiesFile->saveIfNeeded();
}

void MainComponent::refreshVirtualMicOutputChoices()
{
    virtualMicOutputBox.clear(juce::dontSendNotification);
    virtualMicOutputBox.addItem("None", 1);

    if (auto* type = deviceManager.getCurrentDeviceTypeObject())
    {
        type->scanForDevices();
        int itemId = 2;
        for (auto& name : type->getDeviceNames(false)) // false = output devices
            virtualMicOutputBox.addItem(name, itemId++);
    }

    virtualMicOutputBox.setSelectedId(1, juce::dontSendNotification);
}

void MainComponent::setVirtualMicOutputDevice(const juce::String& deviceName)
{
    if (virtualMicDevice != nullptr)
    {
        virtualMicDevice->stop();
        virtualMicDevice.reset();
    }

    if (deviceName.isEmpty())
        return;

    auto* type = deviceManager.getCurrentDeviceTypeObject();
    if (type == nullptr)
        return;

    virtualMicDevice.reset(type->createDevice({}, deviceName));
    if (virtualMicDevice == nullptr)
        return;

    // Match the primary device's sample rate where the virtual cable supports it, to avoid pitch drift.
    auto sampleRate = deviceManager.getCurrentAudioDevice() != nullptr
                           ? deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()
                           : 48000.0;
    auto availableRates = virtualMicDevice->getAvailableSampleRates();
    if (! availableRates.contains(sampleRate) && ! availableRates.isEmpty())
        sampleRate = availableRates[0];

    juce::BigInteger outputChannels;
    outputChannels.setRange(0, 2, true);

    const auto error = virtualMicDevice->open({}, outputChannels, sampleRate, virtualMicDevice->getDefaultBufferSize());
    if (error.isNotEmpty())
    {
        virtualMicDevice.reset();
        return;
    }

    virtualMicDevice->start(&virtualMicCallback);
}
