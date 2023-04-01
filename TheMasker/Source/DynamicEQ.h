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
#include "Constants.h"

class DynamicEQ {
public:
    
    DynamicEQ() {}
    ~DynamicEQ() {}



    void prepareToPlay(array<float,npoints>& _frequencies, int sampleRate, int samplesPerBlock)
    {
        fs = sampleRate;
        numSamples = samplesPerBlock;

        frequencies = _frequencies;
        fbank.getFilterBank(frequencies.data());
        fbank.getFrequencies(fCenters);

        //smoothingSeconds = smoothingWindow * _fftSize * pow(fs, -1);

        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numScChannels; ch++) {
                gains_sm[ch][i].reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
            }
        }
        
        max_in_L.reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
        max_in_R.reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
        
        max_out_L.reset(fs, 0.f, 5.f);
        max_out_R.reset(fs, 0.f, 5.f);

        deltaGetter.prepareToPlay(fs, numSamples, fbank, fCenters.data(), frequencies.data());
        bufferDelayer.prepareToPlay(numSamples, _fftSize, fs);
        filters.prepareToPlay(fs, numSamples, fCenters.data());

        stereoLinked.setSL(stereoLinkAmt);
        setInGain(DEFAULT_IN);
        //deltaGetter.setATQ(DEFAULT_ATQ);

        spectrumPlotter.prepareToPlay(frequencies.data(), fCenters.data());
        ft_out.prepare(frequencies.data(), fCenters.data(), fs);
    }

    void numChannelsChanged(int inCh, int scCh) {
        numInChannels = inCh;
        numScChannels = scCh;
        numChannels = max(numInChannels, numScChannels);

        deltaGetter.setNumChannels(numInChannels, numScChannels, numChannels);
        deltaScaler.setNumChannels(numChannels);
        bufferDelayer.setNumChannels(numChannels);
        filters.setNumChannels(numChannels);
        spectrumPlotter.setNumChannels(numChannels);

        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numChannels; ch++) {
                gains_sm[ch][i].reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
            }
        }
    }

    void releaseResources()
    {

    }

    void processBlock(AudioBuffer<float>& mainBuffer, AudioBuffer<float>& scBuffer)
    {
        max_in_L.skip(mainBuffer.getNumSamples());
        max_in_R .skip(mainBuffer.getNumSamples());
        max_out_L.skip(mainBuffer.getNumSamples());
        max_out_R.skip(mainBuffer.getNumSamples());
        
        mainBuffer.applyGain(inGain);
        scBuffer.applyGain(scGain);

        deltaGetter.getDelta(mainBuffer, scBuffer, curves);

        if (numChannels == 2 && stereoLinkAmt > 0.0f)
        {
            stereoLinked.process(curves[0].delta, curves[1].delta);
        }

        deltaScaler.scale(curves, maskedFreqsAmount, clearFreqsAmount, mixAmount);
        deltaScaler.clip(curves);

        bufferDelayer.delayBuffer(mainBuffer, curves);

        auto val_L = mainBuffer.getRMSLevel(0, 0, numSamples);
        auto val_R = mainBuffer.getRMSLevel(numChannels - 1, 0, numSamples);
        
        if(val_L > max_in_L.getCurrentValue())
            max_in_L.setTargetValue(val_L);
        else
            max_in_L.setCurrentAndTargetValue(max_in_L.getCurrentValue()-(max_in_L.getCurrentValue() * 0.01f));
        
        if(val_L > max_in_R.getCurrentValue())
            max_in_R.setTargetValue(val_R);
        else
            max_in_R.setCurrentAndTargetValue(max_in_R.getCurrentValue()-(max_in_R.getCurrentValue() * 0.01f));
        
        in_volumeMeter.setLevel(val_L, val_R, max_in_L.getCurrentValue(), max_in_R.getCurrentValue());
        filters.filterBlock(mainBuffer, curves, gains_sm);
        mainBuffer.applyGain(outGain * _outExtraGain);
    
        val_L = mainBuffer.getRMSLevel(0, 0, numSamples);
        val_R = mainBuffer.getRMSLevel(numChannels - 1, 0, numSamples);
        
        if(val_L > max_out_L.getCurrentValue())
            max_out_L.setTargetValue(val_L);
        else
            max_out_L.setTargetValue(0.f);
        
        if(val_R > max_out_R.getCurrentValue())
            max_out_R.setTargetValue(val_R);
        else
            max_out_R.setTargetValue(0.f);
        
        out_volumeMeter.setLevel(val_L, val_R, max_out_L.getCurrentValue(), max_out_R.getCurrentValue());
       
        for (int i = 0; i < numChannels; i++)
        ft_out.getFT(mainBuffer, i, curves[i].outSpectrum, curves[i].outSpectrum);

        spectrumPlotter.drawNextFrameOfSpectrum(curves, gains_sm);

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
    
    void toggleSpectrumView(juce::String btn)
    {
        spectrumPlotter.toggleSpectrumView(btn);
    }
    
    void resetMaxVolume(juce::String meter)
    {
        if(meter == "in")
        {
            max_in_L.setCurrentAndTargetValue(0.f);
            max_in_R.setCurrentAndTargetValue(0.f);
        }
        if(meter == "out")
        {
            max_out_L.setCurrentAndTargetValue(0.f);
            max_out_R.setCurrentAndTargetValue(0.f);
        }
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


    int numInChannels = 0;
    int numScChannels = 0;
    int numChannels = 0;

    int fs;
    int numSamples;

    bool nextFFTBlockReady = false;

    struct curve
    {
        array<float, nfilts> delta;
        array<float, nfilts> inputDecimated;
        array<float, nfilts> scDecimated;
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
    CustomSmoothedValue<float> max_in_L, max_in_R, max_out_L, max_out_R;
    

};
