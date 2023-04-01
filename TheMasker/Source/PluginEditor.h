/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomButton.h"
#include "Sliders.h"

//==============================================================================
/**
*/
class TheMaskerAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer,
                                       public juce::Button::Listener
{
public:
    TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor&);
    ~TheMaskerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (Button*) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TheMaskerAudioProcessor& audioProcessor;
    
    VolumeMeter inputVolume, outputVolume;
    
    CustomButton undoButton, redoButton, loadButton, saveButton,
                 toggleIn, toggleSc, toggleD, toggleOut;
    CustomButton resetInButton, resetOutButton;
    
    CustomRotarySlider  inSlider,
                        outSlider,
                        scSlider,
                        compSlider,
                        expSlider,
                        mixSlider;
    
    CustomLinearSlider cleanUpSlider, stereoLinkedSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment  inSliderAttachment,
                outSliderAttachment,
                scSliderAttachment,
                mixSliderAttachment,
                compSliderAttachment,
                expSliderAttachment,
                stereoLinkedSliderAttachment,
                cleanUpSliderAttachment;
    
    
    std::vector<juce::Component*> getComponents();
    std::vector<CustomButton*> getButtons();

    void timerCallback() final {
        repaint();
    }
    
    Array<float> gain
    {
        -24, -18, -12, -6, 0, 6, 12, 18, 24
    };

    
    Array<float> freqs
    {
        20, 50, 100,
        200, 500, 1000,
        2000, 5000, 10000,
        20000
    };
    
    const int fontHeight = 10;
    
    std::unique_ptr<juce::Drawable> svgDrawable;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerAudioProcessorEditor)
};
