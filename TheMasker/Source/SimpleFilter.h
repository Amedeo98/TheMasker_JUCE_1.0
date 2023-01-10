/*
  ==============================================================================

    SimpleFilter.h
    Created: 9 Jan 2023 4:44:05pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

class Filter {
public:
    Filter() {}
    ~Filter() {}

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
    F filter;

private:
};
