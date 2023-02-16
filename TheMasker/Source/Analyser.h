/*
  ==============================================================================

    BlockAnalyser.h
    Created: 13 Feb 2023 10:05:06pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include "Converters.h"


class Analyser
{
public:

    Analyser(const int fftOrder, const int _npoints)
        : forwardFFT (fftOrder), fftSize (1 << fftOrder)
    {
        
        window.resize(fftSize);
        dsp::WindowingFunction<float>::fillWindowingTables(window.data(), fftSize, dsp::WindowingFunction<float>::WindowingMethod::hann, false);
        fifo.resize(fftSize);                          
        fftData.resize(2*fftSize);                          
        result.resize(fftSize);
    }



    //void prepare(vector<float> freqs) {
    //    frequencies = freqs;
    //}

    void process(AudioBuffer<float>& bufferToFill, int ch)
    {
        if (bufferToFill.getNumChannels() > 0)
        {
            auto* channelData = bufferToFill.getReadPointer(ch, 0);

            for (auto i = 0; i < bufferToFill.getNumSamples(); ++i)
                pushNextSampleIntoFifo(channelData[i]);
        }
    }





    void getResult(vector<float>& result)
    {
        if (nextFFTBlockReady)
        {
            
            FloatVectorOperations::multiply(fftData.data(), window.data(), fftSize);

            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data(), false);  // [2]

            FloatVectorOperations::copy(result.data(), fftData.data(), fftSize);



            nextFFTBlockReady = false;


        }




    }

    void pushNextSampleIntoFifo(float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
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


   

    vector<float> result;
    vector<float> frequencies;
    const int fftSize;



private:
    juce::dsp::FFT forwardFFT;                    
    Converter conv;

    vector<float> window;                          
    vector<float> fifo;                          
    vector<float> fftData;                   
    int fifoIndex = 0;                             
    bool nextFFTBlockReady = false;                

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Analyser)
};


