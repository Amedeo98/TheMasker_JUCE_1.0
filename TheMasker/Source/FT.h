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
    FT() : Analyser(_fftOrder, _fftSize)
    {}
    ~FT() {}

    void prepare(array<float,npoints> freqs, array<float,nfilts> fCents, int sampleRate, juce::Colour colour) {
        result_decim.resize(nfilts);
        result_fixed.resize(npoints);
        frequencies = freqs;
        //fCenters.resize(nfilts);
        fCenters = fCents;
        F.resize(fftSize);
        F = conv.linspace(1.0f, static_cast<float>(sampleRate / 2), static_cast<float>(fftSize));
        spectrumDrawer.prepareToPlay(frequencies.data(), colour);
        interp = true;
    }

    void getFT(AudioBuffer<float>& input, int ch, vector<float>& output) {
        process(input, ch);
        getResult(result);

        spectrumDrawer.drawNextFrameOfSpectrum(result);


        if (interp)
        conv.interpolateYvector(F, result, frequencies, true, result_fixed);
        else
        result_fixed = result;



        if (decimated)
            conv.mXv_mult(fbank_values, result_fixed, result_fixed.size(), result_decim);



        output = decimated ? result_decim : result_fixed;
    }

    
    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) {
        spectrumDrawer.drawFrame(g, bounds);
    }

    void setFBank(FilterBank fb) {
        fb.getValues(fbank_values);
        decimated = true;
    }

    vector<float> result_decim;
    vector<float> result_fixed;

private:

    array<float,nfilts> fCenters;
    bool interp;
    array<array<float,npoints>,nfilts> fbank_values;
    Converter conv;
    bool decimated = false;
    vector<float> F;

    SpectrumDrawer spectrumDrawer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT) 
};

