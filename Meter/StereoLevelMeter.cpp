/*
  ==============================================================================

    StereoLevelMeter.cpp
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#include "../punch.h"

namespace punch {

    //----------------------------------------------------------------------------------------------------------------------
    StereoLevelMeter::StereoLevelMeter(float minAmp, float maxAmp, float incAmp, int marginTop, int marginBottom, float leftAnnoWidth, float rightAnnoWidth) :
        leftLevelMeter(marginTop, marginBottom, minAmp, maxAmp, incAmp),
        rightLevelMeter(marginTop, marginBottom, minAmp, maxAmp, incAmp),
        leftAnno(minAmp, maxAmp, incAmp, marginTop, marginBottom, leftAnnoWidth, juce::Justification::left),
        rightAnno(minAmp, maxAmp, incAmp, marginTop, marginBottom, rightAnnoWidth, juce::Justification::right)
    {
        _leftAnnoWidth = leftAnnoWidth;
        _rightAnnoWidth = rightAnnoWidth;
        //startTimerHz(100);
        addAndMakeVisible(leftLevelMeter);
        addAndMakeVisible(rightLevelMeter);
        if (leftAnnoWidth > 0.0) addAndMakeVisible(leftAnno);
        if (rightAnnoWidth > 0.0) addAndMakeVisible(rightAnno);
        _isMono = false;
    };

    bool StereoLevelMeter::canSetRange()
    {
        return leftLevelMeter.canSetRange() || rightLevelMeter.canSetRange();
    };

    void StereoLevelMeter::timerCallback()
    {
        repaint();
    };

    int StereoLevelMeter::getNChannels()
    {
        return _isMono ? 1 : 2;
    }
    void StereoLevelMeter::resized()
    {
        auto r = getLocalBounds();

        auto la = _leftAnnoWidth > 1.0 ? r.removeFromLeft((int)_leftAnnoWidth) : r.removeFromLeft((int)(r.getWidth() * _leftAnnoWidth));
        auto ra = _rightAnnoWidth > 1.0 ? r.removeFromRight((int)_rightAnnoWidth) : r.removeFromRight((int)(r.getWidth() * _rightAnnoWidth));

        leftLevelMeter.setBounds(r.removeFromLeft(leftLevelMeter.getActualWidth()));
        if (!_isMono)
        {
            rightLevelMeter.setVisible(true);
            rightLevelMeter.setBounds(r.removeFromRight(rightLevelMeter.getActualWidth()));
        }
        else
        {
            rightLevelMeter.setVisible(false);
        }
        
        leftLevelMeter.resized();
        rightLevelMeter.resized();

        if (_leftAnnoWidth > 0.0)
        {
            la.setHeight(leftLevelMeter.getActualHeight());
            leftAnno.setBounds(la);
        }
        if (_rightAnnoWidth > 0.0)
        {
            ra.setHeight(rightLevelMeter.getActualHeight());
            rightAnno.setBounds(ra);
        }
    };

    void StereoLevelMeter::setHeight(int height)
    {
        leftLevelMeter.setHeight(height);
        rightLevelMeter.setHeight(height);
    }

    int StereoLevelMeter::getActualHeight()
    {
        return juce::jmax(leftLevelMeter.getActualHeight(), rightLevelMeter.getActualHeight());
    }
    int StereoLevelMeter::getActualWidth()
    {
        return _leftAnnoWidth + leftLevelMeter.getActualWidth() + (_isMono ? 0 : rightLevelMeter.getActualWidth()) + _rightAnnoWidth;
    }
    void StereoLevelMeter::clearClipped()
    {
        leftLevelMeter.clearClipped();
        rightLevelMeter.clearClipped();
    };
    void StereoLevelMeter::setRange(juce::Range<double> r)
    {
        if (leftLevelMeter.canSetRange())
        {
            leftLevelMeter.setOrangeLevel((float)r.getStart());
            leftLevelMeter.setRedLevel((float)r.getEnd());
            leftLevelMeter.setHeight(getBounds().getHeight());
        }
        if (rightLevelMeter.canSetRange())
        {
            rightLevelMeter.setOrangeLevel((float)r.getStart());
            rightLevelMeter.setRedLevel((float)r.getEnd());
            rightLevelMeter.setHeight(getBounds().getHeight());

        }
    }
    void StereoLevelMeter::capture(juce::AudioBuffer<float> amps)
    {
        if (amps.getNumChannels() == 0) return;

        leftLevelMeter.capture(amps, 0); 
        _isMono = amps.getNumChannels() == 1;

        if (!_isMono) rightLevelMeter.capture(amps, 1);
    };
    void StereoLevelMeter::capture(juce::AudioBuffer<double> amps)
    {
        if (amps.getNumChannels() == 0) return;

        leftLevelMeter.capture(amps, 0);
        _isMono = amps.getNumChannels() == 1;

        if (!_isMono) rightLevelMeter.capture(amps, 1);
    };
    
    //-----------------------------------------------------------------------------------------------------------
    void LevelMeter::paint(juce::Graphics& g)
    {

        //g.setColour(juce::Colours::red);
        //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);

        auto levels = maxAmp.getLevels();
        _nLights = maxAmp.getNLevels();

        int y = _mTop;
        int centerx = getBounds().getWidth() / 2;

        int tx = centerx - _lightwidth / 2;

        drawClipped(g, tx, _mTop - _spacing - _clippedheight, _lightwidth, _clippedheight, maxAmp.clipped());

        for (int l = _nLights - 1; l >= 0; l--)
        {
            drawLight(g, tx, y, _lightwidth, _lightheight, levels, l);
            y += _lightheight + _spacing;
        }

        drawSignal(g, tx, _mTop + (_nLights * (_lightheight + _spacing)), _lightwidth, _signalheight, maxAmp.signal());

        maxAmp.clear();
        return;
    };

    void LevelMeter::capture(juce::AudioBuffer<float> amps, int channel)
    {
        maxAmp.capture(amps, channel);
    }
    void LevelMeter::capture(juce::AudioBuffer<double> amps, int channel)
    {
        maxAmp.capture(amps, channel);
    }

    //---------------------------------------------------------------------------------------------------------------------
    UADLevelMeter::UADLevelMeter(int marginTop, int marginBottom, float minAmp, float maxAmp, float incAmp) :
        LevelMeter(marginTop, marginBottom, minAmp, maxAmp, incAmp)
    {
        _orangeLevel = -18.0;
        _redLevel = -3.0;
        _peakholdTimes = 10; // number of times to leave peak 
        _lightheight = 16;
        _lightwidth = 16;
        _spacing = 2;
        _clippedheight = 16;
        _signalheight = 16;
        _lightImages = juce::ImageFileFormat::loadFrom(LevelMeterBinaryData::APILights_png, LevelMeterBinaryData::APILights_pngSize);
    };

    void UADLevelMeter::setHeight(int height)
    {
        int topy = _mTop;
        int bottomy = height - _mBottom;

        _nLights = (int)((float)(bottomy - topy + 1) / (_lightheight + _spacing));
        maxAmp.setNLevels(_nLights);
        auto mindb = (float)maxAmp.getMinAmp();
        auto maxdb = (float)maxAmp.getMaxAmp();
        float dbPerLight = ((maxdb - mindb) / (float)_nLights);

        if (_lightImageIndexes != nullptr) delete _lightImageIndexes;
        _lightImageIndexes = new int[_nLights];

        for (int l = 0; l < _nLights; l++)
        {
            int thisImage = 2; // default to off, on = +3
            float thisdb = mindb + ((float)l * dbPerLight);
            if (thisdb > _redLevel - (dbPerLight * 0.5)) thisImage = 0;
            else if (thisdb > _orangeLevel - (dbPerLight * 0.5)) thisImage = 1;
            _lightImageIndexes[l] = thisImage;
        }
    }
    void UADLevelMeter::resized()
    {
        auto area = getBounds();
        setHeight(area.getHeight());
    }

    int UADLevelMeter::getActualHeight()
    {
        return _mTop + (_nLights * (_lightheight + _spacing)) + _mBottom;
    }
    int UADLevelMeter::getActualWidth()
    {
        return _meterWidth;
    }
    void UADLevelMeter::setRedLevel(float level)
    {
        _redLevel = level;
    }
    void UADLevelMeter::setOrangeLevel(float level)
    {
        _orangeLevel = level;
    }
    void UADLevelMeter::clearClipped()
    {
        maxAmp.setClipped(false);
    }
    void UADLevelMeter::drawLight(juce::Graphics& g, int x, int y, int width, int height, float* levels, int l)
    {
        int index = _lightImageIndexes[l];
        if (levels[l] == 1.0) index += 3;

        g.drawImage(_lightImages, x, y, width, height, 0, index * (height + _spacing), width, height);
    }

    void UADLevelMeter::drawClipped(juce::Graphics& g, int x, int y, int width, int height, bool clipped)
    {
        int index = 0;
        if (clipped) index = 3;
        g.drawImage(_lightImages, x, y, width, height, 0, index * (height + _spacing), width, height);
    }

    void UADLevelMeter::drawSignal(juce::Graphics& g, int x, int y, int width, int height, bool signal)
    {
        int index = 2;
        if (signal) index = 5;
        g.drawImage(_lightImages, x, y, width, height, 0, index * (height + _spacing), width, height);
    }

    //---------------------------------------------------------------------------------------------------
    DrawnLEDLevelMeter::DrawnLEDLevelMeter(int marginTop, int marginBottom, float minAmp, float maxAmp, float incAmp) :
        LevelMeter(marginTop, marginBottom, minAmp, maxAmp, incAmp)
    {
        _peakholdTimes = 10; // number of times to leave peak 
        _lightheight = 9;
        _lightwidth = 17;
        _spacing = 1;
        _clippedheight = 10;
        _signalheight = 10;
        _nLights = 10;
    };

    void DrawnLEDLevelMeter::setHeight(int height)
    {

        int topy = _mTop;
        int bottomy = height - _mBottom;

        _nLights = (int)((float)(bottomy - topy + 1) / (_lightheight + _spacing));
        maxAmp.setNLevels(_nLights);

        if (_lightColors != nullptr) delete _lightColors;
        _lightColors = new juce::Colour[_nLights];
        int orangelight = (int)((float)_nLights * 0.66f);
        for (int l = 0; l < _nLights; l++)
        {
            juce::Colour thiscolor = juce::Colour::fromRGB(0, 0, 0);
            if (l == _nLights - 1) thiscolor = juce::Colour::fromRGB(255, 0, 0);
            else if (l >= orangelight) thiscolor = juce::Colours::orange;
            else thiscolor = juce::Colour::fromRGB(0, 255, 0);
            _lightColors[l] = thiscolor;
        }
    }

    void DrawnLEDLevelMeter::resized()
    {
        auto area = getBounds();
        setHeight(area.getHeight());
    }

    int DrawnLEDLevelMeter::getActualHeight()
    {
        return _mTop + (_nLights * (_lightheight + _spacing)) + _mBottom;
    }
    int DrawnLEDLevelMeter::getActualWidth()
    {
        return _meterWidth;
    }
    void DrawnLEDLevelMeter::clearClipped()
    {
        maxAmp.setClipped(false);
    }

    void DrawnLEDLevelMeter::drawLight(juce::Graphics& g, int x, int y, int width, int height, float* levels, int l)
    {
        g.setColour(juce::Colours::black); // off color
        if (levels[l] == 1.0) g.setColour(_lightColors[l]);
        g.fillRect(x, y, width, height);

        g.setColour(juce::Colours::grey); // border color
        g.drawRect(x, y, width, height, (int)_lightborder);
    }

    void DrawnLEDLevelMeter::drawClipped(juce::Graphics& g, int x, int y, int width, int height, bool clipped)
    {
        g.setColour(juce::Colours::black); // off color
        if (clipped) g.setColour(juce::Colours::red);
        g.fillRect(x, y, width, height);

        g.setColour(juce::Colours::grey); // border color
        g.drawRect(x, y, width, height, (int)_lightborder);
    }

    void DrawnLEDLevelMeter::drawSignal(juce::Graphics& g, int x, int y, int width, int height, bool signal)
    {
        g.setColour(juce::Colours::black); // off color
        if (signal) g.setColour(juce::Colour::fromRGB(0, 255, 0));
        g.fillRect(x, y, width, height);

        g.setColour(juce::Colours::grey); // border color
        g.drawRect(x, y, width, height, (int)_lightborder);
    }

    
#include <cstring>

    namespace LevelMeterBinaryData
    {
        //================== APILights.png ==================
        static const unsigned char temp_binary_data_2[] =
            "\x89PNG\r\n"
            "\x1a\n"
            "\0\0\0\rIHDR\0\0\0\x10\0\0\0j\x08\x02\0\0\0:(F\x91\0\0\0\tpHYs\0\0\x0e\xc4\0\0\x0e\xc4\x01\x95+\x0e\x1b\0\0;\xa7iTXtXML:com.adobe.xmp\0\0\0\0\0<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.6-c067 79.157747, 2015/03/30-23:40:42        \">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "      <rdf:Description rdf:about=\"\"\n"
            "            xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\"\n"
            "            xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
            "            xmlns:photoshop=\"http://ns.adobe.com/photoshop/1.0/\"\n"
            "            xmlns:xmpMM=\"http://ns.adobe.com/xap/1.0/mm/\"\n"
            "            xmlns:stEvt=\"http://ns.adobe.com/xap/1.0/sType/ResourceEvent#\"\n"
            "            xmlns:tiff=\"http://ns.adobe.com/tiff/1.0/\"\n"
            "            xmlns:exif=\"http://ns.adobe.com/exif/1.0/\">\n"
            "         <xmp:CreatorTool>Adobe Photoshop CC 2015 (Windows)</xmp:CreatorTool>\n"
            "         <xmp:CreateDate>2022-12-06T17:20:23-06:00</xmp:CreateDate>\n"
            "         <xmp:ModifyDate>2022-12-08T06:02:16-06:00</xmp:ModifyDate>\n"
            "         <xmp:MetadataDate>2022-12-08T06:02:16-06:00</xmp:MetadataDate>\n"
            "         <dc:format>image/png</dc:format>\n"
            "         <photoshop:ColorMode>3</photoshop:ColorMode>\n"
            "         <xmpMM:InstanceID>xmp.iid:5bd32bae-597d-e740-a775-1e0839763978</xmpMM:InstanceID>\n"
            "         <xmpMM:DocumentID>adobe:docid:photoshop:33c95b82-75c1-11ed-b307-a76dc0d0d01e</xmpMM:DocumentID>\n"
            "         <xmpMM:OriginalDocumentID>xmp.did:0399f88c-3e8a-9c48-89fb-a5c737ff9db8</xmpMM:OriginalDocumentID>\n"
            "         <xmpMM:History>\n"
            "            <rdf:Seq>\n"
            "               <rdf:li rdf:parseType=\"Resource\">\n"
            "                  <stEvt:action>created</stEvt:action>\n"
            "                  <stEvt:instanceID>xmp.iid:0399f88c-3e8a-9c48-89fb-a5c737ff9db8</stEvt:instanceID>\n"
            "                  <stEvt:when>2022-12-06T17:20:23-06:00</stEvt:when>\n"
            "                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
            "               </rdf:li>\n"
            "               <rdf:li rdf:parseType=\"Resource\">\n"
            "                  <stEvt:action>saved</stEvt:action>\n"
            "                  <stEvt:instanceID>xmp.iid:e079a042-256b-2842-ad4d-d822498328f6</stEvt:instanceID>\n"
            "                  <stEvt:when>2022-12-06T17:54:13-06:00</stEvt:when>\n"
            "                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
            "                  <stEvt:changed>/</stEvt:changed>\n"
            "               </rdf:li>\n"
            "               <rdf:li rdf:parseType=\"Resource\">\n"
            "                  <stEvt:action>saved</stEvt:action>\n"
            "                  <stEvt:instanceID>xmp.iid:5bd32bae-597d-e740-a775-1e0839763978</stEvt:instanceID>\n"
            "                  <stEvt:when>2022-12-08T06:02:16-06:00</stEvt:when>\n"
            "                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
            "                  <stEvt:changed>/</stEvt:changed>\n"
            "               </rdf:li>\n"
            "            </rdf:Seq>\n"
            "         </xmpMM:History>\n"
            "         <tiff:Orientation>1</tiff:Orientation>\n"
            "         <tiff:XResolution>960000/10000</tiff:XResolution>\n"
            "         <tiff:YResolution>960000/10000</tiff:YResolution>\n"
            "         <tiff:ResolutionUnit>2</tiff:ResolutionUnit>\n"
            "         <exif:ColorSpace>65535</exif:ColorSpace>\n"
            "         <exif:PixelXDimension>16</exif:PixelXDimension>\n"
            "         <exif:PixelYDimension>106</exif:PixelYDimension>\n"
            "      </rdf:Description>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                                                                                                    \n"
            "                            \n"
            "<?xpacket end=\"w\"?>%\xea""F\x11\0\0\0 cHRM\0\0z%\0\0\x80\x83\0\0\xf9\xff\0\0\x80\xe9\0\0u0\0\0\xea`\0\0:\x98\0\0\x17o\x92_\xc5""F\0\0\x14ZIDATx\x01\0J\x14\xb5\xeb\x01""5=B\x01\x01\x02\0\0\xff\xfe\xff\xff\xff\xff\xff\x02\x01\x01\0\xff\0\0\x02\x01\x01"
            "\0\0\xff\0\0\xff\xff\xff\x02\x01\0\xff\0\x02\0\xff\xff\xff\x01\0\0\0\xff\x02\x01\x02\x01\xff\0\xff\xfd\xfe\xfe\0\0\0\x03\x02\x02\xff\xff\xff\x01\x02\x01\0\xff\xff\xff\xff\xff\xfe\xfe\xfe\xff\xff\xff\xff\0\0\xff\xff\xfe\xff\xff\xff\x02\x01\x01\xff\xff"
            "\0\x04\xff\xfe\0\xff\0\0\x02\x01\x01\x01\x02\x01\xfe\xff\0\x01\x01\x01\xf6\xf6\xf4\xf6\xf9\xf8\x02\x03\x03\t\x06\x07\x08\x08\n"
            "\0\xff\0\x01\x01\0\x01\x01\0\0\xff\xff\xff\xff\xff\x04\xff\xff\0\0\x01\0\0\x01\x01\0\0\0\xf8\xf6\xf5\xe9\xe6\xe3\x10\0\xff\x16\xfc\xfc\xfb\xf5\xf6\xe7\xfe\xfd\xfd\x18\x19\x1c\x1a\x1d\x04\x04\x05\xfd\xfd\xfd\0\0\0\x03\x01\x02\x04\x01\x01\x01\x01\x02\0"
            "\xff\xff\xff\xf9\xf7\xf7\xe7\xe2\xdeG  7\r\r\n"
            "\xf5\xf4\xfd\xf4\xf4\xf1\xf6\xf7\xbf\xe9\xe9\xbe\xe9\xe9,03\x02\x03\x04\x01\0\0\x02\x01\0\x02\0\x01\0\xff\xff\0\xfd\xfc\xfc\xe4\xe5\xe2G  N\x19\x19\xfb\xe3\xe3\xef\xef\xef\xf5\x01\0\x0b\r\r^$%@\xff\xff\xda\xd7\xd3\x03\x03\x03\xfe\xff\xff\xff\0\x01\x02"
            "\xfa\xfb\xf9\xfe\xfe\xff\xf9\xf9\xf9\x11\xfe\xfd""7\r\r\xe5\xd8\xd7\x01\x04\x03\x07\x07\x07\x04\x01\x01\xf8\xfd\xfc\xef\0\xffG\x1c\x1a\xf4\xef\xef\x0e\x0e\r\x01\x01\x01\xff\0\xff\x02\xfc\xfd\xfe\x01\x01\x01\xf7\xfb\xf9\x14\xfd\xfe\n"
            "\xf5\xf5\xfd\0\0\x04\x05\x04\x03\x01\x01\x03\x01\x02\x05\x02\x01\xf7\xfd\xfc\x10\x08\t\x0f\0\0\x0c\x0f\x0c\x02\x01\x01\0\0\0\x02\x02\x01\x01\0\x01\xff\x03\x03\x02\xfb\xf6\xf5\xfd\xf4\xf3\x03\x06\x05\0\xff\xff\0\0\0\0\0\0\0\0\0\x01\0\0\xff\0\0\xfe\0\0"
            "\0\xff\x01\xfe\xff\xff\x01\0\0\x02\x02\x01\x01\x01\0\0\x0b\x08\n"
            "\xe7\xfc\xfc\xf1\xf6\xf7\x07\x02\x04\xfa\xfe\xff\xfd\xff\xfe\xfd\xff\xfe\xfb\xfe\xff\t\x03\x03\xea\xf6\xf6\xf3\0\0\xf9\xf7\xf9\x01\x01\x01\xff\xff\0\x02\x03\x04\x04\0\x01\x01\x05\x05\x06\xfe\x1a\x1b\xbe\xea\xea\x12\0\x01\t\x03\x04\xfb\xfe\xff\xfc\xfe"
            "\xff\n"
            "\x02\x03\x0b\x01\x02\xa9\xdd\xdd\x15\x1e\x1f\xf2\xf1\xf2\0\0\x01\xfe\xff\xff\x02\x03\x02\x02\x02\0\0\x02\x01\x02\x1c\x1a\x1d\xc2\0\0\x9f\xda\xd9\xf9\xf6\xf5\x11\x01\x02\x0f\x02\x02\xf1\xf7\xf6\x8d\xd0\xd0\xd2\x01\x01><@\xfb\xfb\xfb\xff\xfe\xfe\x01\x01"
            "\x01\x01""5>C\x02\x01\x01\xff\0\0\xfe\xfe\xfe\x01\x01\0\xd8\xd1\xce\xf3\xf1\xf0\x11\0\0\xfd\0\0\xf2\0\0\x18!\":8;\xe2\xe4\xe6\0\0\0\xff\xfe\xff\x03\x03\x02\x01""6>C\0\0\0\x02\x02\x01\xfd\xfd\xff\x01\x01\x01\x02\x02\x01\x10\x12\x10\t\x0b\n"
            "\0\xff\0\xfb\xf9\xfa\xeb\xec\xec\xfe\xfd\xfe\0\xff\0\xff\0\xff\x01\x01\x01\0\xff\0\x04\xfe\0\0\x02\x01\0\0\0\0\x01\x01\x01\xff\xff\xff\xfd\xff\xff\xee\xec\xef\xff\xec\xee\xff\0\0\x01\x01\0\0\xff\0\0\x01\0\x01\x02\x01\0\xfe\0\0\x01\x01\x01\0\0\x02\x01"
            "\xff\0\x02\0\x01\xfd\xfe\xff\xff\0\xff\0\x01\0\x02\0\x01\0\x01\xff\0\0\0\x04\x02\x02\x02\0\x01\xff\xff\xff\x02\x01\x02\0\xfe\0\xff\0\0\0\xff\xff\xff\x01\0\x02\x02\x03\x01\xfe\0\xff\0\0\0\x02\x01\0\x01\x01\x01\xff\xff\xff\xff\xff\0\0\0\0\0\0\0\xfc\xfd"
            "\xfe\0\0\0\0\0\0\xfe\xff\xfe\0\xff\0\xfe\xff\xff\0\0\0\x04\xfe\xfe\xfe\0\0\0\x02\x01\x01\0\x01\x02\xff\xfe\xfe\0\xff\xff\0\x01\x01\x02\x01\0\xfd\xfe\xfe\xff\0\0\x03\x02\x01\xff\xff\xff\x02\x02\x01\0\xff\0\0\x01\x02\xff\0\0\x02\xfe\xfe\xff\x02\x01\x02"
            "\xff\0\x01\0\xff\xff\0\xff\0\x01\x02\x01\x02\0\0\xff\0\x01\xff\xff\xff\x05\x03\x03\x01\x01\x02\xff\xfe\xfe\x01\0\x01\xff\0\0\x01\0\0\0\xff\0\x02\x04\x03\x04\0\0\0\0\xff\xff\0\0\0\0\x01\xff\xff\xff\0\xff\x01\x01\0\0\0\x02\x02\x01\xff\0\xfe\xfe\xfe\xfe"
            "\x02\x03\x03\0\x01\0\xff\0\xff\xff\0\0\x03\x01\x01\x04\xff\0\xff\xfe\xfe\xff\x03\x03\x02\xff\xfe\xff\0\0\x01\0\xff\xff\xf5\xf4\xf5\xf5\xf3\xfa\x03\x03\x03\x0b\x0b\x06\x08\x0b\x08\xff\0\xff\0\xff\0\0\xff\0\x01\x01\x01\0\x01\0\x03\x1a\x1f!\x01\0\xff\0\0"
            "\0\xff\0\0\xf8\xf5\xf6\xe0\xdb\xdf\x13\x0e\xf7 \x1c\xea\n"
            "\x04\xe0\xe9\xe7\xe8\xe9\xe8\x01\x0e\x0f\n"
            "\x02\x02\x02\0\0\x01\x01\x01\xff\xfd\xfe\xfe\x04\x01\x01\x01\0\x01\0\xff\xff\0\xfa\xf8\xf8\xe0\xda\xd8""A?!1-\xfa\t\x06\xf4\xfa\xfa\xfd\xf0\xf2\xfc\xc9\xcd\xda\xc8\xc9\xe3""167\x01\x05\x03\0\0\x01\x01\0\0\x03\x1c!#\0\0\0\x01\0\0\xdf\xd9\xdc?<\x1cQG\xfc"
            "\xeb\xeb\xd3\xfa\xf9\xf6\xff\xfe\0\r\x0b\0""80\0\xee\xf1\xfc\xd5\xd6\xf3\x19\x1b\x1b\xff\xff\xff\xff\0\0\x02\xfc\xfd\xfd\xff\0\0\xf7\xf6\xf8\x1f\x1d\x07""0,\xf9\xdb\xde\xd2\x03\x02\0\x06\x06\x01\x03\x04\x01\xf8\xf8\x01\xee\xf2\0:7\0\xf7\xf4\xed\r\x0c"
            "\x0f\xfe\xff\xff\x01\x01\x01\x02\x03\x02\x02\x02\x01\x01\xf4\xf2\xf7\x19\x16\xf3\n"
            "\x07\xe6\0\0\0\x04\x04\x01\x03\x02\0\x04\x02\0\x04\x04\0\xf7\xf7\0\x15\x12\0\x1b\x18\0\t\t\r\0\0\0\xff\xfe\xfe\x04\xfd\xfe\xfe\x03\x03\x03\x02\x03\x04\xf6\xf5\xee\xfb\xfa\xfe\x03\x02\0\0\xff\0\0\0\0\0\0\0\0\0\0\0\x01\0\xfe\xfe\0\xfd\xfe\0\0\xfe\xff\0"
            "\0\0\x02\x02\x02\x02\xfb\xfb\xfb\x01\x01\x01\x0c\r\x08\xdf\xe2\x02\xef\xf1\xff\x07\x07\0\xfb\xfc\0\xfc\xfd\0\xfc\xfd\0\xfc\xfc\0\t\t\0\xe7\xea\0\xe5\xe8\0\xfa\xfc\xf7\x03\x02\x02\x02\x01\x02\x02\x07\x06\x06\xfd\xfd\xfd\x05\x07\x04\xf6\xf9\"\xc9\xce\0"
            "\x13\x0f\0\t\t\xff\xfc\xfc\xff\xfc\xfd\xff\n"
            "\n"
            "\xff\t\x07\0\xbd\xc1\0\x11\x14\"\xf4\xf6\xf3\xff\xff\xfe\xfb\xfd\xfc\x03\x1c!$\xff\xff\xff\x02\x02\x03\x11\x13\r\xcc\xcb\xe8\xee\xf0\xfc\x16\x15\0\x11\x0f\0\x05\x04\0\xe9\xee\0\xad\xb3\0\xd5\xd8\x01LRQ\xee\xee\xee\xfe\xfe\xff\x02\x01\x01\x01""5@E\xff"
            "\xfe\xfe\x03\x02\x02\0\0\0\xfe\xfd\xfd\xd8\xd2\xd1\xf3\xf1\xed \x1c\0\xfc\xfc\0\xe4\xe8\0\x14\x16%@E<\xe2\xe5\xe5\xfe\xfe\xfd\x01\0\x01\x02\x02\x01\x01""6@E\x01\x01\0\x02\x01\x02\xfb\xfb\xfb\x02\x02\x02\x02\x02\x02\x10\x0e\x11\t\t\r\xff\xfe\0\xf9\xfb"
            "\xf5\xf1\xf1\xee\xfe\xff\0\xfd\xfe\xfd\x01\x01\x01\xff\xff\0\x03\x01\0\x04\x01\0\0\xff\xfe\xff\xfc\xff\xfc\x02\x03\x03\x01\x01\x01\xff\xff\xff\xed\xf0\xec\0\xf0\xed\xff\0\0\x02\0\0\0\0\0\xfe\xff\xff\0\0\0\x01\x01\x01\x01\x01\0\xfe\0\0\x02\x02\x01\x01"
            "\xff\xff\xff\0\0\0\x03\x02\x02\xff\xfe\xfe\xfd\xfe\xfe\0\0\x01\x02\x01\x01\x03\x01\x01\xff\xff\xff\xff\xff\xff\x05\x03\x04\x02\x01\x02\xff\xff\xff\0\xff\xff\0\xff\xff\x02\xfe\xff\xff\x01\0\x01\x02\x02\x02\xfe\xfd\xfe\xfe\xfe\xfe\x02\x02\x02\x02\x01\x01"
            "\x01\x01\x02\xff\xff\xff\0\0\0\x02\x03\x03\0\0\xff\0\0\xff\x02\x01\x01\x02\x02\x02\0\x01\x01\x02\0\xff\0\x01\x02\x01\x01\x01\x01\xff\0\0\x03\x03\x03\x01\xff\xff\xff\xff\xff\0\xff\xff\xfe\xff\xff\x03\x02\x02\xff\xff\xff\xfb\xfd\xfd\0\0\0\x01\x01\x01\xff"
            "\xfe\xfe\xff\xff\xff\x02\x02\x02\x01\0\0\0\xfd\xfd\xfd\x01\x01\0\x02\x01\x01\xff\0\0\xff\0\0\xfe\xff\xfe\x02\x01\x01\xff\xff\xff\0\xff\xff\x03\x02\x02\x01\x02\x02\xfe\xff\xff\xfc\xfe\xfe\xfe\xfe\xfe\x04\xfe\0\0\0\0\x01\x03\x02\x02\x01\0\0\xff\xff\0\xfe"
            "\xff\xff\x03\x02\x01\x01\x01\x02\xff\xff\xff\0\0\0\x01\0\0\x01\x01\x01\xfd\xfe\xfe\x02\x01\x01\x02\x02\x02\x02\x01\x01\x04\xff\xfe\xfe\x01\0\0\x02\x03\x03\xff\xff\0\xff\xfe\xfd\x03\x02\x02\xf7\xf3\xf2\xfa\xf6\xf7\x03\x03\x04\x08\x0b\n"
            "\x06\n"
            "\n"
            "\0\0\0\x02\x01\x01\xfe\xff\xff\x02\x01\0\x02\x01\x01\x04\0\xff\xff\0\x01\x01\xff\xff\xff\xfe\xff\xff\xf9\xf7\xf7\xe9\xe1\xdf\x08\x10\t\xff\x0f\x03\xf2\xf7\xf5\xf9\xe0\xf4\x13\x03\x11\x18\"!\x02\x04\x04\x02\0\x01\0\0\0\xfb\xfe\xfe\x04\x01\x01\x02\0\0\0"
            "\xff\0\0\xf9\xf7\xf6\xe8\xdf\xda""4K>\x12""0\x1c\xef\x02\xf5\xf0\xf9\xf4\xf5\xf1\xf4\xe3\xc3\xd7\xd4\xc7\xc8,67\x02\x04\x05\xfe\xff\xff\0\xff\xff\x03\x1a\x1e!\xfe\0\0\x01\xff\xff\xe4\xdb\xd9""4K<\x1fG-\xd0\xe5\xd7\xf5\xf8\xf5\x03\xfe\x01\t\t\t\x19""3"
            " \xed\xef\xef\xeb\xd2\xdf\x18\x1d\x1f\xfe\xfe\xfe\x04\x02\x02\x02\0\0\x01\x02\x01\0\xf7\xf5\xf5\t\x11\n"
            "\x12""0\x1c\xcb\xd9\xcf\x05\x05\x05\x0b\n"
            "\n"
            "\x03\x06\x03\xfb\xfc\xfb\xfd\xf3\xfb\"C,\xf1\xf2\xf0\x10\r\x0c\x02\x01\x01\0\x01\x01\x02\x02\x03\x01\x02\x02\x03\xfa\xf6\xf5\xff\x0f\x05\xef\x02\xf6\0\0\0\x07\x06\x06\0\x03\x03\x01\x04\x04\x04\x05\x03\xfc\xf9\xfa\n"
            "\x10\x0c\0\n"
            "\0\x0c\x08\n"
            "\xfe\xff\xfe\xfd\xfe\xfe\x02\xf8\xfa\xfa\xff\xff\xff\x03\x03\x05\xf3\xf8\xf4\xf1\xf8\xf3\x07\x04\x05\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\x01\x01\0\xfe\0\0\xfe\0\0\0\xff\0\0\x01\x01\0\0\x02\x01\xff\xff\x01\x01\0\x08\x0b\n"
            "\xf8\xea\xf3\xf5\xf2\xf4\x04\x06\x05\xfc\xfa\xfc\xff\xfc\xfc\xff\xfc\xfc\xfd\xfb\xfd\x04\x08\x05\xf5\xea\xf1\0\xf8\0\xf8\xfa\xfa\x02\0\x01\x01\x02\x02\x02\x04\x05\x05\xfe\xff\xff\x06\n"
            "\n"
            "\x15\x04\x12\xe2\xc3\xd8\x02\r\x07\x05\x06\x05\xfc\xf9\xfc\xfd\xfa\xfd\x04\x06\x05\x02\x06\x04\xd1\xaf\xc6\x1b\x18\x1c\xef\xf4\xf3\xfe\0\xff\xff\xfe\xff\x02\x04\x03\x03\x02\x01\x01\xff\x01\0\x19#\"\xf8\xc9\xe7\xd1\xa6\xc3\xf6\xf5\xf6\x05\r\x07\x03\x0b"
            "\x06\xf6\xee\xf3\xc1\x94\xb2\xfc\xda\xf0<DF\xfb\xfb\xfc\xff\xff\xff\x02\x02\x01\x01""7@E\x01\x02\x02\xfe\xfe\xfe\x03\x03\x02\x01\xfe\xff\xd3\xcf\xcb\xf3\xf0\xef\0\x0c\0\0\xfd\0\0\xf7\0\x1e\x1b 7@?\xe4\xe8\xea\x01\0\0\xfb\xfc\xfc\x03\x01\0\x01""7@E\0\0"
            "\0\x01\x01\x01\x01\x01\x01\xff\xff\xff\xff\0\0\x15\x11\x10\x0c\x07\t\x01\x01\0\xf7\xfa\xfb\xea\xed\xed\xff\0\0\xff\xff\xff\xff\xff\xff\x02\x03\x03\xfd\xfd\xfe\x04\xff\0\0\x01\0\0\x01\x01\x01\xff\xff\xff\xff\0\0\0\xff\xff\xe8\xec\xee\x05\x04\x03\xff\xff"
            "\xff\xff\0\0\x01\x01\0\x01\0\x01\xff\xff\xff\0\x01\x01\xfe\xfe\xfe\x01\x01\0\x02\x01\0\x01\x02\x02\x02\xfe\xfe\xfe\xff\xff\xfe\0\xff\xff\xfe\xff\xff\x04\x03\x02\xff\0\0\xfd\xfe\xfe\0\xff\xff\xff\xff\0\xfe\xff\xfe\0\x01\0\x01\0\0\xff\xff\xff\0\0\0\x04"
            "\x01\0\xff\xfe\xfd\xff\0\x01\0\x02\x02\x03\0\xff\xff\0\0\0\x01\0\0\xfd\xfe\xfe\0\0\0\xfe\xfe\xff\x01\x02\0\x01\xff\0\x01\x02\x01\xff\x01\xff\0\xff\xff\xfe\xff\0\x04\0\0\0\0\x01\0\x03\x02\x02\0\0\0\xfc\xff\xff\x02\0\0\xfe\xff\xff\x02\x02\x02\xff\xff\xff"
            "\0\xff\0\x02\x02\x01\x01\0\x01\xff\xff\xfe\xfe\xff\0\x01\x01\x01\x03\x01\x01\x04\xfd\xfd\xfd\x01\x01\x02\xfd\xfe\xfc\0\xff\xff\x03\xff\xff\x06\x01\x01\x03\xff\0\x01\x03\xfe\x01\0\0\xfe\xff\0\xfc\xfd\xff\xfe\x01\0\xfc\0\x02\0\xff\xff\xfd\x01\0\xfd\0\xff"
            "\x04\x01\x02\x01\x01\xff\xff\x02\xfe\xfe\x06\x02\x01\n"
            "\x01\x02\x04\xfe\xfe\x06\x03\x02\x02\xff\0\0\0\0\xfc\xff\xff\xfc\xff\xff\xfc\x01\x01\xf6\0\xfe\xfb\xff\0\xfe\x02\x02\xfb\xfe\xff\x04\0\xfe\0\x03\0\0\x08\x01\x01\r\x01\x01\x04\0\0\x13\0\xff\x03\xfd\xfb\x07\xfe\xfd\x01\x01\x02\xf9\0\0\xfc\x03\x05\xed\0"
            "\x01\x06\0\xff\xf4\x02\x01\xf9\xff\xff\xfb\xff\xff\x04\x01\xff\0\x05\x02\0\x0c\0\x01\r\0\xff\x0b\xfb\xfa\xf8\xf2\xf1""1\xef\xee \xf6\xf6\0\0\0\xdf\t\t\xd2\x14\x15\x1a\x10\x11\xed\x01\x03\xff\xfd\xfe\xfc\x01\0\x05\x01\x02\x04\x04\x01\x01\n"
            "\x01\0\x07\xff\xfe\x0c\xfb\xfb\xf6\xea\xe7\x7f\xe8\xe9\"\xf4\xf4\0\0\0\0\0\0\0\xef\xef\xda\xf0\xf0\x84\x19\x1a\x13\x1a\x1d\xf7\x02\x01\n"
            "\xff\0\xfe\x01\0\x02\x04\x01\0\x05\xff\xff\x0f\xfe\xfd\xf6\xf3\xf1\x87\xe7\xe8\"\xf4\xf4\0\x02\x02\0\x0c\x0c\0\x0c\x0c\0\x02\x02&\xf6\xf6\x83\xe6\xe5\xf7\xf4\xef\x14\x01\0\x04\0\0\x02\0\x01\x02\xfc\xfc\xf9\x03\xff\0\x04\xfd\xfe""4\xef\xee\x1a\xf5\xf5"
            "\0\x02\x02\0\x12\x12\0\r\r\0\r\r\0\x12\x12\0\x02\x02\x1f\xf7\xf7,\xea\xeb\x08\xfe\xfd\x03\x01\0\x03\0\xff\x02\xff\xfd\xff\x03\x01\x01\x08\x01\xff \xf4\xf4\0\0\0\0\n"
            "\n"
            "\0\x05\x05\0\x02\x02\0\x02\x02\0\x05\x05\0\n"
            "\n"
            "\0\0\0\x1f\xf5\xf5\n"
            "\x01\x01\x03\x01\x01\x02\0\0\x02\x02\x01\x01\0\x01\xff\0\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfe\xfe\x01\x03\x03\x01\xff\0\0\0\xff\x02\0\0\0\xfe\0\0\xfc\x02\x02\xdf\x0b\x0b\0\0\0\0\xf6\xf6\0\xfb\xfb\0\xfe\xfe\0\xfe\xfe\0\xfb"
            "\xfb\0\xf6\xf6\0\0\0\xe2\n"
            "\n"
            "\xfb\x03\x04\xff\x02\x01\xfd\xff\x01\x02\0\x04\x04\xfc\0\0\xf9\x02\x04\xd0\x13\x13\xe2\t\t\0\xfe\xfe\0\xee\xee\0\xf3\xf3\0\xf3\xf3\0\xee\xee\0\xfe\xfe\xdd\x02\x02\xdb!\"\xf9\x02\x03\xfb\xfe\xff\xfc\xff\xff\x02\xff\x02\x02\xfd\0\0\xf0\x01\x02\n"
            "\x0f\x11|\x1a\x1b\xd9\n"
            "\n"
            "\0\xfe\xfe\0\xf4\xf4\0\xf4\xf4\0\xfe\xfe\xd5\x04\x04\x89&'\x0c\x14\x16\xea\xfb\xfc\xf9\xfe\xff\xfd\x01\x01\x04\xfd\0\0\xf7\x01\x01\x04\xff\xfe\xec\x01\x03\x15\x1a\x1c\x8d\r\r)\xeb\xea\x1f\xf6\xf6\0\xfe\xfe\xe2\t\t\xdd\"#\x14\x14\x15\xe1\xf3\xf5\xf7\xfc"
            "\xfd\xf7\0\0\xff\x01\x01\x04\xff\0\0\xfa\xfe\xff\xf6\x02\0\0\xfe\0\xe7\x02\x04\x17\x11\x14\xe0\x0e\x0f\xe8\0\0\x02\x03\x03\xfa\x04\x05\x02\x02\x02\xdc\xf2\xf4\xf9\xfc\xfe\xf5\x01\0\xfd\x02\x01\xfd\xff\xff\x04\xfd\0\0\x01\x01\0\xf8\0\x01\xfc\x02\x01\x08"
            "\xfe\xfe\xe5\x01\x01\x03\x01\x04\0\0\xff\x01\xff\x01\x01\xfb\xfc\xfa\xfe\xff\xfa\x01\0\xf9\x04\x02\xfa\xfe\xff\xfc\x01\x01\0\0\0\x04\x01\xff\0\0\x01\x01\xf9\xfe\xff\xfb\0\0\xfa\x01\x01\x05\xfe\x01\x03\0\xff\0\0\xff\x03\0\x01\xfd\xff\xff\xfb\xfe\xff\xff"
            "\x04\x03\xfc\xfe\xff\xfc\0\0\0\xff\xff\0\x01\x01\x04\x02\x04\x03\0\xff\0\x04\x03\x01\xfd\xff\xff\xfc\0\0\xfd\x01\x01\xff\xff\xff\0\0\xff\x01\0\x01\0\0\0\x02\x05\x03\xfe\xfe\xfd\0\x03\x01\x03\xff\0\x01\x01\x01\xff\xff\xff\x02\0\0\0\x01\x02\x01\xff\0\0"
            "\x01\0\0\x02\x01\x01\0\xff\xff\x01\x01\x01\x03\x02\x03\0\0\xff\0\x01\x01\xfe\xff\xfe\0\0\0\x01\x02\x01\0\0\0\0\0\0\x01\x02\x02\x04\x02\0\0\xfe\xfe\xfe\xff\xff\xff\x03\x03\x02\x05\x03\0\xff\x01\x01\x01\x01\0\x02\x01\0\0\0\x03\0\xff\xff\xfd\xff\0\xff\xfe"
            "\0\xfd\xff\0\xff\x01\0\xfb\xfc\xfb\0\0\0\x04\xfe\0\0\x02\x01\x01\x03\x04\x02\x06\x03\x01\x05\x06\x03\xfe\x01\xfd\x03\x02\x02\x02\x02\x02\xff\xff\xff\xff\xff\0\0\0\xff\xff\xfe\x01\xf6\xf8\xfb\xfe\x01\x01\0\0\x02\0\xff\x01\x04\0\xff\xff\x02\x03\x01\x04"
            "\x03\x01\x06\x08\x01\xff\0\xff\x15\x0e\x06\x04\x02\x01\x02\0\x03\x01\x02\x01\xfe\xff\xfd\xfc\xff\xff\xeb\xf1\xfa\x0e\x0c\x04\xf6\xf7\xfd\0\0\x02\xff\xfe\x01\x04\x01\0\xff\x05\x03\x02\t\t\x03\x04\x04\xff\x0f\t\x02\xed\xe8\xed""31\xe5\" \xef\xff\0\x01\xdd"
            "\xdd\x0f\xd2\xd6\x1d\x1b\x1b\x15\xe9\xee\xf9\n"
            "\t\x04\xff\xfe\xfd\xf6\xfd\xfd\x04\x04\x04\x02\x05\x06\x02\x01\x02\x01\x0e\n"
            "\x01\xea\xe2\xe0\x8d\x8e\xe6,1\xee\0\0\0\0\0\0\0\0\xf0\xd0\xca\xeduu\x1e\x1e&)\xe7\xed\xf9\x06\x06\x01\xf9\xfc\xff\x03\x1e\"#\x05\x06\x02\x12\x0e\x05\xed\xe6\xee\x97\x91\xdc'/\xf0\0\0!\0\0""0\0\0 \0\0\x01\x18\x1b\xea""5-\xf3\xb3\xb5\x13\x0c\x10\t\xf9"
            "\xfb\xfe\0\xfe\x01\x04\x03\x02\x01\x02\x02\x01\x04\xff\x01""83\xe6\",\xf1\0\0\x1e\0\0;\0\0\x07\0\0\0\0\0\x1c\0\0\xfe)4\xe1,'\xde\xe7\xf2""4\xf0\0\xfc\x02\x02\x01\x02\x02\x03\x01\x03\x02\x03\x03\x02\x03! \xee\0\0\0\0\0\"\0\0\x07\0\0\0\0\0\0\0\0\x07\0\0"
            "!\0\0\0\"!\xf0\t\x04\x07\xfe\xfe\xfe\0\0\xff\x02\xf9\xfa\xfb\xff\xff\xff\x02\x01\0\0\xff\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xff\xff\xfe\0\x02\x03\x01\x02\x02\0\0\xff\x02\0\xfe\xfe\0\0\0\xfc\0\xfe\xdc\xde\x0f\0\0\0\0\0\xdf\0\0\xf9\0\0\0"
            "\0\0\0\0\0\xf9\0\0\xdf\0\0\0\xdf\xe0\x10\xfd\xff\xfe\x04\x01\x02\xff\0\x02\x04\x01\x04\x05\xfe\xff\xfe\xfc\0\0\xce\xd3\x1e\xd8\xce\xeb(2\xe1\0\0\x02\0\0\x1e\0\0\0\0\0\xe2\0\0\xe1\xd2\xc8\x01\xdb\xe0.\n"
            "\x12\xfd\xfc\xff\xfc\xfd\xfd\xff\x02\x02\0\x01\0\xfe\x01\xef\xf2\xfc\x16\x1c\x15mp!\xce\xc9\x0b\0\0\xdf\0\0\xc1\0\0\xc1\0\0\xdf\xca\xc3\x03{}/\x19\x1f\x1a\xeb\xee\xf6\xfc\xfb\xfe\0\xff\xff\x01""9BF\x08\x07\x04\t\t\x01\x01\x02\x01\x19\x13\x08\xec\xe6\xeb"
            "'%\xdf# \xf1\xff\0\xfd\xe0\xe1\x10\xdf\xe1-\x17 \x1b\xe0\xe2\xeb\xf8\xf8\xfb\xf7\xf8\xfe\xfc\xfc\xfe\x04\xff\xff\0\xfa\xfb\xfd\xff\0\x01\x0b\n"
            "\x05\xe3\xe9\xf7\x15\x14\x13\xec\xf3\x14\xf3\xd8\x05\x02\x02\x05\xfc\xfe\xfe\t\x12\xfc\xdb\xdf\xe9\0\xfc\xfe\xf9\xf9\xfd\xff\0\x03\xfd\xfe\xff\x04\xfe\xff\xff\x01\xfd\xff\xfd\x02\x04\xf5\xf4\xfb\x07\x08\x03\xed\xf0\xf9\xfc\xfd\xff\x04\x03\xf9\0\0\x01"
            "\x01\x02\0\xff\xff\xfe\xfe\xfc\0\xf9\xf9\xfd\xfb\xfd\x01\xfe\xfd\xfc\0\0\0\x04\x01\0\x01\0\x01\x01\xfb\xfb\xfd\xfc\x01\xff\xfc\xfd\xfe\0\x02\x01\x05\x03\x02\xfe\xfd\xfe\xfc\xfd\xfd\x01\xff\0\xfe\x01\x01\xfe\xfd\xfe\xfd\xfe\0\0\xfe\x01\xfd\xfe\xff\x01"
            "\x01\x01\x04\x01\x02\x01\0\x01\x01\x01\0\x01\x03\x03\x03\xfe\xff\xff\xfd\xfb\xfd\xfd\xfc\xff\0\x01\xff\x02\x01\x03\xff\0\xfe\xff\xfe\xff\0\0\x01\x01\0\0\xff\x01\0\x04\x03\x03\x02\x02\x02\x04\x04\x03\x03\x01\0\0\xfe\xff\xff\xfd\xfd\xfc\xff\x01\x01\x03"
            "\x03\x02\x01\0\0\xfe\xff\xff\0\0\xff\0\xff\0\x01\x01\x01\0\0\0\xff\xff\xff\x01\0\0\x01\x02\0\xfe\xfe\xfe\x04\xfe\xfe\xfe\xfe\xff\0\x03\x02\x01\x03\x04\x02\x03\x03\x02\xfd\0\xfe\x01\x03\x03\x04\x02\0\xff\xff\xff\xff\0\x01\0\xfd\xfe\xfe\xfd\xff\x01\xfe"
            "\0\0\xfe\0\0\xff\0\xfe\xfe\xfd\x04\xfe\xff\xff\x03\x02\x01\0\x03\x01\x02\x05\x02\0\x05\x01\x06\x02\x03\x01\x01\x02\xfd\0\xfe\x02\0\0\xff\xff\x01\xff\xff\0\x04\x01\x02\xfd\xf9\xfe\xff\xfb\xfe\xfd\xfc\xfd\x02\x03\x03\x04\x02\x02\x02\x01\x03\x01\x04\x07"
            "\x03\x02\x07\x02\xfe\xff\xff\x07\x11\x05\xff\xfd\xfc\x0b\x02\x02\0\x02\x01\xfc\xff\0\xfc\xff\x02\xfb\xf0\xfd\x02\n"
            "\x02\xfc\xf7\xff\x01\0\x02\x01\xfe\x01\x04\x02\x02\x02\x01\x04\0\0\x06\x01\0\x05\0\x07\t\x01\xf1\xe5\xe2\xe7""0\xe3\xef$\xfb\x01\xff\0\x0f\xdb\x04\x1a\xd5\x1f\x12\x1d\x1f\xf6\xee\xfa\x04\x08\x02\xfd\xfe\0\xfd\xfa\xfe\x04\x01\x02\xff\0\x03\0\0\0\xff\x05"
            "\t\x02\xe5\xe1\xd1\xe7\x8e\xf8\xed-$\0\0\x1a\0\0\0\xef\0\xe5\xed\xcf\xd9\x1et\xeb!'4\xf4\xe9\xf9\x03\x08\x02\0\xfe\0\x03\x1d$$\x04\x07\x04\x06\x11\x06\xf4\xe5\xe4\xdd\x94\xee\xef)7!\0N/\0""0 \0(\x02\0%\xe9\x19\xfd\xf5""3\xda\x13\xb2\x05\x08\x13\x0f\xff"
            "\xfb\0\xff\xfd\xfe\x04\x04\x06\x03\x01\x01\x01\x03\xff\xfd\xe8""5\xe4\xf0%\"\x1e\0""E;\0\x1c\x07\0\x04\0\0\0\x1d\0\x0c\xfe\0""D\xe1,\xea\xe1)\xdc.\xf1\x1b\xfa\xfe\xfd\xfe\0\0\x02\0\0\xff\x01\x01\0\x06\x02\0\xee\"\xf9\0\0\x1c\"\0\x10\x07\0\x04\0\0\x03"
            "\0\0\x03\x07\0\x04!\0\x10\0\0\x1b\xef#\xfa\x06\x06\x02\0\xff\xff\x02\x01\0\x02\xfd\xfd\xfd\0\0\0\0\x02\x01\0\0\0\0\0\xff\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xff\xff\xff\x05\x01\x04\x01\0\x01\0\0\0\x02\xfd\xfd\xff\xfe\xfe\0\xfb\xfe\0\x10\xdb\x05"
            "\0\0\xe5\xdf\0\xf0\xf9\0\xfc\0\0\xfd\0\0\xfd\xf9\0\xfc\xdf\0\xf0\0\0\xe4\x10\xde\x04\xfd\xfe\x02\x01\x03\x01\0\x01\x02\x04\x03\x01\x02\xff\0\xff\xfe\xfe\x02\x1c\xd1\x1f\xeb\xd6\xd9\xe1*\xbc\x02\0""D\x1e\0\x10\0\0\0\xe2\0\xf0\xe1\0\xbc\x04\xd0\xd2-\xdf"
            "8\xf9\x12\xfd\xfd\xfd\xfd\xff\xfc\xfd\x04\0\xfe\0\x03\x01\x02\xfb\xf2\xfd\x0e\x1c\x1c n\xeb\xe2\x89\xc2\xdf""3\xe9\xdf\0\x1c\0\0\0\0\0\xe4\x06\xc8\xd2*\x85\x1b\x11*!\xe6\xde\xe9\xfe\xfd\xff\xfe\xfd\xff\x01<EJ\x02\x06\x02\x03\n"
            "\x02\x01\x01\xff\x0c\x16\n"
            "\xf0\xe6\xe2\xe2$\xde\xef$\xfb\xfe\xff\xfe\x10\xde\x04,\xe2""0\x14\x1d!\xe8\xe4\xec\xfb\xfb\xfc\xfd\xf6\xff\0\xfc\xff\x01<DI\0\x03\x01\x06\x08\x05\0\x06\xff\xff\xfd\x01\x07\x12\x04\x06\x06\x01\x07\x06\x04\x03\x01\x02\xfd\xfe\x02\xf8\xfb\xfd\xf6\xeb\xf5"
            "\xff\x01\0\xfd\xf8\xfd\xfd\xfa\xff\xff\xfd\0\x04\xff\0\xff\x01\xfe\0\xfb\xfa\xfc\xff\xfa\xfe\x05\n"
            "\x05\xfb\xf2\xfc\xfb\xfd\0\xf2\xff\xf9\x03\x02\x03\0\x01\xff\xfe\xff\xff\0\xfd\xff\xfe\xfb\xff\xfd\xfa\xfe\xfe\xfd\xfe\xff\xff\0\x04\xff\xff\0\xff\xff\xff\x01\xfb\xfd\0\x01\0\xfd\xf5\xfd\xfc\x02\xfe\0\x02\x02\0\0\xff\xfd\x01\xfd\0\xfe\0\xff\xff\xff\0"
            "\xfd\0\xff\xfe\0\0\xff\0\x02\x01\x01\0\x01\x01\x01\0\0\xff\xff\xfc\x87\xf6\x8dp\xd9j0\0\0\0\0IEND\xae""B`\x82";

        const char* APILights_png = (const char*)temp_binary_data_2;


        const char* getNamedResource(const char* resourceNameUTF8, int& numBytes);
        const char* getNamedResource(const char* resourceNameUTF8, int& numBytes)
        {
            unsigned int hash = 0;

            if (resourceNameUTF8 != nullptr)
                while (*resourceNameUTF8 != 0)
                    hash = 31 * hash + (unsigned int)*resourceNameUTF8++;

            switch (hash)
            {
            case 0xd77322a1:  numBytes = 20615; return APILights_png;
            default: break;
            }

            numBytes = 0;
            return nullptr;
        }

        const char* namedResourceList[] =
        {
            "APILights_png"
        };

        const char* originalFilenames[] =
        {
            "APILights.png"
        };

        const char* getNamedResourceOriginalFilename(const char* resourceNameUTF8);
        const char* getNamedResourceOriginalFilename(const char* resourceNameUTF8)
        {
            for (unsigned int i = 0; i < (sizeof(namedResourceList) / sizeof(namedResourceList[0])); ++i)
                if (strcmp(namedResourceList[i], resourceNameUTF8) == 0)
                    return originalFilenames[i];

            return nullptr;
        }

    }
}
