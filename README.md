### Punch Modules for JUCE

## Install as submodule in your repo
1. git clone https://github.com/bgillock/punch.git user_modules
2. git submodule add -b master https://github.com/bgillock/punch.git user_modules/punch
3. Commit changes to main repo
-or- 
## Clone into user_module folder
1. cd to you user module folder as specified in Projucer->File->Global paths
2. git clone https://github.com/bgillock/punch.git

## Add module through Projucer
In Projucer
Select Modules pulldown list
Select +->Add a module->Global user modules path->Re-scan path
Select +->Add a module->Global user modules path->punch
Save projucer output to your project

## Using punch extensions

# SmoothSlider
Description: Pressing control while moving mousewheel does finer movement.
Usage: 
#include <punch/punch.h>
punch::SmoothSlider slider(0.8,0.05);

# TwoValueSliderAttachment
Description: Attach plugin parameters to Min and Max values of a TwoValue slider.
Usage:
#include <punch/punch.h>
```
TwoValueSliderAttachment(juce::AudioProcessorValueTreeState& stateToUse,
            const juce::String& parameterID1,
            const juce::String& parameterID2,
            juce::Slider& slider);
```
