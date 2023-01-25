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
#include "Delta.h"
#include "FilterBank.h"
#include "Converters.h"
#include "FilterCascade.h"
#include "Analyser.h"
#include "StereoLinked.h"
#include "DeltaScaler.h"



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



    void prepareToPlay(vector<float> _frequencies, int sampleRate, int inCh, int outCh, int samplesPerBlock, Converter converter)
    {
        fs = sampleRate;
        numInChannels = inCh;
        numOutChannels = outCh;
        frequencies = _frequencies;
        fbank.getFilterBank(frequencies);
        fCenters = fbank.getFrequencies();
        deltaGetter_L.prepareToPlay(sampleRate, samplesPerBlock, fbank, atqWeight, fCenters);
        deltaScaler_L.prepareToPlay();
        delta_L.resize(nfilts);
        //filters.prepareToPlay(sampleRate, samplesPerBlock, numInChannels, fCenters);

    }

    void releaseResources()
    {

    }

    void processBlock(AudioBuffer<float>& mainBuffer, AudioBuffer<float>& scBuffer)
    {

        mainBuffer.applyGain(inGain);
        scBuffer.applyGain(scGain);

        auto [delta_L,threshold_L] = deltaGetter_L.getDelta(mainBuffer, scBuffer, 0);

        //auto [delta_L, delta_R] = stereoLinked.process(delta_L, delta_R);

        delta_L = deltaScaler_L.scale(delta_L, compAmount, expAmount, mixAmount);
        delta_L = deltaScaler_L.clip(delta_L, threshold_L);


        juce::dsp::AudioBlock<float>              ioBuffer(mainBuffer);
        juce::dsp::ProcessContextReplacing<float> context(ioBuffer);


        //filters.updateGains(delta.yValues);
        //filters.filterBlock(context);

        mainBuffer.applyGain(outGain);

        //if (getActiveEditor() != nullptr)
        //outSpectrum.processBlock(mainBuffer, 0, numOutChannels);
    }



    void setComp(float newValue) {
        compAmount = newValue;
    }

    void setExp(float newValue) {
        expAmount = newValue;
    }

    void setAtq(float newValue) {
        atqWeight = newValue;
        deltaGetter_L.setATQ(atqWeight);
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
        deltaGetter_L.drawFrame(g, bounds);
    }

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
    int numOutChannels = 2;
    vector<vector<float>> spreadingMtx;
    FilterBank fbank;
    Converter conv;


    vector<float> delta_L, threshold_L;


    StereoLinked stereoLinked;
    Delta deltaGetter_L;
    DeltaScaler deltaScaler_L;


    FilterCascade filters;



  

};
