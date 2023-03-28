/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


struct LnF : juce::LookAndFeel_V4
{
public:
    LnF(){}
    
    void drawRotarySlider(juce::Graphics&,
                            int x, int y, int width, int height,
                            float sliderPosProportional,
                            float rotaryStartAngle,
                            float rotaryEndAngle,
                            juce::Slider&) override;
    
    void drawLinearSlider (Graphics&,
                                   int x, int y, int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle,
                                   Slider&) override;
};

class CustomButton : public juce::Button
{
public:
    static constexpr int backgroundColourId = 0xdeadbeef;
    
    CustomButton(const juce::String& buttonName = juce::String())
            : juce::Button(buttonName)
    {
        setColour(backgroundColourId, juce::Colours::transparentBlack);
    }
    
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        if(buttonPressed) {
            g.setColour(Colours::black.withAlpha(0.5f));
        }
        else {
            g.setColour(Colours::black.withAlpha(0.0f));
        }
        
        g.fillRect(getLocalBounds());
    }
    
    void toggle()
    {
        buttonPressed = !buttonPressed;
    }
    
private:
    bool buttonPressed = false;
    
};



struct CustomLinearSlider : juce::Slider
{
    CustomLinearSlider(juce::RangedAudioParameter& rap, const juce::String& name, bool displayValue) : juce::Slider(juce::Slider::SliderStyle::LinearHorizontal,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        sliderName(name),
        displayValue(displayValue)
    {
        setLookAndFeel(&lnf);
    }
    
    ~CustomLinearSlider()
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override;
    int getTextHeight() const { return 14; }


private:
    juce::DropShadowEffect shadow;
    juce::DropShadow shadowProps;
    void setShadowProps();
    
    LnF lnf;
    juce::RangedAudioParameter* param;
    juce::String sliderName;
    bool displayValue;
    
};


struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider(juce::RangedAudioParameter& rap, const juce::String& name, bool displayValue) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        sliderName(name),
        displayValue(displayValue)
    {
        setLookAndFeel(&lnf);
    }

    ~CustomRotarySlider()
    {
        setLookAndFeel(nullptr);
    }

    struct LabelPos
    {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds(juce::Rectangle<int> bounds) const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;


private:
    
    LnF lnf;
    juce::RangedAudioParameter* param;
    juce::String sliderName;
    bool displayValue;
    
};


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
    
    std::unique_ptr<juce::Drawable> svgDrawable;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerAudioProcessorEditor)
};
