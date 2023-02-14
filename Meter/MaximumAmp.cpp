/*
  ==============================================================================

    MeterBuffer.cpp
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/
#include "../punch.h"

namespace punch {

    MaximumAmp::MaximumAmp(double min, double max, int nLevels, int peakHoldTimes)
    {
        _nLevels = nLevels;
        _minAmp = min;
        _maxAmp = max;
        _peakHoldTimes = peakHoldTimes;
        _levels = new float[_nLevels];
        _signal = false;
        _clipped = false;
        _peakAmp = -144.0;
    };
    void MaximumAmp::setNLevels(int n)
    {
        delete _levels;
        _nLevels = n;
        _levels = new float[_nLevels];
    }
    void MaximumAmp::capture(juce::AudioBuffer<float> amps, int channel)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            float db = juce::Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
            if (db > _peakAmp) _peakAmp = db;
            _clipped = _clipped || (db > _maxAmp);
            _signal = _signal || (amps.getMagnitude(channel, 0, amps.getNumSamples()) > 0.0);
        }
    }
    void MaximumAmp::capture(juce::AudioBuffer<double> amps, int channel)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            double db = juce::Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
            if (db > _peakAmp) _peakAmp = db;
            _clipped = _clipped || (db > _maxAmp);
            _signal = _signal || (amps.getMagnitude(channel, 0, amps.getNumSamples()) > 0.0);
        }
    }
    float* MaximumAmp::getLevels()
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        if (++_peakTimes > _peakHoldTimes)
        {
            _peakTimes = 0;
            _peakhold = -144.0;
        }

        if (_peakAmp > _peakhold)
        {
            _peakhold = _peakAmp;
            _peakTimes = 0;
        }

        int l = (int)((float)(_peakAmp - _minAmp) / (_maxAmp - _minAmp) * (float)_nLevels);
        int h = (int)((float)(_peakhold - _minAmp) / (_maxAmp - _minAmp) * (float)_nLevels);
        for (int i = 0; i < _nLevels; i++) {
            _levels[i] = 0.0;
            if (i == h) _levels[i] = 1.0;
            if (i <= l) _levels[i] = 1.0;
        }
        return _levels;
    }
    bool MaximumAmp::clipped()
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        return _clipped;
    }
    void MaximumAmp::setClipped(bool clip)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            _clipped = clip;
        }
    }
    bool MaximumAmp::signal()
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        return _signal;
    }
    void MaximumAmp::clear()
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            _peakAmp = -144.0;
            _signal = false;
        }
    }
    
    void SimpleBuffer::init(int maxsize, int numChannels, bool isUsingDoublePrecision)
    {
        _isUsingDouble = isUsingDoublePrecision;
        _maxSize = maxsize;
        _nChannels = numChannels;
        _latencySamples = 0;
        if (!_isUsingDouble)
        {
            _floatBuffer = std::make_unique<juce::AudioBuffer<float>>(_nChannels, _maxSize);
            _doubleBuffer = std::make_unique<juce::AudioBuffer<double>>(1, 1);
        }
        else
        {
            _floatBuffer = std::make_unique<juce::AudioBuffer<float>>(1, 1);
            _doubleBuffer = std::make_unique<juce::AudioBuffer<double>>(_nChannels, _maxSize);
        }
        _nSamples = 0;
    }

    void SimpleBuffer::clear()
    {
        const juce::SpinLock::ScopedTryLockType lock(_mutex);
        if (lock.isLocked())
        {
            _nSamples = 0;
        }
    }
    int SimpleBuffer::getNSamples()
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        return _nSamples;
    }
    int SimpleBuffer::getSize()
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        return _maxSize;
    }
    int SimpleBuffer::getLatencySamples()
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        return _latencySamples;
    }
    int SimpleBuffer::getNChannels()
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        return _nChannels;
    }
    bool SimpleBuffer::getIsUsingDouble()
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        return _isUsingDouble;
    }
    void SimpleBuffer::capture(juce::AudioBuffer<float>& bamps, juce::AudioBuffer<float>& aamps, int latency)
    {
        const juce::SpinLock::ScopedTryLockType lock(_mutex);
        jassert(!_isUsingDouble);
        if (lock.isLocked())
        {
            _latencySamples = latency;
            int nsamps = bamps.getNumSamples();
            int nchannels = bamps.getNumChannels();
            if (nsamps + _nSamples >= _maxSize)
            {
                // buffer overflow!!
                _nSamples = 0;
            }
            for (int c = 0; c < nchannels; c++)
            {
                auto readptr = bamps.getReadPointer(c);
                _floatBuffer->copyFrom(c, _nSamples, readptr, nsamps);
            }
            for (int c = 0; c < nchannels; c++)
            {
                auto readptr = aamps.getReadPointer(c);
                _floatBuffer->copyFrom(c + nchannels, _nSamples, readptr, nsamps);

            }
            _nSamples += nsamps;
        }
    }
    void SimpleBuffer::append(juce::AudioBuffer<float> amps, int nsamps)
    {
        const juce::SpinLock::ScopedTryLockType lock(_mutex);
        jassert(!_isUsingDouble);
        if (lock.isLocked())
        {
            int nchannels = amps.getNumChannels();
            if (nsamps + _nSamples >= _maxSize)
            {
                // buffer overflow!!
                _nSamples = 0;
            }
            for (int c = 0; c < nchannels; c++)
            {
                auto readptr = amps.getReadPointer(c);
                _floatBuffer->copyFrom(c, _nSamples, readptr, nsamps);
                //float r = _floatBuffer->getRMSLevel(c, _nSamples, nsamps);
                //float ar = amps.getRMSLevel(c, 0, nsamps);
                //int x = 0;
            }
            _nSamples += nsamps;
        }
    }
    void SimpleBuffer::append(juce::AudioBuffer<float> amps, int start, int n)
    {
        const juce::SpinLock::ScopedTryLockType lock(_mutex);
        jassert(!_isUsingDouble);
        if (lock.isLocked())
        {
            int nchannels = amps.getNumChannels();
            if (n + _nSamples >= _maxSize)
            {
                // buffer overflow!!
                _nSamples = 0;
            }
            for (int c = 0; c < nchannels; c++)
            {
                auto readptr = amps.getReadPointer(c);
                _floatBuffer->copyFrom(c, _nSamples, &readptr[start], n);
            }
            _nSamples += n;
        }
    }
    void SimpleBuffer::trimStart(int size)
    {
        const juce::SpinLock::ScopedTryLockType lock(_mutex);
        jassert(!_isUsingDouble);
        if (lock.isLocked())
        {
            for (int c = 0; c < _nChannels; c++)
            {
                auto readptr = _floatBuffer->getReadPointer(c);
                _floatBuffer->copyFrom(c, 0, &readptr[size], _nSamples-size);
            }
            _nSamples -= size;
        }
    }
    void SimpleBuffer::capture(juce::AudioBuffer<double>& bamps, juce::AudioBuffer<double>& aamps, int latency)
    {
        const juce::SpinLock::ScopedTryLockType lock(_mutex);
        jassert(_isUsingDouble);
        if (lock.isLocked())
        {
            int nsamps = bamps.getNumSamples();
            int nchannels = bamps.getNumChannels();
            if (nsamps + _nSamples >= _maxSize)
            {
                // buffer overflow!!
                _nSamples = 0;
            }
            for (int c = 0; c < nchannels; c++)
            {
                auto readptr = bamps.getReadPointer(c);
                _doubleBuffer->copyFrom(c, _nSamples, readptr, nsamps);
            }
            for (int c = 0; c < aamps.getNumChannels(); c++)
            {
                auto readptr = aamps.getReadPointer(c);
                _doubleBuffer->copyFrom(c + nchannels, _nSamples, readptr, nsamps);
            }
            _nSamples += nsamps;
        }
    }
    juce::AudioBuffer<float> SimpleBuffer::getBuffer()
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        jassert(!_isUsingDouble);
        return *_floatBuffer;
    }
    const float* SimpleBuffer::getChannelReadPtr(int channel)
    {
        return _floatBuffer->getReadPointer(channel);
    }
    int SimpleBuffer::getSamples(int channel, double* samplesreturned)
    {
        const juce::SpinLock::ScopedLockType lock(_mutex);
        jassert(_isUsingDouble);
        auto readptr = _doubleBuffer->getReadPointer(channel);
        juce::FloatVectorOperations::copy(samplesreturned, readptr, _nSamples);
        return _nSamples;
    }
    class VUHistogram
    {
    public:
        VUHistogram(int nBins, int nBuffs, double minBin, double maxBin)
        {
            _nBins = nBins;
            _nBuffs = nBuffs;
            _minBin = minBin;
            _maxBin = maxBin;
            _hist = new int* [_nBuffs];
            for (int i = 0; i < _nBuffs; i++)
            {
                _hist[i] = new int[_nBins + 2]; // for min/max
                clearBuff(_hist[i]);
            }
            _currentBuff = 0;
        }
        void addAmps(juce::AudioBuffer<float>& amps, int channel)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);
            if (lock.isLocked())
            {
                clearBuff(_hist[_currentBuff]);
                auto channelData = amps.getReadPointer(channel);
                for (int a = 0; a < amps.getNumSamples(); a++)
                {
                    _hist[_currentBuff][getBin(channelData[a], _minBin, _maxBin, _nBins)]++;
                }
            }
        }
        int get_NBuffs()
        {
            return _nBuffs;
        }
        int getNBins()
        {
            return _nBins;
        }
        int getTimeWeightedHistogram(float* histOut)
        {
            // Return 2d array of histogram values starting with the oldest in time and maxBin.
            int b = (_currentBuff - 1); if (b < 0) b = _nBuffs - 1;
            int n = 0;
            while (n < _nBuffs)
            {
                for (int h = 0; h < _nBins; h++)
                {

                }
                b--; if (b < 0) b = _nBuffs - 1;
                n++;
            }
            _currentBuff++;
            if (_currentBuff >= _nBuffs) _currentBuff = 0;
        }
        void getHistTotal(int* tot)
        {
            const juce::SpinLock::ScopedLockType lock(mutex);
            for (int i = 0; i <= _nBins + 1; i++)
            {
                tot[i] = 0;
                for (int b = 0; b < _nBuffs; b++)
                {
                    tot[i] += _hist[b][i];
                }
            }
            return;
        }

    private:
        void clearBuff(int* buff)
        {
            for (int i = 0; i < _nBins + 2; i++)
            {
                buff[i] = 0;
            }
            return;
        }
        int getBin(double amp, double min, double max, int nBins)
        {
            if (amp < min) return 0;
            if (amp > max) return nBins + 1;
            return (int)(((amp - min) / (max - min)) * (double)nBins);
        }
        juce::SpinLock mutex;
        int _nBins;
        int _nBuffs;
        double _minBin;
        double _maxBin;
        int** _hist;
        int _currentBuff;
    };
}