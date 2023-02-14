/*
  ==============================================================================

    FT.h
    Created: 20 Jan 2023 6:17:33pm
    Author:  nikid

  ==============================================================================
*/

#include "Analyser.h"


#pragma once
class FT : public Analyser {

public:
    FT() : Analyser(_fftOrder, npoints)
    {
    }
    ~FT() {}

    void prepare(vector<float>_frequencies, int sampleRate) {
        result_decim.resize(nfilts);
        frequencies = _frequencies;
        scopeData.resize(scopeSize);
        freqs_Error.resize(fftSize);
        F.resize(fftSize);
        F = conv.linspace(1.0f, static_cast<float>(sampleRate / 2), static_cast<float>(fftSize));
        FloatVectorOperations::subtract(freqs_Error.data(), frequencies.data(), F.data(), fftSize);
    }

    vector<float> getFT(AudioBuffer<float>& input, int ch) {
        process(input, ch);
        result = getResult();
        if (decimated)
            result_decim = conv.mXv_mult(fbank_values, result);

        drawNextFrameOfSpectrum();

        return decimated ? result_decim : result;
    }


    void drawNextFrameOfSpectrum()
    {
        freqs_Content = conv.interpolateYvector(F, result, frequencies, false);

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


    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds, juce::Colour colour)
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
                          xVal[1], juce::jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f)});
        }
    }


    void setFBank(FilterBank fb) {
        fbank_values = fb.getValues();
        decimated = true;
    }

    vector<float> result_decim;

private:

    int scopeSize = npoints;
    vector<float> scopeData;
    vector<float> freqs_Content;
    vector<float> freqs_Error;

    vector<vector<float>> fbank_values;
    Converter conv;
    bool decimated = false;
    vector<float> F;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT) };

