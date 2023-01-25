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
#include "FT.h"
#include "PSY.h"



class Delta {

/*
public:

    Curve() {}

    ~Curve() {}

    std::vector<float> yValues;
    std::vector<float> xValues;
    int curveSize;
    Converter conv;


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

    void prepareToPlay(int sampleRate, FilterBank& filterbank, vector<vector<float>>& spreadingMatrix, vector<float> freqs, bool decim, bool spread){
        setDecimated(decim);
        setXValues(freqs);
        spreaded = spread;
        analyser.setupAnalyser(sampleRate, float(sampleRate), fbank, spreadingMtx, decim);
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
        //analyser.addAudioData(mainBuffer, startChannel, numChannels);
        setYValues(analyser.getFD());

        if (decimated) {
            vector<vector<float>> fbValues = fbank.getValues();
            vector<float> _yValues = yValues;
            temp=conv.mXv_mult(fbank.getValues(),yValues);
        }
        else {
            temp = yValues;
        }

        if (spreaded) {
            temp = conv.mXv_mult(spreadingMtx, temp);
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

};*/


public:
    auto getDelta(AudioBuffer<float>& in, AudioBuffer<float>& sc, int ch) {
        inFT = ft_in.getFT(in, ch);
        scFT = ft_sc.getFT(sc, ch);

        scFT = psy.spread(scFT);

        conv.toMagnitudeDb(inFT);
        conv.toMagnitudeDb(scFT);

        scFT = psy.compareWithAtq(scFT, current_atq);
        scFT = difference(inFT, scFT);
        return result{ scFT, inFT };
    }

    void setATQ(float UIatqWeight) {
            current_atq = atq;
            juce::FloatVectorOperations::multiply(current_atq.data(), UIatqWeight+1, nfilts);
            juce::FloatVectorOperations::multiply(current_atq.data(), atqLift, nfilts);
            juce::FloatVectorOperations::add(current_atq.data(), minDBFS, nfilts);
            FloatVectorOperations::clip(current_atq.data(), current_atq.data(), minDBFS, 0.0f, nfilts);
    }

    void prepareToPlay(int sampleRate, int samplesPerBlock, FilterBank fb, float atqW, vector<float> fCenters) {
        scFT.resize(nfilts);
        inFT.resize(nfilts);
        atq.resize(nfilts);
        current_atq.resize(nfilts);
        fCenters.resize(nfilts);
        atq = getATQ(fCenters);
        psy.getSpreadingMtx();
        ft_in.prepare(sampleRate, samplesPerBlock, 1, 1, fb);
        ft_sc.prepare(sampleRate, samplesPerBlock, 1, 1, fb);
        ft_in.setFBank(fb);
        ft_sc.setFBank(fb);
        setATQ(atqW);



    }

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds){
        ft_in.drawFrame(g, bounds);
    }


private:
    //Delta properties
    FT ft_sc;
    FT ft_in;

    PSY psy; 
    Converter conv;

    vector<float> inFT, scFT, current_atq, atq, fCenters;

    float atqWeight;

    struct result { vector<float> delta;  vector<float> threshold;};

    size_t numChannels;

    //Delta properties
    float maxGain = 20;
    int gateThresh = -40;
    int gateKnee = 10;

    //ATQ properties
    int minDBFS = -64;
    float atqLift = 1.6;

    vector<float> difference(vector<float> input, vector<float> rel_thresh) {
        //for (int i = 0; i < input.size(); i++)
            //input[i] = input[i] - rel_thresh[i];
        FloatVectorOperations::subtract(input.data(), rel_thresh.data(), rel_thresh.size());
        return input;

    }

    vector<float> getATQ(vector<float>& f)
    {
        vector<float> values(f.size());
        for (int i = 0; i < f.size(); i++)
        {
            //   matlab function: absThresh=3.64*(f./1000).^-0.8-6.5*exp(-0.6*(f./1000-3.3).^2)+.00015*(f./1000).^4; % edited function (reduces the threshold in high freqs)
            values[i] = 3.64 * pow((f[i] / 1000), -0.8) - 6.5 * exp(-0.6 * pow(f[i] / 1000 - 3.3, 2)) + 0.00015 * pow(f[i] / 1000, 4);
        }
        float minimum = FloatVectorOperations::findMinimum(values.data(), values.size());
        FloatVectorOperations::add(values.data(), -minimum, values.size());

        juce::FloatVectorOperations::multiply(values.data(), atqLift, nfilts);
        juce::FloatVectorOperations::add(values.data(), minDBFS, nfilts);
        FloatVectorOperations::clip(values.data(), values.data(), minDBFS, 0.0f, nfilts);
        return values;

    }

};



/*public:
    void getDelta(AudioFD& input, RelativeThreshold& rel_thresh) {
        std::vector<float> newValues = input.yValues;
        std::vector<float> threshValues = rel_thresh.yValues;
        FloatVectorOperations::subtract(newValues.data(), threshValues.data(), threshValues.size());
        setYValues(newValues);
        setXValues(input.xValues);
        //return *this;
    }

    void clipDelta(RelativeThreshold& threshold) {
        std::vector<float> THclip;
        std::vector<float> newValues;
        newValues.resize(yValues.size());
        THclip.resize(threshold.curveSize);
        for (int i = 0; i < threshold.curveSize; i++) {
            THclip[i] = (1.0f + tanh((threshold.yValues[i] - gateThresh) / gateKnee)) / 2.0f;
        }
        //conv.mXv_mult(yValues, THclip);
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
            temp = tanh(temp / maxGain) * maxGain;
        }
    }*/