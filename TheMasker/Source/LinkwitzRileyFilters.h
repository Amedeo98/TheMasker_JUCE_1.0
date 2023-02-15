/*
  ==============================================================================

    LinkwitzRileyFilters.h
    Created: 25 Jan 2023 10:27:24am
    Author:  nikid

  ==============================================================================
*/

#pragma once

class LinkwitzRileyFilters {
public:
    LinkwitzRileyFilters() {}
    ~LinkwitzRileyFilters() {}



    void prepareToPlay(double sampleRate, int newSamplesPerBlock, float lc_freq, float hc_freq, int nCh) {
        samplesPerBlock = newSamplesPerBlock;
        LC_freq = lc_freq;
        HC_freq = hc_freq;

        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = nCh;
        HC.prepare(spec);
        LC.prepare(spec);

        LC.setType(dsp::LinkwitzRileyFilterType::highpass);
        LC.setCutoffFrequency(LC_freq);
        HC.setType(dsp::LinkwitzRileyFilterType::lowpass);
        HC.setCutoffFrequency(HC_freq);
        
    }

    void process(AudioBuffer<float> inBuffer, AudioBuffer<float>& outputBuffer) {

        juce::dsp::AudioBlock<float>              ioBuffer(inBuffer);
        juce::dsp::ProcessContextReplacing<float> context(ioBuffer);
        HC.process(context);
        LC.process(context);
        context.getOutputBlock().copyTo(outputBuffer, 0, 0, context.getOutputBlock().getNumSamples());
        //return outputBuffer;
    }

    void setNumChannels(int nCh) {
        spec.numChannels = nCh;
        HC.prepare(spec);
        LC.prepare(spec);

    }


private:
    juce::dsp::ProcessSpec spec;
    int samplesPerBlock;
    using LWR = dsp::LinkwitzRileyFilter<float>;
    LWR LC;
    LWR HC;
    float LC_freq;
    float HC_freq;

};
