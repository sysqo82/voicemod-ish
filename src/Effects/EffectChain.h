#pragma once

#include "Effect.h"
#include <memory>
#include <vector>

namespace voicemod
{

// Fixed-order Volume->EQ->Pitch->Echo->Distortion->Reverb chain, per CONTEXT.md "Effect Chain".
class EffectChain
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void addEffect(std::unique_ptr<Effect> effect);

    // See CONTEXT.md "Bypass": skips the whole chain, distinct from per-effect enabled flags.
    bool bypassed = false;

private:
    std::vector<std::unique_ptr<Effect>> effects;
};

}
