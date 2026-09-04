#include "../src/Effects/EchoEffect.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace voicemod
{

namespace
{

constexpr double testSampleRate = 48000.0;

juce::AudioBuffer<float> makeImpulseBuffer(int numSamples)
{
    juce::AudioBuffer<float> buffer(1, numSamples);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    return buffer;
}

}

class EchoEffectTests : public juce::UnitTest
{
public:
    EchoEffectTests() : juce::UnitTest("EchoEffect", "Effects") {}

    void runTest() override
    {
        beginTest("mix=0 passes the dry signal through unchanged");
        {
            EchoEffect echo;
            echo.prepare(testSampleRate, 512);
            echo.setDelayTimeSeconds(100.0f / (float) testSampleRate);
            echo.setFeedback(0.9f);
            echo.setMix(0.0f);

            auto buffer = makeImpulseBuffer(300);
            echo.process(buffer);

            expectEquals(buffer.getSample(0, 0), 1.0f);
            for (int i = 1; i < buffer.getNumSamples(); ++i)
                expectEquals(buffer.getSample(0, i), 0.0f);
        }

        beginTest("mix=1, feedback=0 delays the impulse by the configured delay time, no repeats");
        {
            EchoEffect echo;
            echo.prepare(testSampleRate, 512);
            echo.setDelayTimeSeconds(100.0f / (float) testSampleRate);
            echo.setFeedback(0.0f);
            echo.setMix(1.0f);

            auto buffer = makeImpulseBuffer(300);
            echo.process(buffer);

            expectWithinAbsoluteError(buffer.getSample(0, 100), 1.0f, 1.0e-5f);
            expectWithinAbsoluteError(buffer.getSample(0, 200), 0.0f, 1.0e-5f);
        }

        beginTest("mix=1, feedback=0.5 produces a geometrically decaying series of repeats");
        {
            EchoEffect echo;
            echo.prepare(testSampleRate, 512);
            echo.setDelayTimeSeconds(100.0f / (float) testSampleRate);
            echo.setFeedback(0.5f);
            echo.setMix(1.0f);

            auto buffer = makeImpulseBuffer(400);
            echo.process(buffer);

            expectWithinAbsoluteError(buffer.getSample(0, 100), 1.0f, 1.0e-5f);
            expectWithinAbsoluteError(buffer.getSample(0, 200), 0.5f, 1.0e-5f);
            expectWithinAbsoluteError(buffer.getSample(0, 300), 0.25f, 1.0e-5f);
        }
    }
};

static EchoEffectTests echoEffectTests;

}
