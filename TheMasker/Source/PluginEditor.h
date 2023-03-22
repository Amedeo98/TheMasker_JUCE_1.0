/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"



class VolumeMeter : public Component, private Timer {
public:
    VolumeMeter() {
    }

    void paint(Graphics& g) override {
        g.fillAll(Colours::white);

        g.setColour(Colours::black);

        // Draw the left channel meter
        float leftLevel = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        g.fillRect(Rectangle<int>(getWidth() * 0.25f - 50, getHeight() - leftLevel * getHeight(), 100, leftLevel * getHeight()));
        // Draw the right channel meter
        float rightLevel = jmax(0.0f, jmin(1.0f, currentLevel[1]));
        g.fillRect(Rectangle<int>(getWidth() * 0.75f - 50, getHeight() - rightLevel * getHeight(), 100, rightLevel * getHeight()));
    }

    void resized() override {
        // Nothing to do here, since we're not adding any child components
    }

    void setLevel(float left, float right) {
        currentLevel[0] = left;
        currentLevel[1] = right;
    }

private:
    float currentLevel[2] = { 0.0f, 0.0f };

    void timerCallback() override {
        // Trigger a repaint of the component every time the timer fires
        repaint();
    }
    
};



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

struct CustomLinearSlider : juce::Slider
{
    CustomLinearSlider(juce::RangedAudioParameter& rap, const juce::String& name, bool displayValue) : juce::Slider(juce::Slider::SliderStyle::LinearHorizontal,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        sliderName(name),
        displayValue(displayValue)
    {
        setLookAndFeel(&lnf);
        setShadowProps();
    }
    
    ~CustomLinearSlider()
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
        setShadowProps();
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
    
    juce::DropShadowEffect shadow;
    juce::DropShadow shadowProps;
    void setShadowProps();
    
    LnF lnf;
    juce::RangedAudioParameter* param;
    juce::String sliderName;
    bool displayValue;
    
};


//==============================================================================
/**
*/
class TheMaskerAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
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
    
    VolumeMeter inputVolume, outputVolume;
    
    CustomRotarySlider  inSlider,
                        outSlider,
                        scSlider,
                        compSlider,
                        expSlider,
                        mixSlider,
                        stereoLinkedSlider;
    
    CustomLinearSlider cleanUpSlider;
      
    Label inLabel, outLabel, scLabel, compLabel, expLabel, cleanUpLabel, mixLabel, stereoLabel;
    
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
    
    Colour bgColorLight = Colour::greyLevel(0.8);
    Colour bgColorDark = Colour::greyLevel(0.5);
    Colour primaryColor = Colour(97u, 18u, 167u);
    Colour accentColor = Colour(64u, 200u, 64u);
    
    
    std::vector<juce::Component*> getComponents();

    void timerCallback() final {
        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerAudioProcessorEditor)
};
