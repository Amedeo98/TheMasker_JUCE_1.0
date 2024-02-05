/*
  ==============================================================================

    MultiBandMod.h
    Created: 20 Jan 2023 6:18:30pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Converters.h"
#include "LinkwitzRileyFilters.h"
#include "DynamicEQ.h"

#define MIDDLEBAND_OFF -0.32f
#define LPHP_OFF -17.64f
#define BPEXT_OFF -11.52f

class MultiBandMod {
public:
    MultiBandMod() {}
    ~MultiBandMod() {}

    void prepareToPlay(double sampleRate, int newSamplesPerBlock, float* fCenters) {
        fs = sampleRate;
        samplesPerBlock = newSamplesPerBlock;
        
        for (int i = 0; i < nfilts; i++) {
            freqs[i].fCenter = fCenters[i];
        }

        getBandFreqs();
        
        for (int i = 0; i < nfilts; i++) {
            filters[i].prepareToPlay(sampleRate, samplesPerBlock, freqs[i].f_lc, freqs[i].f_hc);
            gainAdjustments[i] = Decibels::decibelsToGain(MIDDLEBAND_OFF); //0.0; // (i % 2) ? -1.0f : 1.0f;
        }

        ///////////////// BOTTOM END

        setCorrectionGain(0, LPHP_OFF);
        
        // without allpass
        setCorrectionGain(1, -13.805f);
        setCorrectionGain(2, -3.464f);
        setCorrectionGain(3, -1.450f);
        setCorrectionGain(4, -0.650f);
        setCorrectionGain(5, -0.445f);

        ///////////////// TOP END

        setCorrectionGain(-6, -0.311f);
        setCorrectionGain(-5, -0.388f);

        // with allpass (see LinkwitzRileyFilters.h lines 43-44
        //setCorrectionGain(-4, -0.621f);
        //setCorrectionGain(-3, -1.320f);
        //setCorrectionGain(-2, -4.272f);

        // without allpass
        setCorrectionGain(-4, -0.627f);
        setCorrectionGain(-3, -1.412f);
        setCorrectionGain(-2, -3.922f);

        setCorrectionGain(-1, LPHP_OFF);
    }

    void setCorrectionGain(int fIndex, float gainDb, bool invert = false)
    {
        if (fIndex < 0)
            fIndex += nfilts;

        jassert(fIndex < nfilts);
        jassert(fIndex >= 0);

        gainAdjustments[fIndex] = invert ? -Decibels::decibelsToGain(gainDb) : Decibels::decibelsToGain(gainDb);
    }

    float getCorrectionGain(int fIndex)
    {
        if (fIndex < 0)
            fIndex += nfilts;

        jassert(fIndex < nfilts);
        jassert(fIndex >= 0);

        return Decibels::gainToDecibels(gainAdjustments[fIndex]);
    }

    void setNumChannels(int nCh) {
        numCh = nCh;
        inputBuffer_copy.setSize(numCh, samplesPerBlock);
        tempOutput.setSize(numCh, samplesPerBlock);
        for (int i = 0; i < nfilts; i++) {
            filters[i].setNumChannels(numCh);
        }

    }
    
    void filterBlock(AudioBuffer<float>& buffer, auto& curves, auto& gains_sm, bool& processFFTresult) {
        int numSamples = buffer.getNumSamples();
        //inputBuffer_copy.clear();
        inputBuffer_copy.setSize(numCh, numSamples, false, true, true); // This is soooo bad

        for (int ch = 0; ch < numCh; ch++) 
        {
            inputBuffer_copy.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }

        buffer.clear();

        for (int f = 0; f < nfilts; f++) 
        {
            tempOutput.clear();
            tempOutput.setSize(numCh, numSamples);
            filters[f].process(inputBuffer_copy, tempOutput);
            
            for (int ch = 0; ch < numCh; ++ch) {

                if (processFFTresult)
                    gains_sm[ch][f].setTargetValue(Decibels::decibelsToGain(curves[ch].delta[f]));

                for (int sample = 0; sample < numSamples; ++sample) {
                    tempOutput.setSample(ch, sample, tempOutput.getSample(ch, sample) * gains_sm[ch][f].getNextValue());
                }

                buffer.addFrom(ch, 0, tempOutput, ch, 0, numSamples, gainAdjustments[f]);

            }
            
        }
    }
    

    void getBandFreqs() {
        freqs[0].f_lc = minFreq;
        freqs[0].f_hc = (freqs[0].fCenter + freqs[1].fCenter) * 0.5f;
        for (int i = 1; i < nfilts-1; i++) {
            freqs[i].f_lc = freqs[i - 1].f_hc;
            freqs[i].f_hc = (freqs[i].fCenter + freqs[i + 1].fCenter) *0.5f ;
        }
        freqs[nfilts-1].f_lc = freqs[nfilts - 2].f_hc;
        freqs[nfilts-1].f_hc = maxFreq;
    }


private:

    int numCh = 0;
    double fs;
    int samplesPerBlock;

    array<LinkwitzRileyFilters, nfilts> filters;
    AudioBuffer<float> inputBuffer_copy;
    AudioBuffer<float> tempOutput;

    float gainAdjustments[nfilts];

    struct freq
    {
        float f_lc;
        float fCenter;
        float f_hc;
    };

    array<freq, nfilts> freqs;


};
