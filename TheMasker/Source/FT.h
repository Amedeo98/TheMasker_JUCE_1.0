/*
  ==============================================================================

    FT.h
    Created: 20 Jan 2023 6:17:33pm
    Author:  nikid

  ==============================================================================
*/

#include "OverlappingFFTProcessor.h"


#pragma once
class FT : public OverlappingFFTProcessor {

public:
    FT() : OverlappingFFTProcessor(_fftOrder, hopSizeDividerAsPowOf2)
    {
        //startTimerHz (30);
        result.resize(pow(2, _fftOrder - hopSizeDividerAsPowOf2));
        result_decim.resize(nfilts);
    }
    ~FT() {}
    
    
    auto getFT(AudioBuffer<float>& input, int ch)  {
        processFrameInBuffer(1);
        process(input);
        result = getResult();
        if (decimated)
        result_decim = conv.mXv_mult(fbank_values, result);

        drawNextFrameOfSpectrum();


        return decimated ? result_decim : result;
    }
    
    /*void timerCallback() override
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }*/

    

    void setFBank(FilterBank fb) {
        fbank_values = fb.getValues();
        decimated = true;
    }

    

    void drawNextFrameOfSpectrum()
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(result[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)fftSize)),
                mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }

    //CHIAMARE SU PLUGIN EDITOR!!!
    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto width = bounds.getWidth();
            auto height = bounds.getHeight();

            g.drawLine({ (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width),
                                  juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                          (float)juce::jmap(i,     0, scopeSize - 1, 0, width),
                                  juce::jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
        }
    }


    vector<float> result;
    vector<float> result_decim;

private:
    void processFrameInBuffer(const int maxNumChannels) override
    {
        for (int ch = 0; ch < maxNumChannels; ++ch)
            fft.performRealOnlyForwardTransform(fftInOutBuffer.getWritePointer(ch), true);

        // clear high frequency content
        for (int ch = 0; ch < maxNumChannels; ++ch)
            FloatVectorOperations::clear(fftInOutBuffer.getWritePointer(ch, fftSize / 2), fftSize / 2);

        for (int ch = 0; ch < maxNumChannels; ++ch)
            fft.performRealOnlyInverseTransform(fftInOutBuffer.getWritePointer(ch));
    }

    int scopeSize = npoints;
    float scopeData[npoints];
    float mindB = -100.0f;
    float maxdB = 0.0f;
    vector<vector<float>> fbank_values;
    Converter conv;
    bool decimated = false;
    bool nextFFTBlockReady = false;
  

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT)
};


