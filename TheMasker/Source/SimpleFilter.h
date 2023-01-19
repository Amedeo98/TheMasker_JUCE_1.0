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
        filter.reset();
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels, float freq) {
        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = numChannels;
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        filter.setCutoffFrequency(freq);
        filter.setResonance(1 / sqrt(2));
        filter.prepare(spec);

    }

    void updateGain(float g) {
        gain = Decibels::decibelsToGain(g);
    }

    void process(juce::dsp::ProcessContextReplacing<float>& c) {
        filter.process(c);
        outputBlock = c.getOutputBlock();
        outputBlock.multiplyBy(gain);
    }


    using F = juce::dsp::StateVariableTPTFilter<float>;
    F filter;

private:
    float gain;
    juce::dsp::AudioBlock<float> outputBlock;

};
