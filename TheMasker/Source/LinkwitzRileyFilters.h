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



    void prepareToPlay(double sampleRate, int newSamplesPerBlock, float lc_freq, float hc_freq) {
        samplesPerBlock = newSamplesPerBlock;
        LC_freq = lc_freq;
        HC_freq = hc_freq;

        isFirstBand = (LC_freq - minFreq) < 1.0f;
        isLastBand = (HC_freq - maxFreq) < 1.0f;

        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        LC.setType(isFirstBand ? dsp::LinkwitzRileyFilterType::allpass : dsp::LinkwitzRileyFilterType::highpass);
        LC.setCutoffFrequency(LC_freq);
        HC.setType(isLastBand ? dsp::LinkwitzRileyFilterType::allpass : dsp::LinkwitzRileyFilterType::lowpass);
        HC.setCutoffFrequency(HC_freq);
        HC.reset();
        LC.reset();

    }

    void process(AudioBuffer<float> inputBuffer, AudioBuffer<float>& outputBuffer) {
        juce::dsp::AudioBlock<float>              ioBlock(inputBuffer);
        juce::dsp::ProcessContextReplacing<float> context(ioBlock);
        LC.process(context);
        HC.process(context);
        context.getOutputBlock().copyTo(outputBuffer, 0, 0, samplesPerBlock);
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
    bool isFirstBand = false;
    bool isLastBand = false;

};
