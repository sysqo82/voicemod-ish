#pragma once

#include "Effect.h"
#include <juce_dsp/juce_dsp.h>

namespace voicemod
{

// 3-band EQ (low shelf, mid peak, high shelf) at fixed frequencies, gain-only per band.
class EQEffect : public Effect
{
public:
    void prepare(double sampleRate, int maximumBlockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void setLowShelfGainDecibels(float gainDb);
    void setMidPeakGainDecibels(float gainDb);
    void setHighShelfGainDecibels(float gainDb);

private:
    void updateLowShelf();
    void updateMidPeak();
    void updateHighShelf();

    static constexpr float lowShelfFrequencyHz = 200.0f;
    static constexpr float midPeakFrequencyHz = 1000.0f;
    static constexpr float highShelfFrequencyHz = 4000.0f;

    double sampleRate = 48000.0;
    float lowShelfGainDb = 0.0f;
    float midPeakGainDb = 0.0f;
    float highShelfGainDb = 0.0f;

    juce::dsp::IIR::Filter<float> lowShelf;
    juce::dsp::IIR::Filter<float> midPeak;
    juce::dsp::IIR::Filter<float> highShelf;
};

}
