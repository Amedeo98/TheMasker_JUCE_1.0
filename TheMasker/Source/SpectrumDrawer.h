/*
  ==============================================================================

    SpectrumDrawer.h
    Created: 15 Feb 2023 1:29:59pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include"Drawer.h"

class SpectrumDrawer : public Drawer {
public:
    SpectrumDrawer() : Drawer(npoints, npoints)
    {
        mindB = _mindBFS;
        maxdB = 0.0f;
    }

    ~SpectrumDrawer() {}

    void drawNextFrameOfSpectrum(array<float, npoints>& values)
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i * scope_step) * _spectrumSkew);
            auto fftDataIndex = juce::jlimit(0, resultSize, (int)(_spectrumPaddingLowFreq + skewedProportionX * (float)resultSize * (0.5f + _spectrumPaddingHighFreq) ));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(values[fftDataIndex])
                - juce::Decibels::gainToDecibels((float) resultSize)
            ), mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }


    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) override {

        auto left = bounds.getX();
        auto width = bounds.getWidth() + left;
        auto height = bounds.getHeight() + bounds.getY();

        g.setColour(colour);


        for (int i = 1; i < scopeSize; ++i)
        {
            if(jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f) != float(height) &&
               jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f) != float(height))
            {
                xVal = { jmap(freqAxis[i - 1] , 0.f, 1.f, (float)left, (float)width),
                    jmap(freqAxis[i] , 0.f, 1.f, (float)left, (float)width) };
                
                juce::Line<float> line (xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                                        xVal[1], jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f));
                
                g.drawLine (line, 2.0f);
                
                //dots :)
                //g.fillRoundedRectangle(xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f), 3.0f, 3.0f, 6.0f);
                
            }
        }
    }

};
