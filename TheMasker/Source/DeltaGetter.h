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
    void getDelta(AudioBuffer<float>& in, AudioBuffer<float>& sc, auto& deltas, bool& processFFTresult) {

        for (int i = 0; i < inCh; i++) {
            ft_in.getFT(in, i, inFT[i], deltas[i].inSpectrum, processFFTresult);
            if (processFFTresult) {
                conv.magnitudeToDb(inFT[i]);
                FloatVectorOperations::copy(deltas[i].inputDecimated.data(), inFT[i].data(), nfilts);
                FloatVectorOperations::add(deltas[i].inputDecimated.data(), -6.0f, nfilts);
            }
        }

        if (processFFTresult) {
            if (inCh < maxCh) {
                FloatVectorOperations::copy(deltas[1].inSpectrum.data(), deltas[0].inSpectrum.data(), npoints);
                FloatVectorOperations::copy(deltas[1].inputDecimated.data(), deltas[0].inputDecimated.data(), nfilts);
            }
        }

        for (int i = 0; i < scCh; i++) {

            ft_sc.getFT(sc, i, scFT[i], deltas[i].scSpectrum, processFFTresult);
            if (processFFTresult) {
                psy.spread(scFT[i]);
                conv.magnitudeToDb(scFT[i]); // Is it right to do dB after psy?
                //compareWithAtq(scFT[i], current_atq);
                FloatVectorOperations::add(scFT[i].data(), rel_thresh_lift, nfilts); // add here instead of in difference() so to be plotted correctly
                FloatVectorOperations::copy(deltas[i].scDecimated.data(), scFT[i].data(), nfilts);
            }
        }

        if (processFFTresult) {

            if (scCh < maxCh)
            {
                FloatVectorOperations::copy(deltas[1].scSpectrum.data(), deltas[0].scSpectrum.data(), npoints);
                FloatVectorOperations::copy(deltas[1].scDecimated.data(), deltas[0].scDecimated.data(), npoints);
                //FloatVectorOperations::copy(scFT[1].data(), scFT[0].data(), nfilts);
            }

            for (int i = 0; i < maxCh; i++) {
                difference(deltas[i].inputDecimated, deltas[i].scDecimated, deltas[i].delta);
            }

        }

    }

 

    void prepareToPlay(float sampleRate, FilterBank& fb, float* fCenters, float* frequencies) {
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

    


private:

    FT ft_sc;
    FT ft_in;

    PSY psy; 
    Converter conv;
    array<array<float, nfilts>, 2> inFT, scFT;

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
            output[i] = input[i] - (rel_thresh[i]+_relThreshBoost);
            //output[i] = input[i] - rel_thresh[i];
    }

   

};
