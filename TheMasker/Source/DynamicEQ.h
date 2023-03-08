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
#include "Plotter.h"



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
        numSamples = samplesPerBlock;

        numInChannels = inCh;
        if (scCh > 0)
            numScChannels = scCh;
        else
            numScChannels = numInChannels;

        frequencies = _frequencies;
        fbank.getFilterBank(frequencies.data());
        fbank.getFrequencies(fCenters);

        curves.resize(inCh);    
        for (int i = 0; i < numScChannels; ++i) {
            curves[i].delta.resize(nfilts);
            curves[i].threshold.resize(nfilts);
            curves[i].inSpectrum.resize(npoints);
            curves[i].scSpectrum.resize(npoints);
            curves[i].outSpectrum.resize(npoints);
        }

        deltaGetter.prepareToPlay(sampleRate, samplesPerBlock, fbank, fCenters.data(), frequencies.data(), numInChannels, numScChannels);
        deltaScaler.prepareToPlay(numScChannels);

        bufferDelayer.prepareToPlay(samplesPerBlock, inCh, _fftSize);
        filters.prepareToPlay(sampleRate, samplesPerBlock, numInChannels, numScChannels, fCenters.data());

        stereoLinked.setSL(stereoLinkAmt);
        setInGain(DEFAULT_IN);
        deltaGetter.setATQ(DEFAULT_ATQ);

        spectrumPlotter.prepareToPlay(frequencies.data(), fCenters.data());
        ft_out.prepare(frequencies.data(), fCenters.data(), sampleRate);
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
        mainBuffer.applyGain(outGain * _outExtraGain);

       
        for (int i = 0; i<2; i++)
        ft_out.getFT(mainBuffer, i, curves[i].outSpectrum, curves[i].outSpectrum);

        spectrumPlotter.drawNextFrameOfSpectrum(curves[0].inSpectrum, curves[0].scSpectrum, curves[0].outSpectrum, curves[0].delta);
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
        spectrumPlotter.drawFrame(g, bounds);
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


    int fs;
    int numInChannels = 2;
    int numScChannels;
    int numSamples;

    struct result
    {
        vector<float> delta;
        vector<float> threshold;
        vector<float> inSpectrum;
        vector<float> scSpectrum;
        vector<float> outSpectrum;
    };

    array<float, npoints> frequencies;
    array<float, nfilts> fCenters;

    vector<result> curves;

    FilterBank fbank;
    StereoLinked stereoLinked;
    DeltaGetter deltaGetter;
    DeltaScaler deltaScaler;
    MultiBandMod filters;
    BufferDelayer bufferDelayer;
    Plotter spectrumPlotter;

    FT ft_out;

    

};
