/*
  ==============================================================================

    VolumeMeter.h
    Created: 22 Mar 2023 12:22:11pm
    Author:  nikid

  ==============================================================================
*/
#include <JuceHeader.h>
#include "Converters.h"

#pragma once

class VolumeMeter {
public:
    VolumeMeter() {
    }

    void draw(Graphics& g, juce::Rectangle<int>& bounds) {
        
        g.setColour(Colours::white.withAlpha(0.5f));
        
        auto top = bounds.getY()+16;
        auto left = bounds.getX()+14;
        auto height = bounds.getHeight();
        auto width = bounds.getWidth();
        
        auto chWidth = width * 0.2;
        auto chHeight = height - 32;
        
        g.fillRect(Rectangle<int>(left, top, chWidth, chHeight));
        g.fillRect(Rectangle<int>(left + width - (chWidth + 28), top, chWidth, chHeight));
        
        // Draw the left channel meter
        g.setColour(Colours::green);
        float leftLevel = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        leftLevel *= chHeight;
        g.fillRect(Rectangle<int>(left, top + chHeight - leftLevel + 1, chWidth, leftLevel));
        
        // Draw the right channel meter
        g.setColour(Colours::red);
        float rightLevel = jmax(0.0f, jmin(1.0f, currentLevel[1])) * chHeight;
        g.fillRect(Rectangle<int>(left + width - (chWidth + 28), top + chHeight - rightLevel + 1, chWidth, rightLevel));
        
    }

    //void resized() override {
    //    // Nothing to do here, since we're not adding any child components
    //}

    void setLevel(float left, float right) {
        currentLevel[0] = juce::jmap(Decibels::gainToDecibels(left), (float) _mindBFS, 0.0f, 0.0f, 1.0f);
        currentLevel[1] = juce::jmap(Decibels::gainToDecibels(right), (float)_mindBFS, 0.0f, 0.0f, 1.0f);
    }

private:
    float currentLevel[2] = { 0.0f, 0.0f };
    Converter conv;
    
};

