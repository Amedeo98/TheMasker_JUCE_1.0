/*
  ==============================================================================

    Curve.h
    Created: 14 Dec 2022 5:15:30pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include "Converters.h"
#include "Constants.h"
#include "FilterBank.h"
//#include "PluginProcessor.h"
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
            FloatVectorOperations::copy(deltas[i].inputDecimated.data(), inFT[i].data(), nfilts);
        }

        if (inCh < maxCh) {
            FloatVectorOperations::copy(deltas[1].inSpectrum.data(), deltas[0].inSpectrum.data(), npoints);
            FloatVectorOperations::copy(deltas[1].inputDecimated.data(), deltas[0].inputDecimated.data(), nfilts);
        }


        for (int i = 0; i < scCh; i++) {
            ft_sc.getFT(sc, i, scFT[i], deltas[i].scSpectrum);
            FloatVectorOperations::copy(deltas[i].scDecimated.data(), scFT[i].data(), nfilts);
            psy.spread(scFT[i]);
            conv.magnitudeToDb(scFT[i]);
            psy.compareWithAtq(scFT[i], current_atq);
        }

        if (scCh < maxCh) 
        {
            FloatVectorOperations::copy(deltas[1].scSpectrum.data(), deltas[0].scSpectrum.data(), npoints);
            FloatVectorOperations::copy(deltas[1].scDecimated.data(), deltas[0].scDecimated.data(), npoints);
            FloatVectorOperations::copy(scFT[1].data(), scFT[0].data(), nfilts);
        }

        for (int i = 0; i < maxCh; i++) {
            difference(deltas[i].inputDecimated, scFT[i], deltas[i].delta);
        }

    }

 

    void prepareToPlay(int sampleRate, int samplesPerBlock, FilterBank& fb, float* fCenters, float* frequencies) {
        getATQ(fCenters, atq);
        setATQ(DEFAULT_ATQ);
        psy.getSpreadingMtx();
        ft_in.prepare(frequencies, fCenters, sampleRate);
        ft_sc.prepare(frequencies, fCenters, sampleRate);
        ft_in.setFBank(fb);
        ft_sc.setFBank(fb);
    }

    void setNumChannels(int _inCh, int _scCh, int _maxCh) {
        scCh = _scCh;
        inCh = _inCh;
        maxCh = _maxCh;
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
    array<array<float, nfilts>, 2> inFT, scFT;
    array<float,nfilts> current_atq, atq;

    int inCh = 0;
    int scCh = 0;
    int maxCh = 0;

    float maxGain = _maxGain;
    int gateThresh = _gateThresh;
    int gateKnee = _gateKnee;
    float rel_thresh_lift = _relThreshLift;

    int minDBFS = _mindBFS;
    float atqLift = _atqLift;

    void difference(array<float, nfilts>& input, array<float, nfilts>& rel_thresh, array<float, nfilts>& output) {
        for (int i = 0; i < nfilts; i++)
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
