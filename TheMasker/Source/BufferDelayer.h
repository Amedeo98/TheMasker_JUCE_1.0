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
    void prepareToPlay(int samplesPerBlock, int nCh, int numSamplesToDelay) {
        numCh = nCh;
        numSamples = samplesPerBlock;
        delaySamples = numSamplesToDelay;
        storedBuffer.setSize(numCh, numSamples);
        fifo.resize(delaySamples+numSamples);
        readIndex = delaySamples;
    }

    void delayBuffer(AudioBuffer<float>& newBuffer) {
        for(int ch=0; ch < numCh; ch++)
            if (newBuffer.getNumChannels() > 0)
            {
                auto* channelData = newBuffer.getReadPointer(ch, 0);

                for (auto i = 0; i < numSamples; ++i) {
                    pushNextSampleIntoFifo(channelData[i]);
                    newBuffer.setSample(ch, i, getNextSamplefromFifo());
                }
            }

    }


private:
    AudioBuffer<float> storedBuffer;
    vector<float> fifo;
    int numSamples;
    int delaySamples;
    int fifoIndex = 0;
    int readIndex;
    int numCh;


    void pushNextSampleIntoFifo(float sample) noexcept
    {

        if (fifoIndex == (delaySamples+numSamples)) 
        {
            fifoIndex = 0;
        }
        fifo[fifoIndex++] = sample;
    }

    float getNextSamplefromFifo() noexcept
    {
        if (readIndex == (delaySamples + numSamples))
        {
            readIndex = 0;
        }
        return fifo[readIndex++];
    }

};