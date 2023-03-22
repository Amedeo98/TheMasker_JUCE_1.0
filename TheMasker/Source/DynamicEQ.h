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
#include "VolumeMeter.h"



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

        //curves.resize(numInChannels);    

        //gains_sm.resize(numScChannels);
        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numScChannels; ch++) {
                gains_sm[ch][i].reset(fs, smoothingSeconds);
            }
        }

        deltaGetter.prepareToPlay(fs, numSamples, fbank, fCenters.data(), frequencies.data(), numInChannels, numScChannels);
        deltaScaler.prepareToPlay(numScChannels);

        bufferDelayer.prepareToPlay(numSamples, numInChannels, _fftSize, fs);
        filters.prepareToPlay(fs, numSamples, numInChannels, numScChannels, fCenters.data());

        stereoLinked.setSL(stereoLinkAmt);
        setInGain(DEFAULT_IN);
        deltaGetter.setATQ(DEFAULT_ATQ);

        spectrumPlotter.prepareToPlay(frequencies.data(), fCenters.data());
        ft_out.prepare(frequencies.data(), fCenters.data(), fs);
    }

    void numChannelsChanged(int inCh, int scCh) {
        numInChannels = inCh;
        numScChannels = scCh;
        deltaGetter.setNumChannels(numInChannels, numScChannels);
        deltaScaler.setNumChannels(numScChannels);
        filters.setNumChannels(numInChannels, numScChannels);
        //curves.resize(numScChannels);
        //gains_sm.resize(numScChannels);
        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numScChannels; ch++) {
                gains_sm[ch][i].reset(fs, smoothingSeconds);
            }
        }
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

        in_volumeMeter.setLevel(mainBuffer.getRMSLevel(0, 0, numSamples), mainBuffer.getRMSLevel(1, 0, numSamples));
        filters.filterBlock(mainBuffer, curves, gains_sm);
        mainBuffer.applyGain(outGain * _outExtraGain);
        out_volumeMeter.setLevel(mainBuffer.getRMSLevel(0, 0, numSamples), mainBuffer.getRMSLevel(1, 0, numSamples));

       
        for (int i = 0; i < 2; i++)
        ft_out.getFT(mainBuffer, i, curves[i].outSpectrum, curves[i].outSpectrum);

        spectrumPlotter.drawNextFrameOfSpectrum(curves[0].inSpectrum, curves[0].scSpectrum, curves[0].outSpectrum, gains_sm[0]);

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

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& responseBounds, juce::Rectangle<int>& inVolBounds, juce::Rectangle<int>& outVolBounds)
    {
        spectrumPlotter.drawFrame(g, responseBounds);
        in_volumeMeter.draw(g, inVolBounds);
        out_volumeMeter.draw(g, outVolBounds);
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

    struct curve
    {
        array<float, nfilts> delta;
        array<float, nfilts> threshold;
        array<float, npoints> inSpectrum;
        array<float, npoints> scSpectrum;
        array<float, npoints> outSpectrum;
    };

    array<float, npoints> frequencies;
    array<float, nfilts> fCenters;

    array<curve,2> curves;
    array<array<SmoothedValue<float, ValueSmoothingTypes::Linear>, nfilts>, 2> gains_sm;

    float smoothingSeconds = _smoothingSeconds;

    FilterBank fbank;
    StereoLinked stereoLinked;
    DeltaGetter deltaGetter;
    DeltaScaler deltaScaler;
    MultiBandMod filters;
    BufferDelayer bufferDelayer;
    Plotter spectrumPlotter;
    VolumeMeter in_volumeMeter;
    VolumeMeter out_volumeMeter;

    FT ft_out;

    

};
