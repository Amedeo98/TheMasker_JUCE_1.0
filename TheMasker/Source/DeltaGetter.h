/*
  ==============================================================================

    Curve.h
    Created: 14 Dec 2022 5:15:30pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include "Converters.h"
#include "FilterBank.h"
#include "PluginProcessor.h"
#include "DynamicEQ.h"
#include "FT.h"
#include "PSY.h"
#include "DeltaDrawer.h"




class DeltaGetter  {

public:
    void getDelta(AudioBuffer<float>& in, AudioBuffer<float>& sc, auto& deltas) {

        for (int i = 0; i < inCh; i++) {
            ft_in.getFT(in, i, inFT[i], deltas[i].inSpectrum);
            conv.magnitudeToDb(inFT[i]);
            deltas[i].threshold = inFT[i];

        }
        for (int i = 0; i < scCh; i++) {
            ft_sc.getFT(sc, i, scFT[i], deltas[i].scSpectrum);
            psy.spread(scFT[i]);
            conv.magnitudeToDb(scFT[i]);
            psy.compareWithAtq(scFT[i], current_atq);
        }

        for (int i = 0; i < jmax(inCh, scCh); i++) {
            difference(inFT[i], scFT[i], deltas[i].delta);
        }
    }

 

    void prepareToPlay(int sampleRate, int samplesPerBlock, FilterBank fb, float* fCenters, float* frequencies, int numInCh, int numScCh) {
        scFT.resize(numScCh, vector<float>(nfilts));
        inFT.resize(numInCh, vector<float>(nfilts));
        getATQ(fCenters, atq);
        psy.getSpreadingMtx();
        ft_in.prepare(frequencies, fCenters, sampleRate);
        ft_sc.prepare(frequencies, fCenters, sampleRate);
        ft_in.setFBank(fb);
        ft_sc.setFBank(fb);
        setNumChannels(numInCh, numScCh);


    }

    void setNumChannels(int _inCh, int _scCh) {
        scCh = _scCh;
        inCh = _inCh;
    }

    void setATQ(float UIatqWeight) {
        current_atq = atq;
        juce::FloatVectorOperations::multiply(current_atq.data(), UIatqWeight, nfilts);
        juce::FloatVectorOperations::multiply(current_atq.data(), atqLift, nfilts);
        juce::FloatVectorOperations::add(current_atq.data(), minDBFS, nfilts);
        FloatVectorOperations::clip(current_atq.data(), current_atq.data(), minDBFS, 0.0f, nfilts);
    }
    


private:

    FT ft_sc;
    FT ft_in;

    PSY psy; 
    Converter conv;
    vector<vector<float>> inFT, scFT;
    array<float,nfilts> current_atq, atq;

    int inCh;
    int scCh;

    float maxGain = _maxGain;
    int gateThresh = _gateThresh;
    int gateKnee = _gateKnee;
    float rel_thresh_lift = _relThreshLift;

    int minDBFS = _mindBFS;
    float atqLift = _atqLift;

    void difference(vector<float> input, vector<float> rel_thresh, vector<float>& output) {
        int size = nfilts;
        for (int i = 0; i < size; i++)
            output[i] = input[i] - (rel_thresh[i]+rel_thresh_lift);
    }

    void getATQ(float* f, array<float,nfilts>& dest)
    {
        for (int i = 0; i < nfilts; i++)
        {
            //   matlab function: absThresh=3.64*(f./1000).^-0.8-6.5*exp(-0.6*(f./1000-3.3).^2)+.00015*(f./1000).^4; % edited function (reduces the threshold in high freqs)
            dest[i] = 3.64 * pow((f[i] / 1000), -0.8) - 6.5 * exp(-0.6 * pow(f[i] / 1000 - 3.3, 2)) + 0.00015 * pow(f[i] / 1000, 4);
        }
        float minimum = FloatVectorOperations::findMinimum(dest.data(), dest.size());
        FloatVectorOperations::add(dest.data(), -minimum, dest.size());
    }

};
