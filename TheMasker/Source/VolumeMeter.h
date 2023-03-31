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
        auto left = bounds.getX()+24;
        auto height = bounds.getHeight();
        
        auto chWidth = 6;
        auto chHeight = height - 40;
        
        Path bkg;
        g.setColour(Colours::white.withAlpha(0.8f));
        bkg.addRoundedRectangle(left, top, chWidth, chHeight, 6, 2.0f);
        bkg.addRoundedRectangle(left + chWidth + 14, top, chWidth, chHeight, 6, 2.0f);
        auto rL = Rectangle<float>(left - 12, chHeight + top + 4, 24, 16);
        auto rR = Rectangle<float>(left + chWidth + 8, chHeight + top + 4, 24, 16);
        g.fillRoundedRectangle(rL, 4);
        g.fillRoundedRectangle(rR, 4);
        
        PathStrokeType(8.0);
        g.fillPath(bkg);
        g.setColour(Colour(40u, 220u, 0u));
        
        // Draw the left channel meter
        float leftLevel = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        leftLevel *= chHeight;
        g.fillRect(Rectangle<int>(left, top + chHeight - leftLevel + 1, chWidth, leftLevel));
        
        // Draw the right channel meter
        float rightLevel = jmax(0.0f, jmin(1.0f, currentLevel[1])) * chHeight;
        g.fillRect(Rectangle<int>(left + chWidth + 14, top + chHeight - rightLevel + 1, chWidth, rightLevel));
        int offset = 20;
        
        // Draw db value
        String str_L;
        if (dB_L > 0)
            str_L << "+";
        str_L << dB_L;
        
        String str_R;
        if (dB_R > 0)
            str_R << "+";
        str_R << dB_R;
        
        g.setColour(Colours::black);
        g.drawFittedText(str_L, left - 12, chHeight + top + 4, 24, 16, juce::Justification::centred, 1);
        g.drawFittedText(str_R, left + chWidth + 8, chHeight + top + 4, 24, 16, juce::Justification::centred, 1);
        
        for (auto gDb : meterGain)
        {
            String str;
            Rectangle<int> r;
            r.setSize(40, 10);
            r.setX(bounds.getX());
            r.setCentre(r.getCentreX(), bounds.getY()+offset);
            
            g.setColour(Colours::white);
            g.drawFittedText(String(str), r, juce::Justification::centredRight , 1);
            
            str.clear();
            offset += 40;
        }
        


        
    }

    void setLevel(float left, float right) {
        dB_L = int(Decibels::gainToDecibels(left));
        dB_R = int(Decibels::gainToDecibels(right));
        currentLevel[0] = juce::jmap(Decibels::gainToDecibels(left), (float) _mindBFS, 0.0f, 0.0f, 1.0f);
        currentLevel[1] = juce::jmap(Decibels::gainToDecibels(right), (float)_mindBFS, 0.0f, 0.0f, 1.0f);
    }

private:
    float currentLevel[2] = { 0.0f, 0.0f };
    Converter conv;
    int dB_L;
    int dB_R;
    
    Array<float> meterGain
    {
       0, -6, -12, -18, -24
    };
    
};

