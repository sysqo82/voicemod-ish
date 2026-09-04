#pragma once

#include "Effect.h"

namespace voicemod
{

// Volume effect: single gain slider (-inf..+12dB) plus mute, per CONTEXT.md/SPEC.md.
class VolumeEffect : public Effect
{
public:
    void prepare(double sampleRate, int maximumBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;

    // 0.0f == silence (-inf dB); otherwise linear gain corresponding to [-inf, +12dB].
    void setGainDecibels(float newGainDecibels);
    void setMuted(bool shouldBeMuted);

private:
    juce::LinearSmoothedValue<float> smoothedGain { 1.0f };
    bool muted = false;
};

}
