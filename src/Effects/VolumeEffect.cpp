#include "VolumeEffect.h"

namespace voicemod
{

void VolumeEffect::prepare(double sampleRate, int)
{
    smoothedGain.reset(sampleRate, 0.02);
}

void VolumeEffect::process(juce::AudioBuffer<float>& buffer)
{
    if (muted)
    {
        buffer.clear();
        return;
    }

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto gain = smoothedGain.getNextValue();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.getWritePointer(channel)[sample] *= gain;
    }
}

void VolumeEffect::setGainDecibels(float newGainDecibels)
{
    smoothedGain.setTargetValue(juce::Decibels::decibelsToGain(newGainDecibels, -100.0f));
}

void VolumeEffect::setMuted(bool shouldBeMuted)
{
    muted = shouldBeMuted;
}

}
