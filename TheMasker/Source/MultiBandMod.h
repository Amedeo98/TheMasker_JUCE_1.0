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
#include "SimpleFilter.h"
#include "LinkwitzRileyFilters.h"
#include "DynamicEQ.h"



class MultiBandMod {
public:
    MultiBandMod() {}
    ~MultiBandMod() {}

    //using result = DynamicEQ::result;

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels, vector<float> fCenters) {

        filters.resize(nfilts);
        gains.resize(nfilts);
        freqs.resize(nfilts);
        numCh = numChannels;

        //smoothingSeconds = samplesPerBlock / sampleRate * smoothingWindow;
        inputBuffer_copy.setSize(1, samplesPerBlock);
        tempOutput.setSize(1, samplesPerBlock);
        for (int i = 0; i < nfilts; i++) {
            freqs[i].fCenter = fCenters[i];
        }
        getBandFreqs();
        for (int i = 0; i < nfilts; i++) {
            filters[i].prepareToPlay(sampleRate, samplesPerBlock, freqs[i].f_lc, freqs[i].f_hc);
            gains[i].reset(sampleRate, smoothingSeconds);
        }
    }


    
    void filterBlock(AudioBuffer<float>& buffer, auto curves) {
        int numSamples = buffer.getNumSamples();
        for (int ch = 0; ch < numCh; ch++) {
            inputBuffer_copy.clear();
            inputBuffer_copy.addFrom(0, 0, buffer.getReadPointer(ch), numSamples);
            buffer.clear(ch, 0, numSamples);
            for (int f = 0; f < nfilts; f++) {
                gains[f].setTargetValue(Decibels::decibelsToGain(curves[ch].delta[f]));
                //gains[f] = Decibels::decibelsToGain(delta[f]);
                tempOutput.clear();
                tempOutput = filters[f].process(inputBuffer_copy);
                for (int sample = 0; sample < numSamples; sample++) {
                    gains[f].getNextValue();
                    tempOutput.setSample(0, sample, tempOutput.getSample(0, sample) * gains[f].getCurrentValue());
                    //tempOutput.setSample(0, sample, tempOutput.getSample(0, sample) * gains[f]);
                }
                buffer.addFrom(ch, 0, tempOutput.getReadPointer(0), numSamples);
            }
        }



    }

    void getBandFreqs() {
        freqs[0].f_lc = minFreq;
        freqs[0].f_hc = (freqs[0].fCenter + freqs[1].fCenter) / 2;
        for (int i = 1; i < nfilts-1; i++) {
            freqs[i].f_lc = freqs[i - 1].f_hc;
            freqs[i].f_hc = (freqs[i].fCenter + freqs[i + 1].fCenter) / 2;
        }
        freqs[nfilts-1].f_lc = freqs[nfilts - 2].f_hc;
        freqs[nfilts-1].f_hc = maxFreq;
    }


private:
    int numCh;
    float smoothingSeconds = 0.02f;
    float smoothingWindow = 0.5f;
    vector<LinkwitzRileyFilters> filters;
    vector<SmoothedValue<float, ValueSmoothingTypes::Linear>> gains;
    //vector<float> gains;
    AudioBuffer<float> inputBuffer_copy;
    AudioBuffer<float> tempOutput;
    struct freq {float f_lc;  float fCenter; float f_hc; };
    vector<freq> freqs;


};
