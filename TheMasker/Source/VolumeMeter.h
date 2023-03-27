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
        
        g.setColour(Colours::white.withAlpha(0.8f));
        
        auto top = bounds.getY()+16;
        auto left = bounds.getX()+20;
        auto height = bounds.getHeight();
        auto width = bounds.getWidth();
        
        auto chWidth = 6;
        auto chHeight = height - 32;
        
        Path bkg;
        g.setColour(Colours::white.withAlpha(0.8f));
        bkg.addRoundedRectangle(left, top, chWidth, chHeight, 6, 2.0f);
        bkg.addRoundedRectangle(left + chWidth + 18, top, chWidth, chHeight, 6, 2.0f);
        PathStrokeType(8.0);
        g.fillPath(bkg);
        
        g.setColour(Colour(40u, 220u, 0u));
        
        // Draw the left channel meter
        float leftLevel = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        leftLevel *= chHeight;
        g.fillRect(Rectangle<int>(left, top + chHeight - leftLevel + 1, chWidth, leftLevel));
        
        // Draw the right channel meter
        float rightLevel = jmax(0.0f, jmin(1.0f, currentLevel[1])) * chHeight;
        g.fillRect(Rectangle<int>(left + chWidth + 18, top + chHeight - rightLevel + 1, chWidth, rightLevel));
        
    }

    //void resized() override {
    //    // Nothing to do here, since we're not adding any child components
    //}

    void setLevel(float left, float right) {
        currentLevel[0] = left;
        currentLevel[1] = right;
    }

private:
    float currentLevel[2] = { 0.0f, 0.0f };
    Converter conv;
    
};

