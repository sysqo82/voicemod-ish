#include "EQEffect.h"

namespace voicemod
{

void EQEffect::prepare(double newSampleRate, int maximumBlockSize)
{
    sampleRate = newSampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maximumBlockSize, 1 };
    lowShelf.prepare(spec);
    midPeak.prepare(spec);
    highShelf.prepare(spec);

    updateLowShelf();
    updateMidPeak();
    updateHighShelf();
}

void EQEffect::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    lowShelf.process(context);
    midPeak.process(context);
    highShelf.process(context);
}

void EQEffect::setLowShelfGainDecibels(float gainDb)
{
    lowShelfGainDb = gainDb;
    updateLowShelf();
}

void EQEffect::setMidPeakGainDecibels(float gainDb)
{
    midPeakGainDb = gainDb;
    updateMidPeak();
}

void EQEffect::setHighShelfGainDecibels(float gainDb)
{
    highShelfGainDb = gainDb;
    updateHighShelf();
}

void EQEffect::updateLowShelf()
{
    *lowShelf.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, lowShelfFrequencyHz, 0.707f, juce::Decibels::decibelsToGain(lowShelfGainDb));
}

void EQEffect::updateMidPeak()
{
    *midPeak.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, midPeakFrequencyHz, 1.0f, juce::Decibels::decibelsToGain(midPeakGainDb));
}

void EQEffect::updateHighShelf()
{
    *highShelf.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, highShelfFrequencyHz, 0.707f, juce::Decibels::decibelsToGain(highShelfGainDb));
}

}
