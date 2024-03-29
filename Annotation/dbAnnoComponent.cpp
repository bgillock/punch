/*
  ==============================================================================

    dbAnnoComponent.cpp
    Created: 24 Nov 2022 9:32:50am
    Author:  bgill

  ==============================================================================
*/

#include "../punch.h"

namespace punch {

    //==============================================================================

    dbAnnoComponent::dbAnnoComponent(float minAmp, float maxAmp, float incAmp, int marginTop, int marginBottom, float annoWidth, juce::Justification style, bool showSignalClipped) :
        _style(style)
    {
        _minAmp = minAmp;
        _maxAmp = maxAmp;
        _incAmp = incAmp;
        _marginTop = marginTop;
        _marginBottom = marginBottom;
        _annoWidth = annoWidth;
        _minY = 0; // init in resized
        _maxY = 0; // init in resized
        _showSignalClipped = showSignalClipped;
    };


    void dbAnnoComponent::paint(juce::Graphics& g)
    {

        // g.setColour(Colours::red);
        // g.drawRect(0,0,getBounds().getWidth(),getBounds().getHeight(), 1.0);

        int width = getBounds().getWidth();
        int textWidth = width;
        int textHeight = 9;

        g.setColour(juce::Colours::white);        
        g.setFont(juce::Font("Lucinda Sans Typewriter", "Regular", 11.0f));
        if (_showSignalClipped) g.drawText("clip", 0, (int)(_maxY - (textHeight / 2.0f) - 12.0f), textWidth, textHeight, juce::Justification::centredLeft);
        //g.drawRect(0, (int)(_maxY - (textHeight / 2.0f) - 12.0f), textWidth, textHeight);

        juce::StringPairArray dbAnnoPos = get_db_pairs(_minAmp, _maxAmp, _incAmp);

        auto font = g.getCurrentFont();
        for (auto& key : dbAnnoPos.getAllKeys())
        {
            if (dbAnnoPos[key] != "")
            {
                g.setColour(juce::Colours::white);
                auto annoText = dbAnnoPos[key];
                if (_style == juce::Justification::left)
                {
                    annoText = annoText.substring(1, annoText.length()) + annoText.substring(0, 1);
                }
                g.drawText(annoText, 0, (int)(key.getFloatValue() - (textHeight / 2.0f)), textWidth, textHeight, _style);
                int strWidth = font.getStringWidth(annoText);
                float tickHeight = 1.0f;
                if (annoText == "0") tickHeight = 2.5f;
                if (_style == juce::Justification::left)
                {
                    g.drawRect((float)strWidth, key.getFloatValue() - (tickHeight/2.0f), (float)width - strWidth, tickHeight, 1.0f);
                }
                else // must be right
                {
                    g.drawRect(0.0, key.getFloatValue() - (tickHeight / 2.0f), (float)width - strWidth, tickHeight, 1.0f);
                }
            }
            else
            {
                g.setColour(juce::Colours::grey);
                g.drawRect(0.0, key.getFloatValue(), (float)width, 0.5f, 0.5f);
            }
        }
        if (_showSignalClipped) g.drawText("signal", 0, (int)(_minY + (textHeight / 2.0f) ), textWidth, textHeight, juce::Justification::centredLeft);
        //g.drawRect(0, (int)(_minY + (textHeight / 2.0f)), textWidth, textHeight);

    }

    void dbAnnoComponent::resized()
    {
        _maxY = _marginTop;
        _minY = getBounds().getHeight() - _marginBottom;
    }


    void dbAnnoComponent::addPair(juce::StringPairArray& pairs, juce::String format, float v, float pixel)
    {

        char buffer[50];
        juce::String annoString = "";
        if (format != "")
        {
            sprintf(buffer, format.getCharPointer(), (float)v);
            annoString = buffer;
        }
        sprintf(buffer, "%f", pixel);
        juce::String pixelString = buffer;
        pairs.set(pixelString, annoString);
    };

    float dbAnnoComponent::getYFromDb(double db)
    {
        if (db <= _minAmp) return (float)_minY;
        if (db >= _maxAmp) return (float)_maxY;
        if (_maxAmp != _minAmp) return (float)_minY + (((float)db - _minAmp) * ((float)(_maxY - _minY) / (_maxAmp - _minAmp)));
        return (float)_minY;
    }

    juce::StringPairArray dbAnnoComponent::get_db_pairs(float minAmp, float maxAmp, float incAmp)
    {
        juce::StringPairArray pairs;

        for (int v = (int)minAmp; v <= (int)maxAmp; v++)
        {
            if (v % (int)incAmp == 0)
            {
                if (v == 0) {
                    addPair(pairs, "%1.0f", (float)v, getYFromDb(v));
                }
                else
                {
                    addPair(pairs, "%+2.0f", (float)v, getYFromDb(v));
                }
            }
            else
            {
                addPair(pairs, "", (float)v, getYFromDb(v));
            }
        }

        return pairs;
    }
}