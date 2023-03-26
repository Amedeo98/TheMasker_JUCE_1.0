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
#include "CustomSmoothedValue.h"


#define DEFAULT_MASKEDF 0.0f
#define DEFAULT_CLEARF 0.0f
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



    void prepareToPlay(array<float,npoints>& _frequencies, int sampleRate, int inCh, int scCh, int samplesPerBlock, bool _stereoSignals)
    {
        fs = sampleRate;
        numSamples = samplesPerBlock;

        numInChannels = inCh;
        numScChannels = scCh;

        stereoSignals = _stereoSignals;

        frequencies = _frequencies;
        fbank.getFilterBank(frequencies.data());
        fbank.getFrequencies(fCenters);

        //smoothingSeconds = smoothingWindow * _fftSize * pow(fs, -1);

        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numScChannels; ch++) {
                gains_sm[ch][i].reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
            }
        }

        deltaGetter.prepareToPlay(fs, numSamples, fbank, fCenters.data(), frequencies.data(), numInChannels, numScChannels, stereoSignals);
        deltaScaler.prepareToPlay(numScChannels);

        bufferDelayer.prepareToPlay(numSamples, stereoSignals, _fftSize, fs);
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
        stereoSignals = inCh > 1 || scCh > 1;


        deltaGetter.setNumChannels(numInChannels, numScChannels, stereoSignals);
        deltaScaler.setNumChannels(numScChannels);
        bufferDelayer.setStereo(stereoSignals);
        filters.setNumChannels(numInChannels, numScChannels);

        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numScChannels; ch++) {
                gains_sm[ch][i].reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
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

        if (stereoSignals && stereoLinkAmt > 0.0f)
        {
            stereoLinked.process(curves[0].delta, curves[1].delta);
        }

        deltaScaler.scale(curves, maskedFreqsAmount, clearFreqsAmount, mixAmount);
        deltaScaler.clip(curves);

        bufferDelayer.delayBuffer(mainBuffer, curves);

        in_volumeMeter.setLevel(mainBuffer.getRMSLevel(0, 0, numSamples), mainBuffer.getRMSLevel(1, 0, numSamples));
        filters.filterBlock(mainBuffer, curves, gains_sm);
        mainBuffer.applyGain(outGain * _outExtraGain);
        out_volumeMeter.setLevel(mainBuffer.getRMSLevel(0, 0, numSamples), mainBuffer.getRMSLevel(1, 0, numSamples));

       
        for (int i = 0; i < 2; i++)
        ft_out.getFT(mainBuffer, i, curves[i].outSpectrum, curves[i].outSpectrum);

        spectrumPlotter.drawNextFrameOfSpectrum(curves[0].inSpectrum, curves[0].scSpectrum, curves[0].outSpectrum, gains_sm[0]);

    }



    void setMaskedFreqs(float newValue) {
        maskedFreqsAmount = newValue;
    }

    void setClearFreqs(float newValue) {
        clearFreqsAmount = newValue;
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

    float maskedFreqsAmount = DEFAULT_MASKEDF;
    float clearFreqsAmount = DEFAULT_CLEARF;
    float atqWeight = DEFAULT_ATQ;
    float stereoLinkAmt = DEFAULT_SL;
    float mixAmount = DEFAULT_MIX;
    float inGain = Decibels::decibelsToGain(DEFAULT_IN);
    float outGain = Decibels::decibelsToGain(DEFAULT_OUT);
    float scGain = Decibels::decibelsToGain(DEFAULT_SC);


    int numInChannels = 2;
    int numScChannels;
    bool stereoSignals;

    int fs;
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
    array<array<CustomSmoothedValue<float, ValueSmoothingTypes::Linear>, nfilts>, 2> gains_sm;

    float atkSmoothingSeconds = _atkSmoothingSeconds;
    float relSmoothingSeconds = _relSmoothingSeconds;
    float smoothingWindow = _overlapRatio;

    FilterBank fbank;
    StereoLinked stereoLinked;
    DeltaGetter deltaGetter;
    DeltaScaler deltaScaler;
    MultiBandMod filters;
    Plotter spectrumPlotter;
    VolumeMeter in_volumeMeter;
    VolumeMeter out_volumeMeter;

    BufferDelayer bufferDelayer;

    FT ft_out;

    Converter conv;

    

};
