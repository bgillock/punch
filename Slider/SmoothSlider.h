/*
  ==============================================================================

    SmoothSlider.h
    Created: 27 Nov 2022 2pm
    Author:  bgill

  ==============================================================================
*/

#pragma once

namespace punch
{
    class SmoothSlider : public juce::Slider
    {
    public:
        SmoothSlider(float nWheel, float sWheel) : Slider(), NormalWheel(nWheel), SensitiveWheel(sWheel) {}

        void mouseWheelMove(const Mouse& mouse, const Wheel& wheel) override;
    private:
        float NormalWheel = .8f;
        float SensitiveWheel = .05f;
    };
}