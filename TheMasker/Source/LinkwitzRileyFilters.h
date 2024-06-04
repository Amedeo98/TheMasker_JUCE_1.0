/*
  ==============================================================================

    LinkwitzRileyFilters.h
    Created: 25 Jan 2023 10:27:24am
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LinkwitzRileyFilters {
public:
    LinkwitzRileyFilters() {}
    ~LinkwitzRileyFilters() {}



    void prepareToPlay(double sampleRate, int newSamplesPerBlock, float lc_freq, float hc_freq) {
        samplesPerBlock = newSamplesPerBlock;
        LC_freq = lc_freq;
        HC_freq = hc_freq;

        isFirstBand = abs(LC_freq - minFreq) < 1.0f;
        isLastBand = abs(HC_freq - maxFreq) < 1.0f;

        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;

        LC.setType(isFirstBand ? dsp::LinkwitzRileyFilterType::allpass : dsp::LinkwitzRileyFilterType::highpass);
        LC.setCutoffFrequency(LC_freq);
        HC.setType(isLastBand ? dsp::LinkwitzRileyFilterType::allpass : dsp::LinkwitzRileyFilterType::lowpass);
        HC.setCutoffFrequency(HC_freq);
        HC.reset();
        LC.reset();

    }

    // is inputBuffer not referenced to avoid modification?!
    //void process(AudioBuffer<float> inputBuffer, AudioBuffer<float>& outputBuffer)
    //{
    //    int numSamples = inputBuffer.getNumSamples();
    //    juce::dsp::AudioBlock<float>              ioBlock(inputBuffer);
    //    juce::dsp::ProcessContextReplacing<float> context(ioBlock);
    //    if (!isFirstBand) LC.process(context);
    //    if (!isLastBand) HC.process(context);
    //    context.getOutputBlock().copyTo(outputBuffer, 0, 0, numSamples);
    //}

    void process(AudioBuffer<float>& inputBuffer, AudioBuffer<float>& outputBuffer)
    {
        const int numSamples = inputBuffer.getNumSamples();

        for (int ch = inputBuffer.getNumChannels(); --ch>=0;)
            outputBuffer.copyFrom(ch, 0, inputBuffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float>              ioBlock(outputBuffer);
        juce::dsp::ProcessContextReplacing<float> context(ioBlock);
        if (!isFirstBand) LC.process(context);
        if (!isLastBand) HC.process(context);
        //context.getOutputBlock().copyTo(outputBuffer, 0, 0, numSamples);
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinkwitzRileyFilters)
};
