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

    void prepare(float* freqs, float* fCents, int sampleRate) {
        //result_decim.resize(nfilts);
        //result_fixed.resize(npoints);
        frequencies = freqs;
        fCenters = fCents;
        F.resize(fftSize);
        F = conv.linspace(0.0f, static_cast<float>(sampleRate / 2), static_cast<float>(fftSize));
    }

    void getFT(AudioBuffer<float>& input, int ch, auto& output, array<float,npoints>& spectrumOutput) {
        process(input, ch);
        getResult(result);
        conv.interpolateYarray(F, result, frequencies, false, result_fixed);
        FloatVectorOperations::fill(output.data(), 0.0f, decimated ? nfilts : npoints);
        if (decimated)
        {
            conv.mXv_mult(fbank_values, result_fixed, npoints, output);
        }
        else {
            FloatVectorOperations::copy(output.data(), result_fixed.data(), npoints);
        }
        FloatVectorOperations::copy(spectrumOutput.data(), result_fixed.data(), npoints);
    }


    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) {
        spectrumDrawer.drawFrame(g, bounds);
    }

    void setFBank(FilterBank fb) {
        fb.getValues(fbank_values);
        decimated = true;
    }

    array<float, nfilts> result_decim;
    array<float, npoints> result_fixed;

private:

    float* fCenters;
    bool interp;
    array<array<float, npoints>, nfilts> fbank_values;
    bool decimated = false;
    vector<float> F;

    Converter conv;
    SpectrumDrawer spectrumDrawer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT)
};

