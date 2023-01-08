/*
  ==============================================================================

    DynamicEQ.h
    Created: 8 Jan 2023 11:35:56am
    Author:  Amedeo Fresia

  ==============================================================================
*/

#pragma once
using namespace std;

//#include <FilterBank.h>
#include <JuceHeader.h>
#include "Curve.h"
#include "FilterBank.h"
#include "Converters.h"
#include "Analyser.h"
#include <iostream>



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
        conv = converter;
        fs = sampleRate;
        numInChannels = inCh;
        numOutChannels = outCh;
        frequencies = _frequencies;
        fCenters = fbank.getFrequencies();
        atq = getATQ(fCenters);
        //spreadingMtx = getSpreadingFunc(maxFreq, spread_exp);
        fbank.getFilterBank(frequencies);
        fbank.setConverter(conv);
        rel_threshold.getRelativeThreshold(fs, fbank, spreadingMtx);
        inSpectrum.prepareToPlay(sampleRate, fbank, spreadingMtx, true, false);
        scSpectrum.prepareToPlay(sampleRate, fbank, spreadingMtx, true, true);
        outSpectrum.prepareToPlay(sampleRate, fbank, spreadingMtx, false, false);
        //filters.prepareToPlay(sampleRate, samplesPerBlock, numChannels);

    }

    void releaseResources()
    {
        inSpectrum.releaseResources(1000);
        scSpectrum.releaseResources(1000);
        outSpectrum.releaseResources(1000);
        //inputAnalyser.stopThread(1000);
        //outputAnalyser.stopThread(1000);
    }

    void processBlock(AudioBuffer<float>& mainBuffer, AudioBuffer<float>& scBuffer)
    {
        atq.scale(atqWeight);
        applyGain(mainBuffer, inGain);
        applyGain(scBuffer, scGain);

        //if (getActiveEditor() != nullptr) {
        inSpectrum.processBlock(mainBuffer, 0, numInChannels);
        scSpectrum.processBlock(scBuffer, 0, numInChannels);
        //}
        rel_threshold.update(scSpectrum, atq);
        delta.getDelta(inSpectrum, rel_threshold);
        delta.clipDelta(rel_threshold);
        delta.modulateDelta(compAmount,expAmount, stereoLinkAmt, mixAmount);
        






        
       if (wasBypassed) {
            //filters.reset();
            wasBypassed = false;
        }

        //filters.updateGains(delta.yValues);
        //filters.filterBlock(mainBuffer);
        applyGain(mainBuffer, outGain);
        //if (getActiveEditor() != nullptr)
        outSpectrum.processBlock(mainBuffer, 0, numOutChannels);

        //outputAnalyser.addAudioData(mainBuffer, 0, 2);
    }



    void setComp(float newValue) {
        compAmount = newValue;
    }

    void setExp(float newValue) {
        expAmount = newValue;
    }

    void setAtq(float newValue) {
        atqWeight = newValue;
    }

    void setStereoLinked(float newValue) {
        stereoLinkAmt = newValue;
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

    bool checkForNewAnalyserData()
    {
        return inSpectrum.checkForNewData() || scSpectrum.checkForNewData() || outSpectrum.checkForNewData();
    }

    void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minF, Values::signal input)
    {
        if (input==Values::input)
            inSpectrum.createPath(p, bounds, minF);
        else if(input == Values::output)
            outSpectrum.createPath(p, bounds, minF);
        else if (input==Values::sidechain)
            scSpectrum.createPath(p, bounds, minF);

    }


private:

    AbsoluteThreshold getATQ(vector<float>& f)
    {
        vector<float> values;
        for (int i=0; i < f.size(); ++i)
        {
            //   matlab function: absThresh=3.64*(f./1000).^-0.8-6.5*exp(-0.6*(f./1000-3.3).^2)+.00015*(f./1000).^4; % edited function (reduces the threshold in high freqs)
            values[i] = 3.64 * pow((f[i] / 1000), -0.8) - 6.5 * exp(-0.6 * pow(f[i] / 1000 - 3.3, 2)) + 0.00015 * pow(f[i] / 1000, 4);
        }
        float minimum = FloatVectorOperations::findMinimum(values.data(), values.size());
        FloatVectorOperations::add(values.data(), -minimum, values.size());
        return atq.getATQ(values, f);

    }

    void applyGain(AudioBuffer<float>& mainBuffer, float multiplier)
    {
        float*const* mainData = mainBuffer.getArrayOfWritePointers();
        const int numSamples = mainBuffer.getNumSamples();

        for (int ch = mainBuffer.getNumChannels(); --ch >= 0;)
            FloatVectorOperations::multiply(mainData[ch], multiplier, numSamples);
    }
  

    vector<vector<float>> getSpreadingFunc(float maxF, float spread_exp) {
        float fadB = 14.5 + 12;
        float fbdb = 7.5;
        float fbbdb = 26.0;
        float maxbark = conv.hz2bark(maxF);
        float alphaScaled = spread_exp / 20.f;
        vector<float> spreadFuncBarkdB;
        
        spreadFuncBarkdB.resize(2 * nfilts);
        spreadingMtx.resize(nfilts, vector<float>(nfilts));
        vector<float> spreadFuncBarkVoltage(spreadFuncBarkdB);
        vector<float> ascendent = conv.linspace(-maxbark * fbdb, -2.5f, nfilts);
        vector<float> descendent = conv.linspace(1.0f ,-maxbark * fbbdb, nfilts);
        
        FloatVectorOperations::add(ascendent.data(), -fadB, nfilts);
        FloatVectorOperations::add(descendent.data(), -fadB, nfilts);
        
        move(ascendent.begin(), ascendent.end(), std::back_inserter(spreadFuncBarkdB));
        move(descendent.begin(), descendent.end(), std::back_inserter(spreadFuncBarkdB));
        FloatVectorOperations::multiply(spreadFuncBarkdB.data(), alphaScaled, nfilts);

        for (int i = 0; i++ < nfilts;) {
            spreadFuncBarkVoltage[i] = std::pow(10.0f, spreadFuncBarkdB[i]);
            spreadFuncBarkVoltage[i+nfilts] = std::pow(10.0f, spreadFuncBarkdB[i+nfilts]);
            vector<float> temp[nfilts];
            vector<float>::const_iterator first = spreadFuncBarkVoltage.begin() + nfilts - i - 1;
            vector<float>::const_iterator last = spreadFuncBarkVoltage.begin() + 2 * nfilts - i - 2;

            temp->assign(first, last);
            for (int j = 0; j++ < nfilts;)
            {
                spreadingMtx[j,i] = temp[j];
            }
            temp->clear();
        }


        return spreadingMtx;
    }


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
    long fs;
    int numInChannels = 2;
    int numOutChannels = 2;
    float spread_exp = 0.6f;
    vector<vector<float>> spreadingMtx;
    FilterBank fbank;
    Converter conv;
     
    AbsoluteThreshold atq;
    RelativeThreshold rel_threshold;
    RelativeThreshold threshold;
    Delta delta;

    AudioFD inSpectrum;
    AudioFD scSpectrum;
    AudioFD outSpectrum;

    //FilterCascade filters;

    bool wasBypassed = true;


  

};


