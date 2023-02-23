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

    void prepareToPlay(double sampleRate, int newSamplesPerBlock, int numInChannels, int numScChannels, vector<float> fCenters) {
        fs = sampleRate;
        samplesPerBlock = newSamplesPerBlock;
        filters.resize(nfilts);
        freqs.resize(nfilts);
        numInCh = numInChannels;
        numScCh = numScChannels;
        gains_sm.resize(numScCh, vector<SmoothedValue<float, ValueSmoothingTypes::Linear>>(nfilts));
        gains.resize(numScCh, vector<float>(nfilts));
        old_gains.resize(numScCh, vector<float>(nfilts));
        smoothingSeconds = samplesPerBlock / sampleRate * smoothingWindow;
        rampSamples = (int) samplesPerBlock * smoothingWindow ;
        inputBuffer_copy.setSize(numInCh, samplesPerBlock);
        tempOutput.setSize(numInCh, samplesPerBlock);
        for (int i = 0; i < nfilts; i++) {
            freqs[i].fCenter = fCenters[i];
        }
        getBandFreqs();
        for (int i = 0; i < nfilts; i++) {
            filters[i].prepareToPlay(sampleRate, samplesPerBlock, freqs[i].f_lc, freqs[i].f_hc, numInCh);
            for (int ch = 0; ch < numScCh; ch++) {
                gains_sm[ch][i].reset(sampleRate, smoothingSeconds);
            }
        }
    }

    void setNumChannels(int inCh, int scCh) {
        numInCh = inCh;
        numScCh = scCh;
        inputBuffer_copy.setSize(numInCh, samplesPerBlock);
        tempOutput.setSize(numInCh, samplesPerBlock);
        valueRamp.setSize(numInCh, samplesPerBlock);
        gains_sm.resize(numScCh, vector<SmoothedValue<float, ValueSmoothingTypes::Linear>>(nfilts));
        gains.resize(numScCh, vector<float>(nfilts));
        old_gains.resize(numScCh, vector<float>(nfilts));
        for (int i = 0; i < nfilts; i++) {
            filters[i].setNumChannels(numInCh);
            for (int ch = 0; ch < numScCh; ch++) {
                gains_sm[ch][i].reset(fs, smoothingSeconds);
            }
        }

    }
    
    void filterBlock(AudioBuffer<float>& buffer, auto curves) {
        int numSamples = buffer.getNumSamples();
        inputBuffer_copy.clear();
        for (int ch = 0; ch < numInCh; ch++) 
        {
            inputBuffer_copy.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }
        buffer.clear();
        for (int f = 0; f < nfilts; f++) 
        {
            tempOutput.clear();
            valueRamp.clear();
            filters[f].process(inputBuffer_copy, tempOutput);
            
            for (int ch = 0; ch < numScCh; ch++) {
                /*old_gains[ch][f] = gains[ch][f];
                gains[ch][f] = Decibels::decibelsToGain(curves[ch].delta[f]);
                tempOutput.applyGainRamp(0, rampSamples, old_gains[ch][f], gains[ch][f]);*/
                //DBG("Curves:");
                //DBG(curves[ch].delta[f]);
                //DBG("Curves_gain:");

                //DBG(Decibels::decibelsToGain(curves[ch].delta[f]));

                gains_sm[ch][f].setTargetValue(Decibels::decibelsToGain(curves[ch].delta[f]));
                for (int sample = 0; sample < numSamples; sample++) {
                    //tempOutput.setSample(ch, sample, tempOutput.getSample(ch, sample) * gains_sm[ch][f].getNextValue());
                    valueRamp.setSample(ch, sample, gains_sm[ch][f].getNextValue());
                    
                }
                
                FloatVectorOperations::multiply(tempOutput.getWritePointer(ch), valueRamp.getReadPointer(ch), numSamples);


                buffer.addFrom(ch, 0, tempOutput, ch, 0, numSamples);
            }
            //for (auto i = numInCh; i < numScCh; ++i) {
            /*if (numInCh < 2) {
                buffer.addFrom(2, 0, tempOutput, 1, 0, numSamples);
            }*/
            
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
    int numInCh;
    int numScCh;
    int fs;
    int samplesPerBlock;
    float smoothingSeconds = 0.2f;
    int rampSamples;
    float smoothingWindow = 0.8f;
    vector<LinkwitzRileyFilters> filters;
    vector<vector<SmoothedValue<float, ValueSmoothingTypes::Linear>>> gains_sm;
    vector<vector<float>> gains;
    vector<vector<float>> old_gains;
    AudioBuffer<float> inputBuffer_copy;
    AudioBuffer<float> tempOutput;
    AudioBuffer<float> valueRamp;
    struct freq {float f_lc;  float fCenter; float f_hc; };
    vector<freq> freqs;


};
