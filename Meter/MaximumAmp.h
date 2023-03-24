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

    enum AmpType {
        RMS, Peak
    };

    class AmpCapture
    {
    public:
        AmpCapture(double minAmp, double maxAmp, int nLevels, AmpType ampType = AmpType::Peak);
        virtual void capture(juce::AudioBuffer<float> amps, int channel) = 0;
        virtual void capture(juce::AudioBuffer<double> amps, int channel) = 0;
        virtual void clear() = 0;
        virtual float* getLevels() = 0;        
        virtual void setNLevels(int n) = 0;
        double getMinAmp() { return _minAmp; }
        double getMaxAmp() { return _maxAmp; }
        bool clipped();
        void setClipped(bool);
        bool signal();
        int getNLevels() { return _nLevels; }

    protected:
        float getMaxPeak(juce::AudioBuffer<float> amps, int channel, int startSample, int nSamples);
        double getMaxPeak(juce::AudioBuffer<double> amps, int channel, int startSample, int nSamples);
        AmpType _ampType;
        double  _minAmp;
        double _maxAmp;
        int _nLevels;
        juce::SpinLock mutex;
        bool _signal;
        bool _clipped;
    };

    class HistogramAmp : public AmpCapture
    {
    public:
        HistogramAmp(double minAmp, double maxAmp, int nLevels) : 
            AmpCapture(minAmp, maxAmp, nLevels) 
        {
            _peakHoldTimes = 10;
            _levels = new float[_nLevels];
        };
        void capture(juce::AudioBuffer<float> amps, int channel) override;
        void capture(juce::AudioBuffer<double> amps, int channel) override;
        void clear() override;
        float* getLevels() override;
        void setNLevels(int n) override;
    private:
        double _peakAmp;
        float* _levels;
        int _peakHoldTimes;
        int _lastlight = 0;
        double _peakhold = 0.0;
        int _peakTimes = 0;
    };

    class MaximumAmp : public AmpCapture
    {
    public:
        MaximumAmp(double minAmp, double maxAmp, int nLevels) :
            AmpCapture(minAmp, maxAmp, nLevels)
        {
            _levels = new float[_nLevels];
            _peakHoldTimes = 10;
        };
        void capture(juce::AudioBuffer<float> amps, int channel) override;
        void capture(juce::AudioBuffer<double> amps, int channel) override;
        void clear() override;
        float* getLevels() override;
        void setNLevels(int n) override;
    private:
        double _peakAmp;
        float* _levels;
        int _peakHoldTimes;
        int _lastlight = 0;
        double _peakhold = 0.0;
        int _peakTimes = 0;
    };
    class SimpleBuffer
    {
    public:
        SimpleBuffer() { _maxSize = 0; };
        void init(int size, int nChannels, bool isUsingDouble);
        void capture(juce::AudioBuffer<float>& amps);
        void capture(juce::AudioBuffer<double>& amps);
        void clear();
        int getNSamples();
        int getSize();
        int getNChannels();
        bool getIsUsingDouble();
        juce::AudioBuffer<float> *getBuffer();
        void append(juce::AudioBuffer<float>& amps, int n);
        void trimStart(int size);
        const float* getChannelReadPtr(int channel);
        const float* getChannelWritePtr(int channel);
        void dump(std::string pre)
        {
            int a = 0;

            for (int c = 0; c < _floatBuffer->getNumChannels(); c++)
            {
                std::string out = "";
                out += std::to_string(c) + "=[";
                auto readptr = _floatBuffer->getReadPointer(c);
                for (int i = 0; i < std::min(8, _nSamples); i++)
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

    class CompareBuffer
    {
    public:
        CompareBuffer() { _maxSize = 0; };
        void init(int size, int nChannels, bool isUsingDouble);
        void capture(juce::AudioBuffer<float>& bamps, juce::AudioBuffer<float>& aamps, int latency);
        void capture(juce::AudioBuffer<double>& bamps, juce::AudioBuffer<double>& aamps, int latency);
        void clear();
        int getNSamples();
        int getSize();
        int getNChannels();
        int getLatencySamples();
        bool getIsUsingDouble();
        juce::AudioBuffer<float>* getBuffer();
        void append(CompareBuffer* amps, int n);
        void trimStart(int size);
        const float* getBeforeReadPtr(int channel);
        const float* getAfterReadPtr(int channel);

    private:  
        std::unique_ptr<juce::AudioBuffer<float>> _floatBeforeBuffer;
        std::unique_ptr<juce::AudioBuffer<double>> _doubleBeforeBuffer;
        std::unique_ptr<juce::AudioBuffer<float>> _floatAfterBuffer;
        std::unique_ptr<juce::AudioBuffer<double>> _doubleAfterBuffer;
        juce::SpinLock _mutex;
        int _latencySamples = 0;
        int _maxSize;
        int _nSamples;
        int _nChannels;
        bool _isUsingDouble;
    };
}