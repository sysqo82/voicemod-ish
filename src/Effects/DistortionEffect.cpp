#include "DistortionEffect.h"
#include <cmath>

namespace voicemod
{

void DistortionEffect::prepare(double, int)
{
}

void DistortionEffect::process(juce::AudioBuffer<float>& buffer)
{
    auto* data = buffer.getWritePointer(0);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto x = data[i];
        data[i] = (1.0f - drive) * x + drive * waveshape(x);
    }
}

void DistortionEffect::setDrive(float drive01)
{
    drive = juce::jlimit(0.0f, 1.0f, drive01);
}

float DistortionEffect::waveshape(float x)
{
    return std::tanh(shapeAmount * x) / std::tanh(shapeAmount);
}

}
