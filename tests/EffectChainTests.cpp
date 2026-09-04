#include "../src/Effects/EffectChain.h"
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

class EffectChainTests : public juce::UnitTest
{
public:
    EffectChainTests() : juce::UnitTest("EffectChain", "Effects") {}

    void runTest() override
    {
        beginTest("empty chain, not bypassed, leaves the buffer unchanged");
        {
            EffectChain chain;
            chain.prepare(48000.0, 512);

            auto buffer = makeConstantBuffer(0.25f);
            chain.processBlock(buffer);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                expectEquals(buffer.getSample(0, i), 0.25f);
        }

        beginTest("bypassed chain skips an active, muting effect");
        {
            EffectChain chain;
            auto volume = std::make_unique<VolumeEffect>();
            volume->setMuted(true);
            chain.addEffect(std::move(volume));
            chain.prepare(48000.0, 512);
            chain.bypassed = true;

            auto buffer = makeConstantBuffer(0.25f);
            chain.processBlock(buffer);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                expectEquals(buffer.getSample(0, i), 0.25f);
        }

        beginTest("enabled effect in the chain is applied");
        {
            EffectChain chain;
            auto volume = std::make_unique<VolumeEffect>();
            volume->setMuted(true);
            chain.addEffect(std::move(volume));
            chain.prepare(48000.0, 512);

            auto buffer = makeConstantBuffer(0.25f);
            chain.processBlock(buffer);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                expectEquals(buffer.getSample(0, i), 0.0f);
        }

        beginTest("disabled effect in the chain is skipped");
        {
            EffectChain chain;
            auto volume = std::make_unique<VolumeEffect>();
            volume->setMuted(true);
            volume->enabled = false;
            chain.addEffect(std::move(volume));
            chain.prepare(48000.0, 512);

            auto buffer = makeConstantBuffer(0.25f);
            chain.processBlock(buffer);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                expectEquals(buffer.getSample(0, i), 0.25f);
        }
    }
};

static EffectChainTests effectChainTests;

}
