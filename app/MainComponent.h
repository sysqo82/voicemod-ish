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

    void refreshVirtualMicOutputChoices();
    void setVirtualMicOutputDevice(const juce::String& deviceName); // empty = disabled ("None")

    // Virtual Microphone output (v2, see ADR 0004): a second, independent, output-only AudioIODevice
    // fed a copy of the processed mono buffer, so another app can use the paired virtual-cable
    // recording endpoint as its microphone while the user still monitors on the real output device.
    class VirtualMicOutput : public juce::AudioIODeviceCallback
    {
    public:
        VirtualMicOutput() : ringBuffer(1, ringBufferSize) {}

        void pushBlock(const float* data, int numSamples); // called from the primary audio thread

        void audioDeviceIOCallbackWithContext(const float* const*, int, float* const* outputChannelData,
                                              int numOutputChannels, int numSamples,
                                              const juce::AudioIODeviceCallbackContext&) override;
        void audioDeviceAboutToStart(juce::AudioIODevice*) override {}
        void audioDeviceStopped() override {}

    private:
        static constexpr int ringBufferSize = 1 << 16;
        juce::AbstractFifo fifo { ringBufferSize };
        juce::AudioBuffer<float> ringBuffer;
    };

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

    juce::Label virtualMicOutputLabel { {}, "Virtual Mic Output" };
    juce::ComboBox virtualMicOutputBox;
    std::unique_ptr<juce::AudioIODevice> virtualMicDevice;
    VirtualMicOutput virtualMicCallback;

    juce::Label volumeLabel { {}, "Volume" };
    juce::ToggleButton volumeEnabledButton { "On" };
    juce::Label volumeGainNameLabel { {}, "Gain" };
    juce::Slider volumeGainSlider;
    juce::ToggleButton volumeMuteButton { "Mute" };
    juce::TextButton volumeResetButton { "Reset" };

    juce::Label eqLabel { {}, "EQ" };
    juce::ToggleButton eqEnabledButton { "On" };
    juce::Label eqLowNameLabel { {}, "Low" }, eqMidNameLabel { {}, "Mid" }, eqHighNameLabel { {}, "High" };
    juce::Slider eqLowSlider, eqMidSlider, eqHighSlider;
    juce::TextButton eqResetButton { "Reset" };

    juce::Label pitchLabel { {}, "Pitch" };
    juce::ToggleButton pitchEnabledButton { "On" };
    juce::Label pitchNameLabel { {}, "Semitones" };
    juce::Slider pitchSlider;
    juce::TextButton pitchResetButton { "Reset" };

    juce::Label echoLabel { {}, "Echo" };
    juce::ToggleButton echoEnabledButton { "On" };
    juce::Label echoDelayNameLabel { {}, "Delay" }, echoFeedbackNameLabel { {}, "Feedback" }, echoMixNameLabel { {}, "Mix" };
    juce::Slider echoDelaySlider, echoFeedbackSlider, echoMixSlider;
    juce::TextButton echoResetButton { "Reset" };

    juce::Label distortionLabel { {}, "Distortion" };
    juce::ToggleButton distortionEnabledButton { "On" };
    juce::Label distortionDriveNameLabel { {}, "Drive" };
    juce::Slider distortionDriveSlider;
    juce::TextButton distortionResetButton { "Reset" };

    juce::Label reverbLabel { {}, "Reverb" };
    juce::ToggleButton reverbEnabledButton { "On" };
    juce::Label reverbRoomNameLabel { {}, "Room" }, reverbDampingNameLabel { {}, "Damping" }, reverbMixNameLabel { {}, "Mix" };
    juce::Slider reverbRoomSlider, reverbDampingSlider, reverbMixSlider;
    juce::TextButton reverbResetButton { "Reset" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
