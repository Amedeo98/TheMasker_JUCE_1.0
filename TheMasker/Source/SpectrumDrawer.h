/*
  ==============================================================================

    SpectrumDrawer.h
    Created: 15 Feb 2023 1:29:59pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

class SpectrumDrawer {
public:

    void setColour(juce::Colour col) {
        colour = col;
    }

    void drawNextFrameOfSpectrum(vector<float> result)
    {
        //freqs_Content = conv.interpolateYvector(F, result, frequencies, false);

        auto mindB = -100.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < scopeSize; ++i)                         // [3]
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(result[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)fftSize)
            ), mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;                                   // [4]
        }
    }


    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto width = bounds.getWidth();
            auto height = bounds.getHeight();
            g.setColour(colour);
            vector<float> xVal = { (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width),
                                    (float)juce::jmap(i,    0, scopeSize - 1, 0, width) };
            /*   vector<float> xVal = { (float)juce::jmap( frequencies[i-1], (float) minFreq, (float) maxFreq, 0.0f, (float) width),
                                     (float)juce::jmap(  frequencies[i], (float) minFreq, (float) maxFreq, 0.0f, (float) width) };*/
            g.drawLine({ xVal[0], juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                          xVal[1], juce::jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
        }
    }
private:
    juce::Colour colour;
    int fftSize = 1 << _fftOrder;
    int scopeSize = npoints;
    float scopeData[npoints];
};