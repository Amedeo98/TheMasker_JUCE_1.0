/*
  ==============================================================================

    FT.h
    Created: 20 Jan 2023 6:17:33pm
    Author:  nikid

  ==============================================================================
*/

#include "Analyser.h"
#include "SpectrumDrawer.h"


#pragma once
class FT : public Analyser {

public:
    FT() : Analyser(_fftOrder, npoints)
    {}
    ~FT() {}

    void prepare(vector<float>_frequencies, int sampleRate, juce::Colour colour) {
        result_decim.resize(nfilts);
        frequencies = _frequencies;
        //scopeData.resize(scopeSize);
        freqs_Error.resize(fftSize);
        F.resize(fftSize);
        F = conv.linspace(1.0f, static_cast<float>(sampleRate / 2), static_cast<float>(fftSize));
        FloatVectorOperations::subtract(freqs_Error.data(), frequencies.data(), F.data(), fftSize);
        spectrumDrawer.setColour(colour);
    }

    void getFT(AudioBuffer<float>& input, int ch, vector<float>& output) {
        process(input, ch);
        getResult(result);
        if (decimated)
            conv.mXv_mult(fbank_values, result, result_decim);

        spectrumDrawer.drawNextFrameOfSpectrum(result);

        output = decimated ? result_decim : result;
    }

    
    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) {
        spectrumDrawer.drawFrame(g, bounds);
    }

    void setFBank(FilterBank fb) {
        fb.getValues(fbank_values);
        decimated = true;
    }

    vector<float> result_decim;

private:


    vector<float> freqs_Content;
    vector<float> freqs_Error;

    vector<vector<float>> fbank_values;
    Converter conv;
    bool decimated = false;
    vector<float> F;

    SpectrumDrawer spectrumDrawer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT) 
};

