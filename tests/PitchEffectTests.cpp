#include "../src/Effects/PitchEffect.h"
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

// Estimates frequency via positive-going zero crossings over the back portion of the buffer,
// skipping the front portion so the delay line's fill/settling transient isn't measured.
float estimateSettledFrequencyHz(const juce::AudioBuffer<float>& buffer, int settleSamples)
{
    const auto* data = buffer.getReadPointer(0);
    const auto numSamples = buffer.getNumSamples();

    int crossings = 0;
    for (int i = settleSamples + 1; i < numSamples; ++i)
        if (data[i - 1] <= 0.0f && data[i] > 0.0f)
            ++crossings;

    const auto durationSeconds = (float) (numSamples - settleSamples) / (float) testSampleRate;
    return (float) crossings / durationSeconds;
}

float rms(const juce::AudioBuffer<float>& buffer, int startSample)
{
    return buffer.getRMSLevel(0, startSample, buffer.getNumSamples() - startSample);
}

}

class PitchEffectTests : public juce::UnitTest
{
public:
    PitchEffectTests() : juce::UnitTest("PitchEffect", "Effects") {}

    void runTest() override
    {
        constexpr int numSamples = 24000;
        constexpr int settleSamples = 8000;

        beginTest("0 semitones leaves frequency and level unchanged");
        {
            PitchEffect pitch;
            pitch.prepare(testSampleRate, 512);

            auto input = makeSineBuffer(200.0f, numSamples);
            const auto inputRms = rms(input, settleSamples);

            auto buffer = input;
            pitch.process(buffer);

            expectWithinAbsoluteError(estimateSettledFrequencyHz(buffer, settleSamples), 200.0f, 15.0f);
            expectWithinAbsoluteError(rms(buffer, settleSamples), inputRms, 0.15f);
        }

        beginTest("+12 semitones roughly doubles frequency");
        {
            PitchEffect pitch;
            pitch.prepare(testSampleRate, 512);
            pitch.setPitchSemitones(12.0f);

            auto buffer = makeSineBuffer(200.0f, numSamples);
            pitch.process(buffer);

            expectWithinAbsoluteError(estimateSettledFrequencyHz(buffer, settleSamples), 400.0f, 60.0f);
        }

        beginTest("-12 semitones roughly halves frequency");
        {
            PitchEffect pitch;
            pitch.prepare(testSampleRate, 512);
            pitch.setPitchSemitones(-12.0f);

            auto buffer = makeSineBuffer(400.0f, numSamples);
            pitch.process(buffer);

            expectWithinAbsoluteError(estimateSettledFrequencyHz(buffer, settleSamples), 200.0f, 30.0f);
        }
    }
};

static PitchEffectTests pitchEffectTests;

}
