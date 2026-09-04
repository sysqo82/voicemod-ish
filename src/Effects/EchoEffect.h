#pragma once

#include "Effect.h"
#include <vector>

namespace voicemod
{

// Single-tap feedback delay (delay-time, feedback, mix), per CONTEXT.md/SPEC.md "Echo".
class EchoEffect : public Effect
{
public:
    void prepare(double sampleRate, int maximumBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void setDelayTimeSeconds(float seconds);
    void setFeedback(float newFeedback);
    void setMix(float newMix);

private:
    void resizeDelayLine();

    double sampleRate = 48000.0;
    float delayTimeSeconds = 0.3f;
    float feedback = 0.3f;
    float mix = 0.3f;

    std::vector<float> delayLine;
    int pos = 0;
};

}
