#include "../src/Effects/DistortionEffect.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace voicemod
{

namespace
{

juce::AudioBuffer<float> makeConstantBuffer(float value)
{
    juce::AudioBuffer<float> buffer(1, 1);
    buffer.setSample(0, 0, value);
    return buffer;
}

}

class DistortionEffectTests : public juce::UnitTest
{
public:
    DistortionEffectTests() : juce::UnitTest("DistortionEffect", "Effects") {}

    void runTest() override
    {
        beginTest("drive=0 leaves the signal unchanged");
        {
            DistortionEffect distortion;
            distortion.prepare(48000.0, 512);
            distortion.setDrive(0.0f);

            for (float x : { -1.0f, -0.5f, -0.1f, 0.0f, 0.1f, 0.5f, 1.0f })
            {
                auto buffer = makeConstantBuffer(x);
                distortion.process(buffer);
                expectWithinAbsoluteError(buffer.getSample(0, 0), x, 1.0e-6f);
            }
        }

        beginTest("drive=1 boosts a quiet signal and preserves full-scale peak level");
        {
            DistortionEffect distortion;
            distortion.prepare(48000.0, 512);
            distortion.setDrive(1.0f);

            auto quiet = makeConstantBuffer(0.1f);
            distortion.process(quiet);
            expect(quiet.getSample(0, 0) > 0.2f);

            auto fullScale = makeConstantBuffer(1.0f);
            distortion.process(fullScale);
            expectWithinAbsoluteError(fullScale.getSample(0, 0), 1.0f, 1.0e-3f);
        }

        beginTest("drive=1 is odd-symmetric: negating the input negates the output");
        {
            DistortionEffect distortion;
            distortion.prepare(48000.0, 512);
            distortion.setDrive(1.0f);

            auto positive = makeConstantBuffer(0.3f);
            distortion.process(positive);

            auto negative = makeConstantBuffer(-0.3f);
            distortion.process(negative);

            expectWithinAbsoluteError(negative.getSample(0, 0), -positive.getSample(0, 0), 1.0e-6f);
        }

        beginTest("drive=0.5 output lies strictly between the drive=0 and drive=1 outputs");
        {
            constexpr float x = 0.2f;

            DistortionEffect driveZero;
            driveZero.prepare(48000.0, 512);
            driveZero.setDrive(0.0f);
            auto zeroBuffer = makeConstantBuffer(x);
            driveZero.process(zeroBuffer);

            DistortionEffect driveOne;
            driveOne.prepare(48000.0, 512);
            driveOne.setDrive(1.0f);
            auto oneBuffer = makeConstantBuffer(x);
            driveOne.process(oneBuffer);

            DistortionEffect driveHalf;
            driveHalf.prepare(48000.0, 512);
            driveHalf.setDrive(0.5f);
            auto halfBuffer = makeConstantBuffer(x);
            driveHalf.process(halfBuffer);

            expect(halfBuffer.getSample(0, 0) > zeroBuffer.getSample(0, 0));
            expect(halfBuffer.getSample(0, 0) < oneBuffer.getSample(0, 0));
        }
    }
};

static DistortionEffectTests distortionEffectTests;

}
