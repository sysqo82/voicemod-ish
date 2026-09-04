#include "EchoEffect.h"
#include <cmath>

namespace voicemod
{

void EchoEffect::prepare(double newSampleRate, int)
{
    sampleRate = newSampleRate;
    resizeDelayLine();
}

void EchoEffect::process(juce::AudioBuffer<float>& buffer)
{
    auto* data = buffer.getWritePointer(0);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto x = data[i];
        const auto delayed = delayLine[(size_t) pos];

        data[i] = x * (1.0f - mix) + delayed * mix;
        delayLine[(size_t) pos] = x + delayed * feedback;

        pos = (pos + 1) % (int) delayLine.size();
    }
}

void EchoEffect::setDelayTimeSeconds(float seconds)
{
    delayTimeSeconds = seconds;
    resizeDelayLine();
}

void EchoEffect::setFeedback(float newFeedback)
{
    feedback = juce::jlimit(0.0f, 0.95f, newFeedback);
}

void EchoEffect::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
}

void EchoEffect::resizeDelayLine()
{
    const auto numSamples = juce::jmax(1, (int) std::round(delayTimeSeconds * sampleRate));
    delayLine.assign((size_t) numSamples, 0.0f);
    pos = 0;
}

}
