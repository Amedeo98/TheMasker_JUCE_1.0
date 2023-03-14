/*
  ==============================================================================

    BufferDelayer.h
    Created: 24 Feb 2023 8:06:00pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>



class BufferDelayer {
public:
    void prepareToPlay(int samplesPerBlock, int nCh, int numSamplesToDelay, int sampleRate) {
        numCh = nCh;
        numSamples = samplesPerBlock;
        delaySamples = numSamplesToDelay*8;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = numCh;
        spec.sampleRate = sampleRate;
        delayLine.prepare(spec);
        delayLine.setMaximumDelayInSamples(delaySamples);
        delayLine.setDelay(delaySamples);
    }

    void delayBuffer(AudioBuffer<float>& newBuffer) {
        for(int ch=0; ch < numCh; ch++)
            if (newBuffer.getNumChannels() > 0)
            {
                auto* channelData = newBuffer.getReadPointer(ch, 0);
                for (auto i = 0; i < numSamples; ++i) {
                    delayLine.pushSample(ch, channelData[i]);
                    newBuffer.setSample(ch, i, delayLine.popSample(ch));
                }
            }

    }


private:
    int numSamples;
    int delaySamples;
    int numCh;
    dsp::DelayLine<float> delayLine;
    juce::dsp::ProcessSpec spec;

    //void pushNextSampleIntoFifo(float sample, int ch) noexcept
    //{

    //    if (fifoIndex == (delaySamples+numSamples)) 
    //    {
    //        fifoIndex = 0;
    //    }
    //    //storedBuffer.setSample(ch, fifoIndex++, sample);
    //    fifo[ch][fifoIndex++] = sample;
    //}

    //float getNextSamplefromFifo(int ch) noexcept
    //{
    //    if (readIndex == (delaySamples + numSamples))
    //    {
    //        readIndex = 0;
    //    }
    //    //return storedBuffer.getSample(ch, readIndex++);
    //    return fifo[ch][readIndex++];
    //}

};