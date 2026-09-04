#include "../src/Effects/EQEffect.h"
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

// RMS over the back half of the buffer, to skip the filter's settling transient.
float settledRms(const juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();
    const auto start = numSamples / 2;
    return buffer.getRMSLevel(0, start, numSamples - start);
}

}

class EQEffectTests : public juce::UnitTest
{
public:
    EQEffectTests() : juce::UnitTest("EQEffect", "Effects") {}

    void runTest() override
    {
        beginTest("flat EQ (all gains 0dB) passes a 1kHz sine through with unchanged RMS");
        {
            EQEffect eq;
            eq.prepare(testSampleRate, 4800);

            auto buffer = makeSineBuffer(1000.0f, 4800);
            const auto inputRms = settledRms(buffer);

            eq.process(buffer);
            const auto outputRms = settledRms(buffer);

            expectWithinAbsoluteError(outputRms, inputRms, 0.02f);
        }

        beginTest("boosting the low shelf raises RMS of a low-frequency sine well above a high-frequency sine");
        {
            EQEffect flatEq;
            flatEq.prepare(testSampleRate, 4800);
            auto lowFlat = makeSineBuffer(80.0f, 4800);
            flatEq.process(lowFlat);
            const auto lowFlatRms = settledRms(lowFlat);

            EQEffect boostedEq;
            boostedEq.prepare(testSampleRate, 4800);
            boostedEq.setLowShelfGainDecibels(12.0f);

            auto lowBoosted = makeSineBuffer(80.0f, 4800);
            boostedEq.process(lowBoosted);
            const auto lowBoostedRms = settledRms(lowBoosted);

            auto highBoosted = makeSineBuffer(8000.0f, 4800);
            boostedEq.process(highBoosted);
            const auto highBoostedRms = settledRms(highBoosted);

            auto highFlatEq = EQEffect();
            highFlatEq.prepare(testSampleRate, 4800);
            auto highFlat = makeSineBuffer(8000.0f, 4800);
            highFlatEq.process(highFlat);
            const auto highFlatRms = settledRms(highFlat);

            expect(lowBoostedRms > lowFlatRms * 1.5f);
            expectWithinAbsoluteError(highBoostedRms, highFlatRms, 0.05f);
        }
    }
};

static EQEffectTests eqEffectTests;

}
