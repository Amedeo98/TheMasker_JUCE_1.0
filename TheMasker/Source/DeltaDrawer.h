/*
  ==============================================================================

    DeltaDrawer.h
    Created: 3 Mar 2023 10:09:56am
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include "Drawer.h"
#include "Converters.h"

class DeltaDrawer : public Drawer {
public:
    DeltaDrawer() : Drawer(nfilts, nfilts)
    {
        mindB = (float) -_maxGain;
        maxdB = (float) _maxGain;
        result_step = pow(resultSize, -1);

    }
    ~DeltaDrawer() {}


    void drawNextFrameOfSpectrum(vector<float> result)
    {


        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * _spectrumSkew);
            auto fftDataIndex = juce::jlimit(0, resultSize / 2, (int)(skewedProportionX * (float)resultSize * 0.5f));
            auto level = juce::jmap(result[fftDataIndex], mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) override {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto width = bounds.getWidth();
            auto height = bounds.getHeight();
            auto left = bounds.getX();
            xVal = { jmap(freqAxis[i - 1] , 0.f, 1.f, (float)left, (float)width),
                                   jmap(freqAxis[i] , 0.f, 1.f, (float)left, (float)width) };

            g.setColour(colour
                .withAlpha(jlimit(0.0f, 1.0f, abs(scopeData[i]-0.5f) * 1.5f)) //1.5f : moltiplicatore dell'alfa del rosso - poi clippato tra 0 e 1
            );
            g.drawLine(xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                xVal[1], jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f));
        }
    }


private:
    float result_step;
};
