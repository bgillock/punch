/*
  ==============================================================================

    MaximumAmp.h
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/

#pragma once
#include "../punch.h"

namespace punch {

    class MaximumAmp
    {
    public:
        MaximumAmp(double minAmp, double maxAmp, int nLevels, int peakHoldTimes);
        void capture(juce::AudioBuffer<float> amps, int channel);
        void capture(juce::AudioBuffer<double> amps, int channel);
        void clear();
        float* getLevels();
        double getMinAmp() { return _minAmp; }
        double getMaxAmp() { return _maxAmp; }
        bool clipped();
        void setClipped(bool);
        bool signal();
        int getNLevels() { return _nLevels; }
        void setNLevels(int n);
    private:
        double _minAmp;
        double _maxAmp;
        double _peakAmp;
        int _nLevels;
        juce::SpinLock mutex;
        float* _levels;
        int _peakHoldTimes;
        int _lastlight = 0;
        double _peakhold = 0.0;
        int _peakTimes = 0;
        bool _signal;
        bool _clipped;
    };

    class SimpleBuffer
    {
    public:
        SimpleBuffer() { _maxSize = 0; };
        void init(int size, int nChannels, bool isUsingDouble);
        void capture(juce::AudioBuffer<float> amps, int channel);
        void capture(juce::AudioBuffer<double> amps, int channel);
        void clear();
        int getNSamples();
        int getSize();
        int getNChannels();
        bool getIsUsingDouble();
        int getSamples(int channel, float* samplesreturned);
        int getSamples(int channel, double* samplesreturned);
    private:
        std::unique_ptr<juce::AudioBuffer<float>> _floatBuffer;
        std::unique_ptr<juce::AudioBuffer<double>> _doubleBuffer;
        juce::SpinLock _mutex;
        int _maxSize;
        int _nSamples;
        int _nChannels;
        bool _isUsingDouble;
    };
}