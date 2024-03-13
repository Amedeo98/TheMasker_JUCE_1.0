/*
  ==============================================================================

    Sliders.h
    Created: 31 Mar 2023 11:09:58pm
    Author:  Amedeo Fresia

  ==============================================================================
*/

#include <JuceHeader.h>
#pragma once

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
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LnF)
};


class CustomLinearSlider : public juce::Slider
{
public:
    CustomLinearSlider(juce::RangedAudioParameter& rap, const juce::String& name, bool displayValue, bool zeroToOne) : juce::Slider(juce::Slider::SliderStyle::LinearHorizontal,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        sliderName(name),
        displayValue(displayValue),
        zeroToOne(zeroToOne)
    {
        setLookAndFeel(&lnf);
    }
    
    ~CustomLinearSlider()
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override;
    int getTextHeight() const { return 14; }
    juce::String sliderName;
    bool zeroToOne;

private:
    LnF lnf;
    juce::RangedAudioParameter* param;
    bool displayValue;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLinearSlider)
    
};


class CustomRotarySlider : public juce::Slider
{
public:
    CustomRotarySlider(juce::RangedAudioParameter& rap, const juce::String& name, bool displayValue, bool zeroToOne) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
        sliderName(name),
        param(&rap),
        displayValue(displayValue),
        zeroToOne(zeroToOne)
    {
        setLookAndFeel(&lnf);
    }

    ~CustomRotarySlider()
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds(juce::Rectangle<int> bounds) const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
    juce::String sliderName;
    bool zeroToOne;

private:
    
    LnF lnf;
    juce::RangedAudioParameter* param;
    bool displayValue;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomRotarySlider)
};


