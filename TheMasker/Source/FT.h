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
        frequencies = freqs;
        fCenters = fCents;
        F.resize(fftSize*0.5f);
        F = conv.linspace(0.0f, static_cast<float>(sampleRate * 0.5f), static_cast<float>(fftSize*0.5f));
    }

    void getFT(AudioBuffer<float>& input, int ch, auto& output, array<float,npoints>& spectrumOutput, bool& processFFTresult) {
        process(input, ch, processFFTresult);
        getResult(result);
        conv.interpolateYvector(F, result, frequencies, false, result_fixed);
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

    void setFBank(FilterBank& fb) {
        fb.getValues(fbank_values);
        decimated = true;
    }

    array<float, nfilts> result_decim;
    array<float, npoints> result_fixed;

private:

    float* fCenters;
    array<array<float, npoints>, nfilts> fbank_values;
    bool decimated = false;
    vector<float> F;

    Converter conv;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT)
};

