/*
  ==============================================================================

    SmoothSlider.cpp
    Created: 27 Nov 2022 2pm
    Author:  bgill

  ==============================================================================
*/
namespace punch
{
    using Slider = juce::Slider;
    using Mouse = juce::MouseEvent;
    using Wheel = juce::MouseWheelDetails;

    void SmoothSlider::mouseWheelMove(const Mouse& mouse, const Wheel& wheel)
    {
        auto newWheel = wheel;
        const auto speed = mouse.mods.isCtrlDown() ? SensitiveWheel : NormalWheel;
        newWheel.deltaY *= speed;
        
        Slider::mouseWheelMove(mouse, newWheel);
    }  
}