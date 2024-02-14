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
class TheMaskerComponent  : public juce::Component, private juce::Timer,
    public juce::Button::Listener
#ifdef fineTuneCoeff
    , public juce::Slider::Listener
#endif // fineTuneCoeff
{
public:
    TheMaskerComponent (TheMaskerAudioProcessor&);
    ~TheMaskerComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (Button*) override;

private:
    TheMaskerAudioProcessor& audioProcessor;
    
    Rectangle<int> in_area,
        out_area,
        responseArea;

    Array<float> xs;

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
    
#ifdef fineTuneCoeff
        Slider TS1, TS2, TS3, TS4, TS5, TS6, TS7, TS8, TS9, TS10, TS11;
        int tsIndx[11];
        bool corrInv[11];

        void sliderValueChanged(Slider* slider) override;
        void setupTunerSlider(Slider& slider, int i);

#endif // fineTuneCoeff

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
        //repaint(responseArea);
        //repaint(in_area);
        //repaint(out_area);
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerComponent)
};


//==============================================================================

class Wrapper : public AudioProcessorEditor {
    
public:
    Wrapper(TheMaskerAudioProcessor&); 
    
    void resized() override;
    
private:
    static constexpr int originalWidth{ 824 };
    static constexpr int originalHeight{ 476 };
    
    TheMaskerComponent theMaskerComponent;
}; 
