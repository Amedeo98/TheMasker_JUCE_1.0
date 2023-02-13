/*
  ==============================================================================

    Curve.h
    Created: 14 Dec 2022 5:15:30pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Converters.h"
#include "FilterBank.h"
#include "PluginProcessor.h"
#include "DynamicEQ.h"
#include "FT.h"
#include "PSY.h"



class DeltaGetter  {

public:
    void getDelta(AudioBuffer<float>& in, AudioBuffer<float>& sc, auto& deltas) {

        for (int i = 0; i < inCh; i++) {

            inFT[i] = ft_in.getFT(in, i);
            conv.toMagnitudeDb(inFT[i]);
            deltas[i].threshold = inFT[i];

        }
        for (int i = 0; i < scCh; i++) {

            scFT[i] = ft_sc.getFT(sc, i);
            scFT[i] = psy.spread(scFT[i]);
            conv.toMagnitudeDb(scFT[i]);
            scFT[i] = psy.compareWithAtq(scFT[i], current_atq);
        }

        for (int i = 0; i < jmax(inCh, scCh); i++) {
            scFT[i] = difference(inFT[i], scFT[i]);
            deltas[i].delta = scFT[i];
        }

    }

 

    void prepareToPlay(int sampleRate, int samplesPerBlock, FilterBank fb, float atqW, vector<float> fCenters, int numInCh, int numScCh) {
        scFT.resize(numScCh, vector<float>(nfilts));
        inFT.resize(numInCh, vector<float>(nfilts));
        atq.resize(nfilts);
        current_atq.resize(nfilts);
        fCenters.resize(nfilts);
        atq = getATQ(fCenters);
        psy.getSpreadingMtx();
        ft_in.prepare(samplesPerBlock);
        ft_sc.prepare(samplesPerBlock);
        ft_in.setFBank(fb);
        ft_sc.setFBank(fb);
        setATQ(atqW);
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
    
    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds){
        ft_in.drawFrame(g, bounds);
    }


private:
    //Delta properties
    FT ft_sc;
    FT ft_in;

    PSY psy; 
    Converter conv;

    vector<vector<float>> inFT, scFT;
    vector<float> current_atq, atq, fCenters;

    float atqWeight;
    int inCh;
    int scCh;


    size_t numChannels;

    //Delta properties
    float maxGain = 20;
    int gateThresh = -40;
    int gateKnee = 10;

    //ATQ properties
    int minDBFS = -64;
    float atqLift = 1.6;

    vector<float> difference(vector<float> input, vector<float> rel_thresh) {
        FloatVectorOperations::subtract(input.data(), rel_thresh.data(), rel_thresh.size());
        return input;

    }

    vector<float> getATQ(vector<float>& f)
    {
        vector<float> values(f.size());
        for (int i = 0; i < f.size(); i++)
        {
            //   matlab function: absThresh=3.64*(f./1000).^-0.8-6.5*exp(-0.6*(f./1000-3.3).^2)+.00015*(f./1000).^4; % edited function (reduces the threshold in high freqs)
            values[i] = 3.64 * pow((f[i] / 1000), -0.8) - 6.5 * exp(-0.6 * pow(f[i] / 1000 - 3.3, 2)) + 0.00015 * pow(f[i] / 1000, 4);
        }
        float minimum = FloatVectorOperations::findMinimum(values.data(), values.size());
        FloatVectorOperations::add(values.data(), -minimum, values.size());

        juce::FloatVectorOperations::multiply(values.data(), atqLift, nfilts);
        juce::FloatVectorOperations::add(values.data(), minDBFS, nfilts);
        FloatVectorOperations::clip(values.data(), values.data(), minDBFS, 0.0f, nfilts);
        return values;

    }

};
