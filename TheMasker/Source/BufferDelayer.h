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
        oldBuffer.setSize(nCh,samplesPerBlock) ;
        delaySamples = numSamplesToDelay;
    }

    void delayBuffer(AudioBuffer<float>& newBuffer) {
        //newBuffer.copyFrom(oldBuffer);

    }
private:
    AudioBuffer<float> oldBuffer;
    int delaySamples;
};