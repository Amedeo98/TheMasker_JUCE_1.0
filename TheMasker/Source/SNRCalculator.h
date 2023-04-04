/*
  ==============================================================================

    SNRCalculator.h
    Created: 4 Apr 2023 4:33:57pm
    Author:  nikid

  ==============================================================================
*/
#include <JuceHeader.h>

#pragma once
class SNRCalculator {
public:
    void prepare(int ch, int dim) {
        numCh = ch;
        dimension = dim;
        signal.setSize(numCh, dimension);
        error.setSize(numCh, dimension);
    }

    void pushInput(AudioBuffer<float> in) {
        signal.clear();
        for (int ch = 0; ch < numCh; ch++) {
            signal.copyFrom(ch, 0, in.getReadPointer(ch), dimension);
        }
    } 
    
    void pushOutput(AudioBuffer<float> out) {
        error.clear();
        for (int ch = 0; ch < numCh; ch++) {
            for (int i = 0; i < dimension; i++) {
                error.setSample(ch, i, signal.getSample(ch, i) - out.getSample(ch, i));
            }
            //error.copyFrom(ch, 0, signal.getReadPointer(ch), dimension);
            //error.copyFrom(ch, 0, out.getReadPointer(ch), dimension, -1.0f);
        }
    }

    void calculateSNR() {
        float signalPow = 0.0f, noisePow = 0.0f, meanSnr = 0.0f;
        signalPow = pow(rssq(signal), 2);
        noisePow = pow(rssq(error), 2);
        snr = 10 * log10f(signalPow / noisePow) / dimension;

        DBG("SNR current:");
        DBG(snr);
        DBG(" \n");

        lastSnrValues.insert(lastSnrValues.begin(), snr);
        int size = min((int) lastSnrValues.size(), maxMeanDim);
        
        for (int i = 0; i < size; i++) {
            meanSnr += lastSnrValues[i] / size;
        }
        DBG("mean SNR:");
        DBG(meanSnr);
        DBG(" \n");

    }

    

private:
    AudioBuffer<float> signal;
    AudioBuffer<float> error;
    int dimension;
    int numCh;
    int maxMeanDim = 2000;
    float snr;
    vector<float> lastSnrValues;

    float rssq(AudioBuffer<float> signal) {

        float signalPow = 0.0f;

        for (int ch = 0; ch < numCh; ch++)
            for (int i = 0; i < dimension; i++) {
                signalPow += pow(abs(signal.getSample(ch, i)), 2);
            }
        return pow(signalPow, 0.5f);
    }
};