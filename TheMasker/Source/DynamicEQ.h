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
#include "Analyser.h"
#include "StereoLinked.h"
#include "DeltaScaler.h"
#include "MultiBandMod.h"
#include "BufferDelayer.h"
#include "DeltaDrawer.h"



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



    void prepareToPlay(array<float,npoints> _frequencies, int sampleRate, int inCh, int scCh, int samplesPerBlock)
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
        fbank.getFrequencies(fCenters);

        curves.resize(inCh);            
        deltaGetter.prepareToPlay(sampleRate, samplesPerBlock, fbank, DEFAULT_ATQ, fCenters, frequencies, numInChannels, numScChannels);
        deltaScaler.prepareToPlay(numScChannels);

        for (int i = 0; i < numScChannels; ++i) {
            curves[i].delta.resize(nfilts);
            curves[i].threshold.resize(nfilts);
        }

        filters.prepareToPlay(sampleRate, samplesPerBlock, numInChannels, numScChannels, fCenters);

        outFT.resize(2, vector<float>(_fftSize));
        ft_out.prepare(frequencies, fCenters, sampleRate, out_colour);
        stereoLinked.setSL(stereoLinkAmt);
        bufferDelayer.prepareToPlay(samplesPerBlock, inCh, _fftSize);
        setInGain(DEFAULT_IN);
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

        if (numScChannels == 2 && stereoLinkAmt > 0.0f)
        {
            stereoLinked.process(curves[0].delta, curves[1].delta);
        }

        deltaScaler.scale(curves, compAmount, expAmount, mixAmount);
        deltaScaler.clip(curves);
        bufferDelayer.delayBuffer(mainBuffer);
        filters.filterBlock(mainBuffer, curves);
        mainBuffer.applyGain(outGain+_outExtraGain);

       
        for (int i = 0; i<2; i++)
        ft_out.getFT(mainBuffer, i, outFT[i]);


    }



    void setComp(float newValue) {
        compAmount = newValue;
    }

    void setExp(float newValue) {
        expAmount = newValue;
    }

    void setAtq(float newValue) {
        atqWeight = newValue;
        deltaGetter.setATQ(atqWeight);
    }

    void setStereoLinked(float newValue) {
        stereoLinkAmt = newValue;
        stereoLinked.setSL(stereoLinkAmt);
    }

    void setMix(float newValue) {
        mixAmount = newValue;
    }

    void setInGain(float newInValue) {
        inGain = Decibels::decibelsToGain(newInValue);
    }

    void setOutGain(float newOutValue) {
        outGain = Decibels::decibelsToGain(newOutValue);
    }

    void setScGain(float newScValue) {
        scGain = Decibels::decibelsToGain(newScValue);
    }

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        deltaGetter.drawFrame(g, bounds);
        ft_out.drawFrame(g, bounds);
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

    juce::Colour out_colour = Colour(0.3f, 1.0f, 1.0f, 1.0f);
    vector<vector<float>> outFT;

    array<float, npoints> frequencies;
    array<float, nfilts> fCenters;
    int fs = 0;
    int numInChannels = 2;
    int numScChannels;
    FilterBank fbank;
    int numSamples = 0;
    vector<result> curves;

    StereoLinked stereoLinked;
    DeltaGetter deltaGetter;
    DeltaScaler deltaScaler;
    FT ft_out;

    MultiBandMod filters;
    BufferDelayer bufferDelayer;



  

};
