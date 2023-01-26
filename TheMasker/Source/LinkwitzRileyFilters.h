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



    void prepareToPlay(double sampleRate, int samplesPerBlock, float lc_freq, float hc_freq) {
        outputBuffer.setSize(1, samplesPerBlock);
        LC = dsp::LinkwitzRileyFilter<float>();
        HC = dsp::LinkwitzRileyFilter<float>();
        LC_freq = lc_freq;
        HC_freq = hc_freq;

        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = 1;
        HC.prepare(spec);
        LC.prepare(spec);

        LC.setType(dsp::LinkwitzRileyFilterType::highpass);
        LC.setCutoffFrequency(LC_freq);
        HC.setType(dsp::LinkwitzRileyFilterType::lowpass);
        HC.setCutoffFrequency(HC_freq);
        
    }

    AudioBuffer<float> process(AudioBuffer<float> inBuffer) {

        juce::dsp::AudioBlock<float>              ioBuffer(inBuffer);
        juce::dsp::ProcessContextReplacing<float> context(ioBuffer);
        HC.process(context);
        LC.process(context);
        for (int sample = 0; sample < inBuffer.getNumSamples(); sample++) {
            outputBuffer.setSample(0, sample, context.getOutputBlock().getSample(0, sample));
        }
        return outputBuffer;
    }


private:
    AudioBuffer<float> outputBuffer;
    using LWR = dsp::LinkwitzRileyFilter<float>;
    LWR LC;
    LWR HC;
    float LC_freq;
    float HC_freq;
    };
