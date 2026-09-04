#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace voicemod
{

// A single DSP stage in the EffectChain; see CONTEXT.md "Effect".
class Effect
{
public:
    virtual ~Effect() = default;

    virtual void prepare(double sampleRate, int maximumBlockSize) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;

    bool enabled = true;
};

}
