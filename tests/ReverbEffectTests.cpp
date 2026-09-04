#include "../src/Effects/ReverbEffect.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace voicemod
{

namespace
{

constexpr double testSampleRate = 48000.0;

juce::AudioBuffer<float> makeSineBuffer(float frequencyHz, int numSamples)
{
    juce::AudioBuffer<float> buffer(1, numSamples);
    auto* data = buffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i)
        data[i] = std::sin(2.0f * juce::MathConstants<float>::pi * frequencyHz * (float) i / (float) testSampleRate);
    return buffer;
}

juce::AudioBuffer<float> makeImpulseBuffer(int numSamples)
{
    juce::AudioBuffer<float> buffer(1, numSamples);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    return buffer;
}

float sumAbs(const juce::AudioBuffer<float>& buffer, int startSample)
{
    float total = 0.0f;
    for (int i = startSample; i < buffer.getNumSamples(); ++i)
        total += std::abs(buffer.getSample(0, i));
    return total;
}

}

class ReverbEffectTests : public juce::UnitTest
{
public:
    ReverbEffectTests() : juce::UnitTest("ReverbEffect", "Effects") {}

    void runTest() override
    {
        beginTest("wetDryMix=0 (fully dry) passes the signal through unchanged once settled");
        {
            ReverbEffect reverb;
            reverb.prepare(testSampleRate, 512);
            reverb.setWetDryMix(0.0f);

            // The internal dry/wet gains are smoothed over ~10ms after setWetDryMix, so skip that
            // settling window (worst case ~480 samples at 48kHz) before comparing.
            constexpr int numSamples = 2000;
            constexpr int settleSamples = 600;

            auto input = makeSineBuffer(300.0f, numSamples);
            auto buffer = input;
            reverb.process(buffer);

            for (int i = settleSamples; i < buffer.getNumSamples(); ++i)
                expectWithinAbsoluteError(buffer.getSample(0, i), input.getSample(0, i), 1.0e-3f);
        }

        beginTest("wetDryMix=1 (fully wet) leaves an audible reverb tail after an impulse");
        {
            ReverbEffect reverb;
            reverb.prepare(testSampleRate, 4096);
            reverb.setWetDryMix(1.0f);
            reverb.setRoomSize(0.8f);
            reverb.setDamping(0.5f);

            auto buffer = makeImpulseBuffer(4096);
            reverb.process(buffer);

            // Well after the impulse, a fully-wet reverb should still have non-negligible energy (the tail),
            // whereas fully-dry silence would be exactly zero here.
            expect(sumAbs(buffer, 1000) > 0.01f);
        }
    }
};

static ReverbEffectTests reverbEffectTests;

}
