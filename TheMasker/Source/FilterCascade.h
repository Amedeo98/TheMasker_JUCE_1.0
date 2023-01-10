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



class FilterCascade {
public:
    FilterCascade(){}
    ~FilterCascade(){}

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels, vector<float> freqs) {
        
        filters.resize(nfilts);
        for (int f = 0; f < nfilts; f++) {
            filters[f].prepareToPlay(sampleRate, samplesPerBlock, numChannels);
        }
    }

    void updateGains(vector<float>& delta){

    }

    void filterBlock(AudioBuffer<float>& mainBuffer) {

    }

    void reset() {
        for (int f = 0; f < nfilts; f++) {
            filters[f].reset();
        }
    }
private:
vector<Filter> filters;
};
