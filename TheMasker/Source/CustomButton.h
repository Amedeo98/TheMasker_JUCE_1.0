/*
  ==============================================================================

    CustomButton.h
    Created: 31 Mar 2023 11:04:19pm
    Author:  Amedeo Fresia

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomButton)
};
