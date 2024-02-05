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
//#include "SNRCalculator.h"

class DynamicEQ {
public:
    
    DynamicEQ() {}
    ~DynamicEQ() {}



    void prepareToPlay(array<float,npoints>& _frequencies, double sampleRate, int samplesPerBlock)
    {
        fs = sampleRate;
        numSamples = samplesPerBlock;

        FloatVectorOperations::copy(frequencies.data(), _frequencies.data(), npoints);
        fbank.getFilterBank(frequencies.data());
        fbank.getFrequencies(fCenters);

        //smoothingSeconds = smoothingWindow * _fftSize * pow(fs, -1);

        for (int i = 0; i < nfilts; i++) {
            for (int ch = 0; ch < numScChannels; ch++) {
                gains_sm[ch][i].reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
            }
        }
        
        in_volumeMeter.prepareToPlay(fs, 5.f, true);
        out_volumeMeter.prepareToPlay(fs, 5.f, false);
        
        deltaGetter.prepareToPlay(fs, fbank, fCenters.data(), frequencies.data());
        deltaScaler.prepareToPlay(fCenters.data());
        bufferDelayer.prepareToPlay(numSamples, _fftSize, fs, numChannels);
        filters.prepareToPlay(fs, numSamples, fCenters.data());

        stereoLinked.setSL(stereoLinkAmt);
        //setInGain(inGain);
        deltaScaler.setATQ(atqWeight);
        //int nSamplesToSkip = pow(_editorRefreshRate, -1) * fs;
        spectrumPlotter.prepareToPlay(frequencies.data(), fCenters.data(), fs, numSamples);
        ft_out.prepare(frequencies.data(), fCenters.data(), fs);
        //snrCalc.prepare(numChannels, _snrWindow);
    }

    void numChannelsChanged(int inCh, int scCh) {
        numInChannels = inCh;
        numScChannels = scCh;
        numChannels = max(numInChannels, numScChannels);

        deltaGetter.setNumChannels(numInChannels, numScChannels, numChannels);
        deltaScaler.setNumChannels(numChannels);
        bufferDelayer.setNumChannels(numInChannels); // was numChannels
        filters.setNumChannels(numInChannels);       // was numChannels
        spectrumPlotter.setNumChannels(numChannels);

        for (int i = 0; i < nfilts; i++) 
        {
            for (int ch = 0; ch < numChannels; ch++) 
            {
                gains_sm[ch][i].reset(fs, atkSmoothingSeconds, relSmoothingSeconds);
            }
        }


        //snrCalc.prepare(numChannels, _snrWindow);
    }

    void releaseResources()
    {

    }

    void processBlock(AudioBuffer<float>& mainBuffer, AudioBuffer<float>& scBuffer)
    {
        //if (0)
        {

            //snrCalc.generateNoise(mainBuffer);
            int currentNumSamples = mainBuffer.getNumSamples();

            in_volumeMeter.skip(mainBuffer);
            out_volumeMeter.skip(mainBuffer);

            mainBuffer.applyGain(inGain);
            scBuffer.applyGain(scGain);

            deltaGetter.getDelta(mainBuffer, scBuffer, curves, processFFTresult);

            if (processFFTresult) {

                if (numChannels == 2 && stereoLinkAmt > 0.0f)
                {
                    stereoLinked.process(curves[0].delta, curves[1].delta);
                }

                deltaScaler.scale(curves, maskedFreqsAmount, clearFreqsAmount, mixAmount);
                deltaScaler.clip(curves);
            }

            bufferDelayer.delayBuffer(mainBuffer, curves);


            //snrCalc.pushInput(mainBuffer);

            in_volumeMeter.setLevel(mainBuffer.getRMSLevel(0, 0, currentNumSamples), mainBuffer.getRMSLevel(numInChannels - 1, 0, currentNumSamples));
            in_volumeMeter.setSCLevel(scBuffer.getRMSLevel(0, 0, currentNumSamples), scBuffer.getRMSLevel(numScChannels - 1, 0, currentNumSamples));

            filters.filterBlock(mainBuffer, curves, gains_sm, processFFTresult);
            mainBuffer.applyGain(outGain * _outExtraGain);

            out_volumeMeter.setLevel(mainBuffer.getRMSLevel(0, 0, currentNumSamples), mainBuffer.getRMSLevel(numInChannels - 1, 0, currentNumSamples));
            

            if (processFFTresult) {
                for (int i = 0; i < numInChannels; i++)
                    ft_out.getFT(mainBuffer, i, curves[i].outSpectrum, curves[i].outSpectrum, processFFTresult);

                processFFTresult = false;
            }

            spectrumPlotter.drawNextFrameOfSpectrum(curves, gains_sm);

            //snrCalc.pushOutput(mainBuffer);
            //snrCalc.calculateSNR();
        }
    }

    void setCorrectionGain(int fIndex, float gainDb, bool invert = false)
    {
        filters.setCorrectionGain(fIndex, gainDb, invert);
    }

    float getCorrectionGain(int fIndex)
    {
        return filters.getCorrectionGain(fIndex);
    }

    void setMaskedFreqs(float newValue) {
        maskedFreqsAmount = jlimit(-1.0f, 1.0f, (pow(newValue, 3.0f) + newValue) * 0.5f);
    }

    void setClearFreqs(float newValue) {
        clearFreqsAmount = jlimit(-1.0f, 1.0f, (pow(newValue, 3.0f) + newValue) * 0.5f);
    }

    void setAtq(float newValue) {
        atqWeight = jlimit(0.0f, 1.0f,(pow(newValue, 0.5f) + newValue / 10.0f) * 0.91f);
        deltaScaler.setATQ(atqWeight);
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
            in_volumeMeter.resetMaxVolume();
        }
        if(meter == "out")
        {
            out_volumeMeter.resetMaxVolume();
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


    int numInChannels = 2;
    int numScChannels = 2;
    int numChannels = 2;

    double fs;
    int numSamples;

    bool processFFTresult = false;

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
    

    //SNRCalculator snrCalc;

};
