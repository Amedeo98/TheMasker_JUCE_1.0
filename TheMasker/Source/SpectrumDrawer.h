/*
  ==============================================================================

    SpectrumDrawer.h
    Created: 15 Feb 2023 1:29:59pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include"Drawer.h"
#include "CustomSmoothedValue.h"


class SpectrumDrawer : public Drawer {
public:
    SpectrumDrawer() : Drawer(npoints, npoints)
    {
        mindB = _mindBFS;
        maxdB = 0.0f;
    }

    ~SpectrumDrawer() {}


    void resetSmoothingValues(double fs, int nSamples) {
        for (int i = 0; i < npoints; i++) {
            scopeData[i].reset(fs, _atkSpectrumSeconds, _relSpectrumSeconds);
        }
        numSamples = nSamples;

    }

    void updateValues() {
        for (int i = 0; i < npoints; i++) {
            scopeData[i].getNextValue();
        }
    }

    void drawNextFrameOfSpectrum(array<float, npoints>& values)
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i * scope_step) * _spectrumSkew);
            auto fftDataIndex = juce::jlimit(0, resultSize, (int)(_spectrumPaddingLowFreq + skewedProportionX * (float)resultSize * (0.5f + _spectrumPaddingHighFreq) ));
            auto level = juce::jmap(juce::Decibels::gainToDecibels(values[fftDataIndex]) - juce::Decibels::gainToDecibels((float) resultSize)
                    , mindB, maxdB, 0.0f, 1.0f);

            scopeData[i].skip(numSamples);
            //scopeData[i].getNextValue();

            if(level > scopeData[i].getCurrentValue())
                scopeData[i].setTargetValue(level);
            else 
                scopeData[i].setTargetValue(-1.0f);

        }
    }


    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) override {

        auto left = bounds.getX();
        auto width = bounds.getWidth() + left;
        auto height = bounds.getHeight();

        g.setColour(colour);

        for (int i = 1; i < scopeSize; ++i)
        {
            float yValMapped_left = jmap(scopeData[i - 1].getCurrentValue(), 0.0f, 1.0f, (float)height, 0.0f);
            float yValMapped_right = jmap(scopeData[i].getCurrentValue(), 0.0f, 1.0f, (float)height, 0.0f);

            if (max(yValMapped_left, yValMapped_right) < float(height))
            {
                xVal = { jmap(freqAxis[i - 1] , 0.f, 1.f, (float)left, (float)width),
                    jmap(freqAxis[i] , 0.f, 1.f, (float)left, (float)width) };
                
                juce::Line<float> line (xVal[0], yValMapped_left,
                                        xVal[1], yValMapped_right);
                


                if(xVal[1] < width )
                    g.drawLine (line, 2.0f);
                //dots :)
                //g.fillRoundedRectangle(xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f), 3.0f, 3.0f, 6.0f);
                
            }
        }
    }
private:
    array<CustomSmoothedValue<float, juce::ValueSmoothingTypes::Linear>, npoints> scopeData;
    int numSamples;

};
