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

    void prepareToPlay(vector<float> freqs, vector<float> fCents, juce::Colour col) {
        colour = col;
        frequencies.resize(_fftSize);
        fCents.resize(nfilts);
        freqAxis.resize(_fftSize);
        frequencies = freqs;
        fCenters = fCents;

        for (int i = 0; i < _fftSize; i++)
        {
            freqAxis[i] = juce::mapFromLog10(frequencies[i], (float)minFreq, (float)maxFreq);
        }

    }




  

    void drawNextFrameOfSpectrum(vector<float> result)
    {


        for (int i = 0; i < scopeSize; ++i)                         // [3]
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, _fftSize / 2, (int)(skewedProportionX * (float)_fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(result[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)_fftSize)
            ), mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;                                   // [4]
        }
    }



    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        auto left = bounds.getX();
        for (int i = 1; i < scopeSize; ++i)
        {

            g.setColour(colour);
            vector<float> xVal = { jmap( freqAxis[i-1] , 0.f, 1.f, (float) left, (float)width),
                                   jmap( freqAxis[i] , 0.f, 1.f, (float) left, (float)width)};
            
            
            /*   vector<float> xVal = { (float)juce::jmap( frequencies[i-1], (float) minFreq, (float) maxFreq, 0.0f, (float) width),
                                     (float)juce::jmap(  frequencies[i], (float) minFreq, (float) maxFreq, 0.0f, (float) width) };*/
            g.drawLine( xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                          xVal[1], jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) );
        }
    }

    float toLog(float x) {
        return log10(x);
        //return mapFromLog10<float>(x, log10(minFreq), log10(maxFreq));
    }
private:
    vector<float> freqAxis;
    vector<float> frequencies;
    vector<float> fCenters;
    float scope_step = pow(_fftSize,-1);
    Converter conv;
    float mindB = -100.0f;
    float maxdB = 0.0f;
    juce::Colour colour;
    int scopeSize = _fftSize;
    float scopeData[_fftSize];
};