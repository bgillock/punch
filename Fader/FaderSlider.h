/*
  ==============================================================================

    FaderSlider.h
    Created: 21 Nov 2022 9:26:21am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include "../punch.h"
namespace punch 
{
class FaderSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FaderSliderLookAndFeel();
    ~FaderSliderLookAndFeel();
    void drawLinearSliderBackground(juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle, juce::Slider&) override;
    void drawLabel(juce::Graphics& g, juce::Label& label);
    int getSliderThumbRadius(juce::Slider& slider) override;


private:
    juce::Image thumbImage;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FaderSliderLookAndFeel)
};

class FaderSlider  : public punch::SmoothSlider
{
public:
    FaderSlider();
    ~FaderSlider() override;

private:
    FaderSliderLookAndFeel faderSliderLookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaderSlider)
};

namespace FaderBinaryData
{
    extern const char* faderalphanonflat_png;
    const int            faderalphanonflat_pngSize = 20692;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 1;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource(const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename(const char* resourceNameUTF8);
}
}