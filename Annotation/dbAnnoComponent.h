/*
  ==============================================================================

    dbAnnoComponent.h
    Created: 24 Nov 2022 9:32:50am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include "../punch.h"

namespace punch {

    //==============================================================================
    /*
    */
    class dbAnnoComponent : public juce::Component
    {
    public:
        dbAnnoComponent(float minAmp, float maxAmp, float incAmp, int marginTop, int marginBottom, float annoWidth,
            juce::Justification style, bool showSignalClipped = false);
        void paint(juce::Graphics& g) override;
        void resized() override;

    private:

        float _minAmp;
        float _maxAmp;
        float _incAmp;
        int _marginTop;
        int _marginBottom;
        int _minY;
        int _maxY;
        float _annoWidth;
        juce::Justification _style;
        bool _showSignalClipped;

        float getYFromDb(double db);
        void addPair(juce::StringPairArray& pairs, juce::String format, float v, float pixel);

        juce::StringPairArray get_db_pairs(float minAmp, float maxAmp, float incAmp);
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(dbAnnoComponent)
    };
}