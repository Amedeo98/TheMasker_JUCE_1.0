/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

//==============================================================================
/**
*/
class TheMaskerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor&);
    ~TheMaskerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TheMaskerAudioProcessor& audioProcessor;
    
    CustomRotarySlider
    inSlider,
    outSlider,
    scSlider,
    compSlider,
    expSlider,
    cleanUpSlider;
    
    std::vector<juce::Component*> getComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerAudioProcessorEditor)
};
