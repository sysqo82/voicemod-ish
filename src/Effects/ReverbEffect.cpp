#include "ReverbEffect.h"

namespace voicemod
{

void ReverbEffect::prepare(double sampleRate, int maximumBlockSize)
{
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maximumBlockSize, 1 };
    reverb.prepare(spec);
    reverb.reset();
    updateParameters();
}

void ReverbEffect::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

void ReverbEffect::setRoomSize(float value01)
{
    parameters.roomSize = juce::jlimit(0.0f, 1.0f, value01);
    updateParameters();
}

void ReverbEffect::setDamping(float value01)
{
    parameters.damping = juce::jlimit(0.0f, 1.0f, value01);
    updateParameters();
}

void ReverbEffect::setWetDryMix(float wet01)
{
    const auto wet = juce::jlimit(0.0f, 1.0f, wet01);

    // juce::Reverb internally scales dryLevel by 2 and wetLevel by 3 before applying them as
    // gains, so these divisors keep wet=0/wet=1 at unity gain at each end of the mix range.
    parameters.dryLevel = (1.0f - wet) * 0.5f;
    parameters.wetLevel = wet / 3.0f;
    updateParameters();
}

void ReverbEffect::updateParameters()
{
    reverb.setParameters(parameters);
}

}
