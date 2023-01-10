/*
  ==============================================================================

    Curve.h
    Created: 14 Dec 2022 5:15:30pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Converters.h"
#include "FilterBank.h"
#include "PluginProcessor.h"
#include "Analyser.h"



class Curve {

public:

    Curve() {}

    ~Curve() {}

    //using enum domain;
    //using enum scale;

    std::vector<float> yValues;
    std::vector<float> xValues;
    int curveSize;

    void setYValues(std::vector<float> newY) {
        curveSize = newY.size();
        yValues.resize(curveSize);
        yValues = newY;
    }

    void setXValues(std::vector<float> newX) {
        curveSize = newX.size();
        xValues.resize(curveSize);
        xValues = newX;
    }

    void setConverter(Converter c) {
        conv = c;
    }

    std::vector<float> getXValues() {
       return xValues;
    }
    std::vector<float> getYValues() {
       return yValues;
    }
    Converter conv;

};


class AbsoluteThreshold : public Curve {
public:
    void setATQ(std::vector<float> y, std::vector<float> x) 
    {
        setYValues(y);
        setXValues(x);
        
    }


    void scale(float UIatqWeight) {
        vector<float> values = yValues;
        juce::FloatVectorOperations::multiply(values.data(), UIatqWeight, curveSize);
        juce::FloatVectorOperations::multiply(values.data(), atqLift, curveSize);
        juce::FloatVectorOperations::add(values.data(), minDBFS, curveSize);
        FloatVectorOperations::clip(values.data(), values.data(), minDBFS, 0.0f, curveSize);
        setYValues(values);
    }
private:
    //ATQ properties
    int minDBFS = -64;
    float atqLift = 1.6;

  
};



class AudioFD : public Curve {
public:

    void prepareToPlay(float sampleRate, FilterBank& filterbank, vector<vector<float>>& spreadingMatrix, vector<float> freqs, bool decim, bool spread){
        setDecimated(decim);
        setXValues(freqs);
        spreaded = spread;
        analyser.setupAnalyser(int(sampleRate), float(sampleRate), fbank, spreadingMtx);
        fbank = filterbank;
        spreadingMtx = spreadingMatrix;
        setConverter(fbank.getConverter());
        decim ? yValues.resize(nfilts) : yValues.resize(npoints);
    }


    void releaseResources(int timeOut){
        analyser.stopThread(timeOut);
    }

    bool checkForNewData() {
        return analyser.checkForNewData();
    }

    void createPath(juce::Path& p, const juce::Rectangle<int>& bounds, float minF) {
        analyser.createPath(p, bounds.toFloat(), minF);
    }

    void processBlock(AudioBuffer<float>& mainBuffer, int startChannel, int numChannels) {
        analyser.addAudioData(mainBuffer, startChannel, numChannels);
        setYValues(analyser.getFD());

        if (decimated) {
            vector<vector<float>> fbValues = fbank.getValues();
            vector<float> _yValues = yValues;
            temp=conv.mXv_mult(fbank.getValues(),yValues);
            /*std::transform(fbank.getValues().begin(), fbank.getValues().end(),
                yValues.begin(), temp.begin(),
                std::multiplies<float>());*/
            //setDecimated(true);
        }
        else {
            temp = yValues;
        }

        if (spreaded) {
            temp = conv.mXv_mult(spreadingMtx, temp);
        /*    std::transform(spreadingMtx.begin(), spreadingMtx.end(),
              temp.begin(), temp.begin(),
              std::multiplies<float>()); */
        }

        for (int i = 0; i < temp.size(); i++) {
            temp[i] = real(conv.amp2db(temp[i]));
        }
        setYValues(temp);

    }


    void setDecimated(bool decim) {
        decimated = decim;
    }


private:
    bool decimated = false;
    bool spreaded = false;
    FilterBank fbank;
    vector<vector<float>> spreadingMtx;
    vector<float> temp;
    Analyser<float> analyser;



};

class RelativeThreshold : public Curve {
public:


    RelativeThreshold getRelativeThreshold(int sampleRate, FilterBank& filterbank, vector<vector<float>>& spreadMtx) {
        yValues.resize(nfilts);
        //xValues.resize(curveSize);
        setXValues(filterbank.getFrequencies());
        //fs = sampleRate;
        //fbank = filterbank;
        //spreadingMtx = spreadMtx;
        //setYValues() ...?
        return *this;
    }

    void update(AudioFD& scSpectrum, AbsoluteThreshold& atq) {
        setYValues(jmax(scSpectrum.getYValues(), atq.yValues));
    }


private:
    //int fs;
    //vector<vector<float>> spreadingMtx;
    //FilterBank fbank;

};

class Delta : public Curve {
public:
    Delta getDelta(AudioFD& input, RelativeThreshold& rel_thresh) {
        std::vector<float> newValues = input.yValues;
        std::vector<float> threshValues = rel_thresh.yValues;
        FloatVectorOperations::subtract(newValues.data(), threshValues.data(), threshValues.size());
        setYValues(newValues);
        setXValues(input.xValues);
        return *this;
    }

    void clipDelta(RelativeThreshold& threshold) {
        std::vector<float> THclip;
        std::vector<float> newValues;
        newValues.resize(yValues.size());
        THclip.resize(threshold.curveSize);
        for (int i = 0; i < threshold.curveSize; i++) {
            THclip[i] = (1.0f + tanh((threshold.yValues[i] - gateThresh) / gateKnee)) / 2.0f;
        }
        std::transform(yValues.begin(), yValues.end(),
            THclip.begin(), newValues.data(), 
            std::multiplies<float>()); 
        setYValues(newValues);
    }

    void modulateDelta(float UIcomp, float UIexp, float UIsl, float UImix) {
        //std::vector<float> monoValues;
        for (int i = 0; i < curveSize; i++) {
            float& temp = yValues[i];
            //monoValues = mean(yValues);
            //temp = UIsl * monoValues + (1-UIsl) * temp;
            temp = temp > 0.0f ? temp * UIcomp : temp * UIexp;
            temp = temp * UImix;
            tanh(temp / maxGain)* maxGain;
        }
    }


private:
    //Delta properties
    float maxGain = 20;
    int gateThresh = -40;
    int gateKnee = 10;
};



