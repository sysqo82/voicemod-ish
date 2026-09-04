#pragma once

#include "Effect.h"

namespace voicemod
{

// Single soft-clip/overdrive algorithm with one Drive knob, per CONTEXT.md/SPEC.md "Distortion".
class DistortionEffect : public Effect
{
public:
    void prepare(double sampleRate, int maximumBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;

    // 0 = unaffected passthrough, 1 = full tanh soft-clip character.
    void setDrive(float drive01);

private:
    static float waveshape(float x);

    static constexpr float shapeAmount = 5.0f;
    float drive = 0.0f;
};

}
