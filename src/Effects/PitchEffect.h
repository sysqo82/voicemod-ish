#pragma once

#include "Effect.h"
#include <array>

namespace voicemod
{

// Simple resampling-style pitch shift (no formant preservation), per CONTEXT.md/SPEC.md.
// Implemented as a two-tap granular delay line, the standard cheap real-time pitch-shift technique:
// two overlapping reads of a circular buffer at a rate offset from the write rate, crossfaded.
class PitchEffect : public Effect
{
public:
    void prepare(double sampleRate, int maximumBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void setPitchSemitones(float semitones);

private:
    float readInterpolated(float delaySamples) const;
    void writeSample(float x);
    static float triangularWindow(float phase01);
    static void wrapPhase(float& phase);

    static constexpr int windowSizeSamples = 1024;
    static constexpr int bufferSizeSamples = 4096;

    std::array<float, bufferSizeSamples> delayLine {};
    int writePos = 0;

    float pitchRatio = 1.0f;
    float tap1Phase = 0.0f;
    float tap2Phase = (float) windowSizeSamples / 2.0f;
};

}
