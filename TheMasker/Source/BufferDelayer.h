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
        //storedBuffer.setSize(numCh, delaySamples + numSamples);

        for (int ch=0; ch < numCh; ch++)
        fifo[ch].resize(delaySamples + numSamples);

        readIndex = delaySamples;
    }

    void delayBuffer(AudioBuffer<float>& newBuffer) {
        for(int ch=0; ch < numCh; ch++)
            if (newBuffer.getNumChannels() > 0)
            {
                auto* channelData = newBuffer.getReadPointer(ch, 0);

                for (auto i = 0; i < numSamples; ++i) {
                    pushNextSampleIntoFifo(channelData[i], ch);
                    newBuffer.setSample(ch, i, getNextSamplefromFifo(ch));
                }
            }

    }


private:
    //AudioBuffer<float> storedBuffer;
    array<vector<float>, 2> fifo;
    int numSamples;
    int delaySamples;
    int fifoIndex = 0;
    int readIndex;
    int numCh;


    void pushNextSampleIntoFifo(float sample, int ch) noexcept
    {

        if (fifoIndex == (delaySamples+numSamples)) 
        {
            fifoIndex = 0;
        }
        //storedBuffer.setSample(ch, fifoIndex++, sample);
        fifo[ch][fifoIndex++] = sample;
    }

    float getNextSamplefromFifo(int ch) noexcept
    {
        if (readIndex == (delaySamples + numSamples))
        {
            readIndex = 0;
        }
        return /*storedBuffer.getSample(ch, readIndex++);*/ fifo[ch][readIndex++];
    }

};