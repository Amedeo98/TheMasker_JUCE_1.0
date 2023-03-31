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
        
        //recatngles for numeric values
        auto rL = Rectangle<float>(left - 12, chHeight + top + 4, 24, 16);
        auto rR = Rectangle<float>(left + chWidth + 8, chHeight + top + 4, 24, 16);
        g.fillRoundedRectangle(rL, 4);
        g.fillRoundedRectangle(rR, 4);
        
        PathStrokeType(8.0);
        g.fillPath(bkg);
    
        g.setColour(Colour(200u, 64u, 164u).withAlpha(0.8f));
        
        // Draw the left channel meter
        float leftLevel = currentLevel[0] * chHeight;
        g.fillRect(Rectangle<int>(left, top + chHeight - max_L* chHeight, chWidth, max_L* chHeight));
        
        // Draw the right channel meter
        //float rightLevel = mapToLog10(currentLevel[1], 0.01f, 1.0f) * chHeight;
        float rightLevel = currentLevel[1] * chHeight;
        g.fillRect(Rectangle<int>(left + chWidth + 14, top + chHeight - max_R* chHeight, chWidth, max_R* chHeight));
        
        gradient = ColourGradient(Colour(255u, 100u, 200u), left, top, Colour(40u, 220u, 0u), left+chWidth, top+chHeight, false);
        g.setGradientFill(gradient);
        
        g.fillRect(Rectangle<int>(left, top + chHeight - leftLevel, chWidth, leftLevel));
        g.fillRect(Rectangle<int>(left + chWidth + 14, top + chHeight - rightLevel, chWidth, rightLevel));
        
        // Draw db value
        String str_L;
        if (int(dB_L) > 0) str_L << "+";
        str_L << int(dB_L);
        
        String str_R;
        if (int(dB_R) > 0) str_R << "+";
        str_R << int(dB_R);
        
        g.setColour(Colours::black);
        g.drawFittedText(str_L, left - 12, chHeight + top + 4, 24, 16, juce::Justification::centred, 1);
        g.drawFittedText(str_R, left + chWidth + 8, chHeight + top + 4, 24, 16, juce::Justification::centred, 1);
        
        /*
        for (auto gDb : meterGain)
        {
            String str;
            if (gDb > 0)
                str << "+";
            if (gDb <= -90)
                str << "-inf";
            else
                str << gDb;
            
            Rectangle<int> r;
            r.setSize(20, 10);
            r.setX(bounds.getX());
            
            auto y = jmap(gDb, (float)(_mindBFS),6.0f,0.0f,1.0f);
            y = jmax(0.0f, jmin(1.0f, y));
            y = mapToLog10(y, 0.01f, 1.0f) * chHeight;
            
            r.setCentre(r.getCentreX(), top + chHeight - y);
            
            g.setColour(Colours::black);
            g.drawFittedText(String(str), r, juce::Justification::centredRight , 1);
            
            str.clear();
        } */
    
    }
    

    void setLevel(float left, float right, float max_lev_L, float max_lev_R) {
        dB_L = Decibels::gainToDecibels(max_lev_L);
        dB_R = Decibels::gainToDecibels(max_lev_R);

        currentLevel[0] = juce::jmap(Decibels::gainToDecibels(left), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        currentLevel[1] = juce::jmap(Decibels::gainToDecibels(right), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        currentLevel[0] = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        currentLevel[1] = jmax(0.0f, jmin(1.0f, currentLevel[1]));
        
        max_L = juce::jmap(dB_L, (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        max_R = juce::jmap(dB_R, (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        max_L = jmax(0.0f, jmin(1.0f, max_L));
        max_R = jmax(0.0f, jmin(1.0f, max_R));
    }
    

private:
    float currentLevel[2] = { 0.0f, 0.0f };
    Converter conv;
    float dB_L, dB_R;
    float max_L, max_R;
    ColourGradient gradient{};
    
    Array<float> meterGain
    {
       -90, -48, -24, -18, -12, -6, 0, 6
    };
    
    
};

