/*
  ==============================================================================

    StereoLevelMeter.h
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include "../punch.h"

namespace punch {

    //==============================================================================
    class LevelMeter : public juce::Component
    {
    public:
        LevelMeter(int marginTop, int marginBottom, float minAmp, float maxAmp, float) :
            _mTop(marginTop),
            _mBottom(marginBottom),
            maxAmp(minAmp, maxAmp, 20, 10) {};
        void paint(juce::Graphics&);
        virtual void resized() override = 0;
        void capture(juce::AudioBuffer<float> amps, int channel);
        void capture(juce::AudioBuffer<double> amps, int channel);
        virtual void drawLight(juce::Graphics& g, int x, int y, int width, int height, float* levels, int l) = 0;
        virtual void drawSignal(juce::Graphics& g, int x, int y, int width, int height, bool signal) = 0;
        virtual void drawClipped(juce::Graphics& g, int x, int y, int width, int height, bool clipped) = 0;
        virtual int getActualHeight() = 0;
        virtual void setHeight(int height) = 0;
        virtual bool canSetRange() = 0;
        virtual void setRedLevel(float) {};
        virtual void setOrangeLevel(float) {};
        MaximumAmp maxAmp;
        int _mTop;
        int _mBottom;
        int _peakholdTimes = 0;
        int _lightheight = 0;
        int _lightwidth = 0;
        int _spacing = 0;
        int _clippedheight = 0;
        int _signalheight = 0;
        int _nLights = 0;
    };

    class DrawnLEDLevelMeter : public LevelMeter
    {
    public:
        DrawnLEDLevelMeter(int marginTop, int marginBottom, float minAmp, float maxAmp, float incAmp);
        void resized() override;
        void setHeight(int height);
        int getActualHeight();
        void clearClipped();
        void drawLight(juce::Graphics& g, int x, int y, int width, int height, float* levels, int l);
        void drawSignal(juce::Graphics& g, int x, int y, int width, int height, bool signal);
        void drawClipped(juce::Graphics& g, int x, int y, int width, int height, bool clipped);
        bool canSetRange() { return false; }
    private:
        const juce::Colour _peakColor = juce::Colour::fromRGB(255, 0, 0);
        const juce::Colour _signalColor = juce::Colour::fromRGB(0, 255, 0);
        const float _lightborder = 1.5;
        juce::Colour* _lightColors = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawnLEDLevelMeter);
    };

    class UADLevelMeter : public LevelMeter
    {
    public:
        UADLevelMeter(int marginTop, int marginBottom, float minAmp, float maxAmp, float incAmp);
        void resized() override;
        void setHeight(int height);
        int getActualHeight();
        void clearClipped();
        void drawLight(juce::Graphics& g, int x, int y, int width, int height, float* levels, int l);
        void drawSignal(juce::Graphics& g, int x, int y, int width, int height, bool signal);
        void drawClipped(juce::Graphics& g, int x, int y, int width, int height, bool clipped);
        bool canSetRange() { return true; }
        void setRedLevel(float level);
        void setOrangeLevel(float level);
    private:
        const int _clippedImageOn = 3;
        const int _clippedImageOff = 0;
        const int _signalImageOn = 5;
        const int _signalImageOff = 2;
        float _redLevel = -3.0;
        float _orangeLevel = -18.0;
        int* _lightImageIndexes = nullptr;
        juce::Image _lightImages;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UADLevelMeter);
    };
    class StereoLevelMeter : public juce::Component,
        public juce::Timer
    {
    public:
        StereoLevelMeter(float minAmp, float maxAmp, float incAmp, int marginTop, int marginBottom, float leftAnnoWidth, float rightAnnoWidth);
        void resized() override;
        void capture(juce::AudioBuffer<float> amps);
        void capture(juce::AudioBuffer<double> amps);
        void init();
        void clearClipped();
        bool canSetRange();
        void setRange(juce::Range<double> r);
        int getActualHeight();
        void setHeight(int height);
        void timerCallback() override;
    private:

        UADLevelMeter leftLevelMeter;
        UADLevelMeter rightLevelMeter;
        dbAnnoComponent leftAnno;
        dbAnnoComponent rightAnno;
        float _leftAnnoWidth;
        float _rightAnnoWidth;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoLevelMeter);
    };

    namespace LevelMeterBinaryData
    {

        extern const char* APILights_png;
        const int            APILights_pngSize = 20615;

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
