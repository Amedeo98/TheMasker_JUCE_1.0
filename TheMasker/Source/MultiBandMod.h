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
            gainAdjustments[i] = 1.0f;
        }

        gainAdjustments[0] = 0.1f; 
        gainAdjustments[1] = 0.15f;
        gainAdjustments[2] = 0.5f;
        //gainAdjustments[3] = 0.8f;
        //gainAdjustments[nfilts-7] = 0.95f;
        //gainAdjustments[nfilts-6] = 0.6f;
        //gainAdjustments[nfilts-5] = 0.7f;
        gainAdjustments[nfilts-4] = 0.85f;
        gainAdjustments[nfilts-3] = 0.8f;
        gainAdjustments[nfilts-2] = 0.55f;
        gainAdjustments[nfilts-1] = 0.2f;
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
        inputBuffer_copy.clear();
        inputBuffer_copy.setSize(numCh, numSamples);
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
            
            for (int ch = 0; ch < numCh; ch++) {

                if (processFFTresult)
                gains_sm[ch][f].setTargetValue(Decibels::decibelsToGain(curves[ch].delta[f]));

                for (int sample = 0; sample < numSamples; sample++) {
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
