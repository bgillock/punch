/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 punch
  vendor:             BitRate27
  version:            1.0.0
  name:               Punch extensions for JUCE
  description:        Classes to extend JUCE
  website:            http://www.juce.com/juce
  license:            GPL
  minimumCppStandard: 14

  dependencies:       juce_gui_basics, juce_audio_processors, juce_core

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define PUNCH_H_INCLUDED
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "./TwoValueAttachment/TwoValueAttachment.h"
#include "./Slider/SmoothSlider.h"
#include "./Fader/FaderSlider.h"
#include "./Annotation/dbAnnoComponent.h"
#include "./Meter/MaximumAmp.h"
#include "./Meter/StereoLevelMeter.h"
