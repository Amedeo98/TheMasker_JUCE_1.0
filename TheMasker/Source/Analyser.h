/*
  ==============================================================================

    BlockAnalyser.h
    Created: 13 Feb 2023 10:05:06pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include "Constants.h"


class Analyser
{
public:

    Analyser(const int fftOrder, const int fftSize_)
        : forwardFFT (fftOrder), fftSize (fftSize_)
    {
        //dsp::WindowingFunction<float>::fillWindowingTables(window.data(), fftSize, dsp::WindowingFunction<float>::WindowingMethod::hann, false);
        dsp::WindowingFunction<float>::fillWindowingTables(window.data(), fftSize, dsp::WindowingFunction<float>::WindowingMethod::flatTop, false);
    }


    void process(AudioBuffer<float>& bufferToFill, int ch, bool& processFFTresult)
    {
        if (bufferToFill.getNumChannels() > 0)
        {
            auto* channelData = bufferToFill.getReadPointer(ch, 0);

            for (auto i = 0; i < bufferToFill.getNumSamples(); ++i)
                pushNextSampleIntoFifo(channelData[i]);
        }
        if (nextFFTBlockReady) processFFTresult = true;
    }


    void getResult(array<float, _fftSize>& result)
    {
        if (nextFFTBlockReady) {
            FloatVectorOperations::multiply(fftData.data(), window.data(), fftSize);

            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data(), false);  // [2]

            FloatVectorOperations::multiply(fftData.data(), 1.0f / (float)fftSize, fftSize); // 1.0 was missing this scaling

            FloatVectorOperations::copy(result.data(), fftData.data(), fftSize);

            nextFFTBlockReady = false;
        }
    }

    void pushNextSampleIntoFifo(float sample) noexcept
    {
        if (fifoIndex == fftSize)               
        {
            if (!nextFFTBlockReady)            
            {
                FloatVectorOperations::fill(fftData.data(), 0.0f, fftSize * 2);
                memcpy(fftData.data(), fifo.data(), fftSize);
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        fifo[fifoIndex++] = sample;            
    }


   

    array<float, _fftSize> result;
    float* frequencies;
    const int fftSize;
    bool nextFFTBlockReady = false;



private:
    juce::dsp::FFT forwardFFT;                    

    array<float,_fftSize> window;                          
    array<float,_fftSize> fifo;                          
    array<float,_fftSize*2> fftData;                   
    int fifoIndex = 0;                             

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Analyser)
};


