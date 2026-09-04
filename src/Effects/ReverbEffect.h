#pragma once

#include "Effect.h"
#include <juce_dsp/juce_dsp.h>

namespace voicemod
{

// Algorithmic reverb (juce::dsp::Reverb) with room-size/damping/wet-dry, per CONTEXT.md/SPEC.md.
class ReverbEffect : public Effect
{
public:
    void prepare(double sampleRate, int maximumBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void setRoomSize(float value01);
    void setDamping(float value01);
    void setWetDryMix(float wet01); // 0 = fully dry, 1 = fully wet

private:
    void updateParameters();

    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters parameters;
};

}
