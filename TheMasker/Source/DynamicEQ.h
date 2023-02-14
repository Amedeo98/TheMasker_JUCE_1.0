/*
  ==============================================================================

    DynamicEQ.h
    Created: 14 Dec 2022 5:47:47pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
using namespace std;

//#include <FilterBank.h>
#include <JuceHeader.h>
#include "DeltaGetter.h"
#include "FilterBank.h"
#include "Converters.h"
#include "FilterCascade.h"
#include "Analyser.h"
#include "StereoLinked.h"
#include "DeltaScaler.h"
#include "MultiBandMod.h"



#define DEFAULT_COMP 0.0f
#define DEFAULT_EXP 0.0f
#define DEFAULT_ATQ 0.0f
#define DEFAULT_SL 0.0f
#define DEFAULT_MIX 1.0f
#define DEFAULT_IN 0.0f
#define DEFAULT_OUT 0.0f
#define DEFAULT_SC 0.0f



class DynamicEQ {
public:
    
    DynamicEQ() {}
    ~DynamicEQ() {}



    void prepareToPlay(vector<float> _frequencies, int sampleRate, int inCh, int scCh, int samplesPerBlock, Converter converter)
    {
        fs = sampleRate;
        numInChannels = inCh;
        numSamples = samplesPerBlock;
        if (scCh > 0)
            numScChannels = scCh;
        else
            numScChannels = numInChannels;

        frequencies = _frequencies;
        fbank.getFilterBank(frequencies);
        fCenters.resize(nfilts);
        fCenters = fbank.getFrequencies();

        curves.resize(inCh);            
        deltaGetter.prepareToPlay(sampleRate, samplesPerBlock, fbank, DEFAULT_ATQ, fCenters, frequencies, numInChannels, numScChannels);
        deltaScaler.prepareToPlay(numScChannels);

        for (int i = 0; i < numScChannels; ++i) {
            curves[i].delta.resize(nfilts);
            curves[i].threshold.resize(nfilts);
        }

        stereoLinked.prepareToPlay();

        filters.prepareToPlay(sampleRate, samplesPerBlock, numInChannels, numScChannels, fCenters);

    }

    void numChannelsChanged(int inCh, int scCh) {
        numInChannels = inCh;
        numScChannels = scCh;
        deltaGetter.setNumChannels(numInChannels, numScChannels);
        deltaScaler.setNumChannels(numScChannels);
        filters.setNumChannels(numInChannels, numScChannels);
    }

    void releaseResources()
    {

    }

    void processBlock(AudioBuffer<float>& mainBuffer, AudioBuffer<float>& scBuffer)
    {

        mainBuffer.applyGain(inGain);
        scBuffer.applyGain(scGain);


        deltaGetter.getDelta(mainBuffer, scBuffer, curves);

        if (numScChannels == 2)
        {
            stereoLinked.process(curves[0].delta, curves[1].delta);

        }

        deltaScaler.scale(curves, compAmount, expAmount, mixAmount);
        deltaScaler.clip(curves);
        filters.filterBlock(mainBuffer, curves);
        mainBuffer.applyGain(outGain);

    }



    void setComp(float newValue) {
        compAmount = newValue;
    }

    void setExp(float newValue) {
        expAmount = newValue;
    }

    void setAtq(float newValue) {
        atqWeight = newValue;
        //deltaGetter.setATQ(atqWeight);
    }

    void setStereoLinked(float newValue) {
        stereoLinkAmt = newValue;
        stereoLinked.setSL(stereoLinkAmt);
    }

    void setMix(float newValue) {
        mixAmount = newValue;
    }

    void setInGain(float newValue) {
        inGain = Decibels::decibelsToGain(newValue);
    }

    void setOutGain(float newValue) {
        outGain = Decibels::decibelsToGain(newValue);
    }

    void setScGain(float newValue) {
        scGain = Decibels::decibelsToGain(newValue);
    }

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        deltaGetter.drawFrame(g, bounds);
    }

    struct result { vector<float> delta;  vector<float> threshold; };


private:

    float compAmount = DEFAULT_COMP;
    float expAmount = DEFAULT_EXP;
    float atqWeight = DEFAULT_ATQ;
    float stereoLinkAmt = DEFAULT_SL;
    float mixAmount = DEFAULT_MIX;
    float inGain = Decibels::decibelsToGain(DEFAULT_IN);
    float outGain = Decibels::decibelsToGain(DEFAULT_OUT);
    float scGain = Decibels::decibelsToGain(DEFAULT_SC);

    vector<float> frequencies;
    vector<float> fCenters;
    int fs = 0;
    int numInChannels = 2;
    int numScChannels;
    vector<vector<float>> spreadingMtx;
    FilterBank fbank;
    Converter conv;
    int numSamples = 0;
    vector<result> curves;

    StereoLinked stereoLinked;
    DeltaGetter deltaGetter;
    DeltaScaler deltaScaler;


    MultiBandMod filters;



  

};
