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
        result_decim.resize(nfilts);
        result_fixed.resize(npoints);
        frequencies = freqs;
        fCenters = fCents;
        F.resize(fftSize);
        F = conv.linspace(0.0f, static_cast<float>(sampleRate / 2), static_cast<float>(fftSize));
        interp = true;
    }

    void getFT(AudioBuffer<float>& input, int ch, vector<float>& output, vector<float>& spectrumOutput) {
        process(input, ch);
        getResult(result);

        if (interp)
            conv.interpolateYvector(F, result, frequencies, false, result_fixed);
        else
            result_fixed = result;

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

    vector<float> result_decim;
    vector<float> result_fixed;

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

