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
    void prepareToPlay(int samplesPerBlock, bool _stereoSignals, int numSamplesToDelay, int sampleRate) {
        numSamples = samplesPerBlock;
        delaySamples = numSamplesToDelay;
        bufferSpec.maximumBlockSize = samplesPerBlock;
        stereoSignals = _stereoSignals;
        numCh = stereoSignals ? 2 : 1;
        bufferSpec.numChannels = numCh;
        bufferSpec.sampleRate = sampleRate;

        bufferDelayLine.prepare(bufferSpec);
        bufferDelayLine.setMaximumDelayInSamples(delaySamples);

        inDelayLine.prepare(bufferSpec);
        inDelayLine.setMaximumDelayInSamples(delaySamples);

        scDelayLine.prepare(bufferSpec);
        scDelayLine.setMaximumDelayInSamples(delaySamples);
    }

    void delayBuffer(AudioBuffer<float>& newBuffer, auto& curves) {
        for(int ch=0; ch < numCh; ch++)
            {
                auto* channelData = newBuffer.getReadPointer(ch, 0);
                auto* inSpectrumData = curves[ch].inSpectrum.data();
                auto* scSpectrumData = curves[ch].scSpectrum.data();
                for (auto i = 0; i < numSamples; ++i) {
                    bufferDelayLine.pushSample(ch, channelData[i]);
                    /*inDelayLine.pushSample(ch, inSpectrumData[i]);
                    scDelayLine.pushSample(ch, scSpectrumData[i]);*/
                    newBuffer.setSample(ch, i, bufferDelayLine.popSample(ch));
                   /* curves[ch].inSpectrum[i] = inDelayLine.popSample(ch);
                    curves[ch].scSpectrum[i] = scDelayLine.popSample(ch);*/
                }
            }

    }

  
    void setStereo(bool stereo) {
        stereoSignals = stereo;
    }

private:
    int numSamples;
    int delaySamples;
    int numCh;
    bool stereoSignals;
    dsp::DelayLine<float> bufferDelayLine;
    dsp::DelayLine<float> inDelayLine;
    dsp::DelayLine<float> scDelayLine;
    juce::dsp::ProcessSpec bufferSpec;

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