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
        void capture(juce::AudioBuffer<float> bamps, juce::AudioBuffer<float> aamps);
        void capture(juce::AudioBuffer<double> bamps, juce::AudioBuffer<double> aamps);
        void clear();
        int getNSamples();
        int getSize();
        int getNChannels();
        bool getIsUsingDouble();
        juce::AudioBuffer<float> getBuffer();
        void append(juce::AudioBuffer<float> amps, int n);
        void append(juce::AudioBuffer<float> amps, int start, int n);
        void trimStart(int size);
        const float* getChannelReadPtr(int channel);
        int getSamples(int channel, double* samplesreturned);
        void dump(std::string pre)
        {
            int a = 0;

            for (int c = 0; c < _floatBuffer->getNumChannels(); c++)
            {
                std::string out = ""; 
                out += std::to_string(c) + "=[";
                auto readptr = _floatBuffer->getReadPointer(c);
                for (int i = 0; i < std::min(8,_nSamples); i++)
                {
                    out += std::to_string(readptr[i]) + ",";
                }
                continue;
            }
            return;
        };
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