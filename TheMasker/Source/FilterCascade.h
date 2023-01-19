/*
  ==============================================================================

    FilterCascade.h
    Created: 26 Dec 2022 10:36:19pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Converters.h"
#include "SimpleFilter.h"
#include "DynamicEQ.h"



class FilterCascade {
public:
    FilterCascade(){}
    ~FilterCascade(){}

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels, vector<float> freqs) {
        
        filters.resize(nfilts);
        outputBlock.resize(nfilts);
        for (int f = 0; f < nfilts; f++) {
            filters[f].reset();
            filters[f].prepareToPlay(sampleRate, samplesPerBlock, numChannels, freqs[f]);
        }
    }

    void updateGains(vector<float>& delta){
        for (int f = 0; f < nfilts; f++) {
            filters[f].updateGain(delta[f]);
        }
    }

    void filterBlock(juce::dsp::ProcessContextReplacing<float>& context) {
        for (int f = 0; f < nfilts; f++) {
            filters[f].process(context);
        }
       

    }

    void reset() {
        for (int f = 0; f < nfilts; f++) {
            filters[f].reset();
        }
    }

private:
vector<Filter> filters;
vector<juce::dsp::AudioBlock<float>> outputBlock;

};
