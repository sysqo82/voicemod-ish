#include "PitchEffect.h"
#include <cmath>

namespace voicemod
{

void PitchEffect::prepare(double, int)
{
    delayLine.fill(0.0f);
    writePos = 0;
    tap1Phase = 0.0f;
    tap2Phase = (float) windowSizeSamples / 2.0f;
}

void PitchEffect::process(juce::AudioBuffer<float>& buffer)
{
    auto* data = buffer.getWritePointer(0);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        writeSample(data[i]);

        float y;

        if (std::abs(pitchRatio - 1.0f) < 1.0e-4f)
        {
            // No shift: a single fixed-delay read is a clean passthrough, no crossfade needed.
            y = readInterpolated((float) windowSizeSamples / 2.0f);
        }
        else
        {
            const auto read1 = readInterpolated(tap1Phase);
            const auto read2 = readInterpolated(tap2Phase);
            const auto w1 = triangularWindow(tap1Phase / (float) windowSizeSamples);
            const auto w2 = triangularWindow(tap2Phase / (float) windowSizeSamples);

            y = read1 * w1 + read2 * w2;

            const auto delta = 1.0f - pitchRatio;
            tap1Phase += delta;
            tap2Phase += delta;
            wrapPhase(tap1Phase);
            wrapPhase(tap2Phase);
        }

        data[i] = y;
    }
}

void PitchEffect::setPitchSemitones(float semitones)
{
    pitchRatio = std::pow(2.0f, semitones / 12.0f);
}

void PitchEffect::writeSample(float x)
{
    delayLine[(size_t) writePos] = x;
    writePos = (writePos + 1) % bufferSizeSamples;
}

float PitchEffect::readInterpolated(float delaySamples) const
{
    float readPos = (float) writePos - delaySamples;
    while (readPos < 0.0f)
        readPos += (float) bufferSizeSamples;

    const auto index0 = (int) readPos % bufferSizeSamples;
    const auto index1 = (index0 + 1) % bufferSizeSamples;
    const auto frac = readPos - std::floor(readPos);

    return delayLine[(size_t) index0] * (1.0f - frac) + delayLine[(size_t) index1] * frac;
}

float PitchEffect::triangularWindow(float phase01)
{
    return 1.0f - std::abs(2.0f * phase01 - 1.0f);
}

void PitchEffect::wrapPhase(float& phase)
{
    if (phase >= (float) windowSizeSamples)
        phase -= (float) windowSizeSamples;
    else if (phase < 0.0f)
        phase += (float) windowSizeSamples;
}

}
