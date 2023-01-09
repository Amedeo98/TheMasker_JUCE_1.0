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


//template <typename F>
class FilterCascade {
public:
    FilterCascade(){}
    ~FilterCascade(){}

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels) {
        
        //filters.resize(nfilts);
        for (int f = 0; ++f < nfilts;) {
            //filters[f].prepareToPlay(sampleRate, samplesPerBlock, numChannels);
        }
    }

    void updateGains(vector<float>& delta){

    }

    void filterBlock(AudioBuffer<float>& mainBuffer) {

    }
private:
//vector<Filter> filters;
};

class Filter {
public:
    Filter(){}
    ~Filter(){}

    void reset() {
    }
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels) {
        filter.reset();

        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = numChannels;
        filter.prepare(spec);
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    }

    void updateGain() {

    }

    using F = juce::dsp::StateVariableTPTFilter<float>;

private:
    F filter;
};