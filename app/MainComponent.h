#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../src/Effects/EffectChain.h"
#include "../src/Effects/VolumeEffect.h"
#include "../src/Effects/EQEffect.h"
#include "../src/Effects/PitchEffect.h"
#include "../src/Effects/EchoEffect.h"
#include "../src/Effects/DistortionEffect.h"
#include "../src/Effects/ReverbEffect.h"
#include <atomic>

// Single window, all effect sections visible at once, per SPEC.md.
class MainComponent : public juce::Component,
                      private juce::AudioIODeviceCallback,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                           float* const* outputChannelData, int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    void timerCallback() override;

    void loadSettings();
    void saveSettings();
    static juce::PropertiesFile::Options makePropertiesOptions();

    // A simple horizontal input level bar (see SPEC.md "input level meter").
    class LevelMeter : public juce::Component
    {
    public:
        void setLevel(float newLevel) { level = newLevel; repaint(); }
        void paint(juce::Graphics& g) override;

    private:
        float level = 0.0f;
    };

    juce::AudioDeviceManager deviceManager;
    juce::AudioDeviceSelectorComponent deviceSelector;

    voicemod::EffectChain effectChain;
    voicemod::VolumeEffect* volumeEffect = nullptr;
    voicemod::EQEffect* eqEffect = nullptr;
    voicemod::PitchEffect* pitchEffect = nullptr;
    voicemod::EchoEffect* echoEffect = nullptr;
    voicemod::DistortionEffect* distortionEffect = nullptr;
    voicemod::ReverbEffect* reverbEffect = nullptr;

    std::atomic<float> currentInputLevel { 0.0f };
    juce::AudioBuffer<float> monoScratchBuffer;

    std::unique_ptr<juce::PropertiesFile> propertiesFile;

    juce::ToggleButton bypassButton { "Bypass" };
    juce::Label inputLevelLabel { {}, "Input level" };
    LevelMeter inputLevelMeter;

    juce::Label volumeLabel { {}, "Volume" };
    juce::Slider volumeGainSlider;
    juce::ToggleButton volumeMuteButton { "Mute" };
    juce::TextButton volumeResetButton { "Reset" };

    juce::Label eqLabel { {}, "EQ" };
    juce::Slider eqLowSlider, eqMidSlider, eqHighSlider;
    juce::TextButton eqResetButton { "Reset" };

    juce::Label pitchLabel { {}, "Pitch" };
    juce::Slider pitchSlider;
    juce::TextButton pitchResetButton { "Reset" };

    juce::Label echoLabel { {}, "Echo" };
    juce::Slider echoDelaySlider, echoFeedbackSlider, echoMixSlider;
    juce::TextButton echoResetButton { "Reset" };

    juce::Label distortionLabel { {}, "Distortion" };
    juce::Slider distortionDriveSlider;
    juce::TextButton distortionResetButton { "Reset" };

    juce::Label reverbLabel { {}, "Reverb" };
    juce::Slider reverbRoomSlider, reverbDampingSlider, reverbMixSlider;
    juce::TextButton reverbResetButton { "Reset" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
