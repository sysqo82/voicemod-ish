#include "EffectChain.h"

namespace voicemod
{

void EffectChain::prepare(double sampleRate, int maximumBlockSize)
{
    for (auto& effect : effects)
        effect->prepare(sampleRate, maximumBlockSize);
}

void EffectChain::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    for (auto& effect : effects)
        if (effect->enabled)
            effect->process(buffer);
}

void EffectChain::addEffect(std::unique_ptr<Effect> effect)
{
    effects.push_back(std::move(effect));
}

}
