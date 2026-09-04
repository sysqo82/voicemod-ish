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

    // Returns a raw, non-owning pointer to the just-added effect for the caller to configure/bind to UI.
    template <typename EffectType>
    EffectType* addEffect(std::unique_ptr<EffectType> effect)
    {
        auto* rawPointer = effect.get();
        effects.push_back(std::move(effect));
        return rawPointer;
    }

    // See CONTEXT.md "Bypass": skips the whole chain, distinct from per-effect enabled flags.
    bool bypassed = false;

private:
    std::vector<std::unique_ptr<Effect>> effects;
};

}
