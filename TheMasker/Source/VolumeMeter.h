/*
  ==============================================================================

    VolumeMeter.h
    Created: 22 Mar 2023 12:22:11pm
    Author:  nikid

  ==============================================================================
*/
#include <JuceHeader.h>
#include "Converters.h"
#include "CustomSmoothedValue.h"
#include "Constants.h"

#pragma once

class VolumeMeter {
public:
    VolumeMeter() {
    }
    
    void prepareToPlay(int fs, float relSmoothingSeconds, bool sc) {
        hasSC = sc;
        max_lev_L.reset(fs, 0.f, relSmoothingSeconds);
        max_lev_R.reset(fs, 0.f, relSmoothingSeconds);
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
    
        g.setColour(_purple);
        
        // Draw the left channel meter (max_L)
        float leftLevel = currentLevel[0] * chHeight;
        g.fillRect(Rectangle<int>(left, top + chHeight - max_L* chHeight, chWidth, max_L* chHeight));
        
        // Draw the right channel meter (max_R)
        float rightLevel = currentLevel[1] * chHeight;
        g.fillRect(Rectangle<int>(left + chWidth + 14, top + chHeight - max_R* chHeight, chWidth, max_R* chHeight));

         
        //real time rms
        gradient = ColourGradient(_yellow, left, top, _green, left+chWidth, top+chHeight, false);
        g.setGradientFill(gradient);
        g.fillRect(Rectangle<int>(left, top + chHeight - leftLevel, chWidth, leftLevel));
        g.fillRect(Rectangle<int>(left + chWidth + 14, top + chHeight - rightLevel, chWidth, rightLevel));
        
        
        //sc
         if(hasSC)
        {
            float scL = scLevel[0] * chHeight;
            float scR = scLevel[1] * chHeight;
            g.setColour(_yellow);
            g.fillRect(Rectangle<int>(left-1, top + chHeight - scL, chWidth+2, 2.f));
            g.fillRect(Rectangle<int>(left + chWidth + 14-1, top + chHeight - scR, chWidth+2, 2.f));
        }
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
        
        for (auto gDb : meterGain)
        {
            String str;
            if (gDb > 0)
                str << "+";
            if (gDb <= -96)
                str << "-inf";
            else
                str << gDb;
            
            Rectangle<int> r;
            r.setSize(20, 10);
            r.setX(bounds.getX());
            
            auto y = jmap(gDb, (float)(_mindBFS),6.0f,0.0f,1.0f);
            y = jmax(0.0f, jmin(1.0f, y)) * chHeight;
            
            r.setCentre(r.getCentreX(), top + chHeight - y);
            
            g.setColour(Colours::black.withAlpha(0.3f));
            g.drawFittedText(String(str), r, juce::Justification::centredRight , 1);
            
            str.clear();
        }
    
    }
    
    void skip(AudioBuffer<float>& buffer) {
        max_lev_L.skip(buffer.getNumSamples());
        max_lev_R.skip(buffer.getNumSamples());
    }
    

    void setLevel(float left, float right) {
         if(left > max_lev_L.getCurrentValue())
         {
             max_lev_L.setTargetValue(left);
             dB_L = Decibels::gainToDecibels(max_lev_L.getCurrentValue());
         }
         else
             max_lev_L.setTargetValue(0.f);

         if(right> max_lev_R.getCurrentValue())
         {
             max_lev_R.setTargetValue(right);
             dB_R = Decibels::gainToDecibels(max_lev_R.getCurrentValue());
         }
         else
             max_lev_R.setTargetValue(0.f);
         
        

        currentLevel[0] = juce::jmap(Decibels::gainToDecibels(left), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        currentLevel[1] = juce::jmap(Decibels::gainToDecibels(right), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        currentLevel[0] = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        currentLevel[1] = jmax(0.0f, jmin(1.0f, currentLevel[1]));
        
        max_L = juce::jmap(Decibels::gainToDecibels(max_lev_L.getCurrentValue()), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        max_R = juce::jmap(Decibels::gainToDecibels(max_lev_R.getCurrentValue()), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        max_L = jmax(0.0f, jmin(1.0f, max_L));
        max_R = jmax(0.0f, jmin(1.0f, max_R));
    }
    
    
    void setSCLevel(float left, float right)
    {
        scLevel[0] = juce::jmap(Decibels::gainToDecibels(left), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        scLevel[1] = juce::jmap(Decibels::gainToDecibels(right), (float)_mindBFS, 6.0f, 0.0f, 1.0f);
        scLevel[0] = jmax(0.0f, jmin(1.0f, scLevel[0]));
        scLevel[1] = jmax(0.0f, jmin(1.0f, scLevel[1]));
    }
    
    void resetMaxVolume() {
        max_lev_L.setCurrentAndTargetValue(0.f);
        max_lev_R.setCurrentAndTargetValue(0.f);
    }
    

private:
    float currentLevel[2] = { 0.0f, 0.0f };
    float scLevel[2] = { 0.0f, 0.0f };
    bool hasSC = true;
    
    Converter conv;
    float dB_L, dB_R;
    float max_L, max_R;
    ColourGradient gradient{};
    
    Array<float> meterGain
    {
        -96, -48, -24, -18, -12, -6, 0
    };
    
    CustomSmoothedValue<float, ValueSmoothingTypes::Linear> max_lev_L, max_lev_R;

    
};

