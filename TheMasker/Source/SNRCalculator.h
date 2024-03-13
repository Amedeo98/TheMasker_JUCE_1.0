/*
  ==============================================================================

    SNRCalculator.h
    Created: 4 Apr 2023 4:33:57pm
    Author:  nikid

  ==============================================================================
*/
#include <JuceHeader.h>
using namespace std;

#pragma once
class SNRCalculator {
public:
    void prepare(int ch, int maxDim) {
        numCh = ch;
        maxDimension = maxDim;
        inSignal.resize(numCh);
        outSignal.resize(numCh);
        error.resize(numCh);
        for (int ch = 0; ch < numCh; ch++) {
            inSignal[ch].clear();
            outSignal[ch].clear();
            error[ch].clear();
        }
    }

    void pushInput(AudioBuffer<float> in) {
        //signal.clear();
        for (int ch = 0; ch < numCh; ch++) {
            for (int sample = 0; sample < in.getNumSamples(); sample++) {
                inSignal[ch].push_back(in.getSample(ch, sample));
            }
            error[ch].resize(currentDimension + in.getNumSamples());

        }
        currentDimension += in.getNumSamples();


    }

    void pushOutput(AudioBuffer<float> out) {
        for (int ch = 0; ch < numCh; ch++) {
            for (int sample = 0; sample < out.getNumSamples(); sample++) {
                outSignal[ch].push_back(out.getSample(ch, sample));
            }
        }
        //error.copyFrom(ch, 0, signal.getReadPointer(ch), dimension);
        //error.copyFrom(ch, 0, out.getReadPointer(ch), dimension, -1.0f);
    }

    void calculateSNR() {

        float signalPow = 0.0f, noisePow = 0.0f, meanSnr = 0.0f;

        //error.clear();
        for (int ch = 0; ch < numCh; ch++) {
            FloatVectorOperations::subtract(error[ch].data(), inSignal[ch].data(), outSignal[ch].data(), currentDimension);
        }
        signalPow = rssq(inSignal);
        noisePow = rssq(error);
        snr = 10 * log10f(signalPow / noisePow);

        DBG("SNR current:");
        DBG(snr);
        DBG(" \n");

        lastSnrValues.insert(lastSnrValues.begin(), snr);
        int size = min((int)lastSnrValues.size(), maxMeanDim);

        for (int i = 0; i < size; i++) {
            meanSnr += lastSnrValues[i] / size;
        }
        DBG("mean SNR:");
        DBG(meanSnr);
        DBG(" \n");

    }


    void generateNoise(AudioBuffer<float>& buffer) {
        buffer.clear();
        for (int ch = 0; ch < numCh; ch++) {
            for (int i = 0; i < buffer.getNumSamples(); i++) {
                const auto x = 2.0f * (rand.nextFloat() - 0.5f) * 0.5f;
                buffer.setSample(ch, i, x);
            }
        }
    }


private:
    std::vector<std::vector<float>> inSignal;
    vector<vector<float>> outSignal;
    vector<vector<float>> error;
    int currentDimension = 0;
    int maxDimension;
    int numCh;
    int maxMeanDim = 2000;
    float snr;
    vector<float> lastSnrValues;
    Random rand;


    float rssq(vector<vector<float>> signal) {

        float signalPow = 0.0f;

        for (int ch = 0; ch < numCh; ch++)
            for (int i = 0; i < min((int)currentDimension, maxDimension); i++) {
                signalPow += pow(signal[ch][i], 2);
            }
        return signalPow;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SNRCalculator)
};