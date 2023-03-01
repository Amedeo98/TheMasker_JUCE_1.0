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



class DeltaGetter  {

public:
    void getDelta(AudioBuffer<float>& in, AudioBuffer<float>& sc, auto& deltas) {

        for (int i = 0; i < inCh; i++) {

            ft_in.getFT(in, i, inFT[i]);
            conv.toMagnitudeDb(inFT[i]);
            deltas[i].threshold = inFT[i];

        }
        for (int i = 0; i < scCh; i++) {

            ft_sc.getFT(sc, i, scFT[i]);
            psy.spread(scFT[i]);
            conv.toMagnitudeDb(scFT[i]);
            psy.compareWithAtq(scFT[i], current_atq);
        }

        for (int i = 0; i < jmax(inCh, scCh); i++) {
            difference(inFT[i], scFT[i], scFT[i]);
            deltas[i].delta = scFT[i];
        }

    }

 

    void prepareToPlay(int sampleRate, int samplesPerBlock, FilterBank fb, float atqW, array<float,nfilts> fCenters, array<float,npoints> frequencies, int numInCh, int numScCh) {
        scFT.resize(numScCh, vector<float>(nfilts));
        inFT.resize(numInCh, vector<float>(nfilts));
        //atq.resize(nfilts);
        //current_atq.resize(nfilts);
        //fCenters.resize(nfilts);
        getATQ(fCenters, atq);
        psy.getSpreadingMtx();
        ft_in.prepare(frequencies, fCenters, sampleRate, in_colour);
        ft_sc.prepare(frequencies, fCenters, sampleRate, sc_colour);
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
        ft_sc.drawFrame(g, bounds);
        ft_in.drawFrame(g, bounds);
    }


private:
    juce::Colour in_colour = Colour(0.5f, 1.0f, 1.0f, 1.0f);
    juce::Colour sc_colour = Colour(1.0f, 1.0f, 1.0f, 1.0f);
    FT ft_sc;
    FT ft_in;

    PSY psy; 
    Converter conv;

    vector<vector<float>> inFT, scFT;
    array<float,nfilts> current_atq, atq;

    array<float, nfilts> fCenters;

    float atqWeight;
    int inCh;
    int scCh;
    float rel_thresh_lift = 3.0f;

    size_t numChannels;

    float maxGain = 20;
    int gateThresh = -40;
    int gateKnee = 10;

    int minDBFS = -64;
    float atqLift = 1.6;

    void difference(vector<float> input, vector<float> rel_thresh, vector<float>& output) {
        int size = rel_thresh.size();
        for (int i = 0; i < size; i++)
            output[i] = input[i] - (rel_thresh[i]+rel_thresh_lift);
        //FloatVectorOperations::subtract(output.data(), input.data(), rel_thresh.data(), rel_thresh.size());
    }

    void getATQ(array<float,nfilts>& f, array<float,nfilts>& dest)
    {
        //dest.resize(nfilts);
        for (int i = 0; i < f.size(); i++)
        {
            //   matlab function: absThresh=3.64*(f./1000).^-0.8-6.5*exp(-0.6*(f./1000-3.3).^2)+.00015*(f./1000).^4; % edited function (reduces the threshold in high freqs)
            dest[i] = 3.64 * pow((f[i] / 1000), -0.8) - 6.5 * exp(-0.6 * pow(f[i] / 1000 - 3.3, 2)) + 0.00015 * pow(f[i] / 1000, 4);
        }
        float minimum = FloatVectorOperations::findMinimum(dest.data(), dest.size());
        FloatVectorOperations::add(dest.data(), -minimum, dest.size());

        /*juce::FloatVectorOperations::multiply(dest.data(), atqLift, nfilts);
        juce::FloatVectorOperations::add(dest.data(), minDBFS, nfilts);
        FloatVectorOperations::clip(dest.data(), dest.data(), minDBFS, 0.0f, nfilts);*/

    }

};
