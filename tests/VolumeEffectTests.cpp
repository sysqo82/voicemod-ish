#include "../src/Effects/VolumeEffect.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace voicemod
{

namespace
{

juce::AudioBuffer<float> makeConstantBuffer(float value, int numSamples = 8)
{
    juce::AudioBuffer<float> buffer(1, numSamples);
    buffer.clear();
    for (int i = 0; i < numSamples; ++i)
        buffer.setSample(0, i, value);
    return buffer;
}

}

class VolumeEffectTests : public juce::UnitTest
{
public:
    VolumeEffectTests() : juce::UnitTest("VolumeEffect", "Effects") {}

    void runTest() override
    {
        beginTest("muted produces silence regardless of gain");
        {
            VolumeEffect volume;
            volume.prepare(48000.0, 512);
            volume.setGainDecibels(6.0f);
            volume.setMuted(true);

            auto buffer = makeConstantBuffer(1.0f);
            volume.process(buffer);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                expectEquals(buffer.getSample(0, i), 0.0f);
        }

        beginTest("unity gain (0dB), unmuted, passes signal through unchanged");
        {
            VolumeEffect volume;
            volume.prepare(48000.0, 512);
            volume.setGainDecibels(0.0f);
            volume.setMuted(false);

            auto buffer = makeConstantBuffer(0.5f);
            volume.process(buffer);
            volume.process(buffer); // second block: past the gain smoothing ramp

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                expectWithinAbsoluteError(buffer.getSample(0, i), 0.5f, 1.0e-4f);
        }

        beginTest("-6dB scales the signal by the corresponding linear gain");
        {
            VolumeEffect volume;
            volume.prepare(48000.0, 512);
            volume.setGainDecibels(-6.0f);
            volume.setMuted(false);

            auto buffer = makeConstantBuffer(1.0f, 4096);
            volume.process(buffer);

            const auto expectedGain = juce::Decibels::decibelsToGain(-6.0f, -100.0f);
            expectWithinAbsoluteError(buffer.getSample(0, buffer.getNumSamples() - 1), expectedGain, 1.0e-3f);
        }
    }
};

static VolumeEffectTests volumeEffectTests;

}
