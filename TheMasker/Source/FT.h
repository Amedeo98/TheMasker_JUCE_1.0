/*
  ==============================================================================

    FT.h
    Created: 20 Jan 2023 6:17:33pm
    Author:  nikid

  ==============================================================================
*/

#include "OverlappingFFTProcessor.h"


#pragma once

class AnalyserComponent   : public juce::AudioAppComponent,
                            private juce::Timer
{
public:
    enum
    {
        fftOrder  = 11,             // [1] (=> 2^10 = 1024 points)
        fftSize   = 1 << fftOrder,  // [2]
        scopeSize = 512             // [3]
    };
    
    AnalyserComponent()
        : forwardFFT (fftOrder),
          window (fftSize, juce::dsp::WindowingFunction<float>::hann) //hanning
    {
        setOpaque (true);
        setAudioChannels (2, 0);  // we want a couple of input channels but no outputs
        startTimerHz (30);
        setSize (700, 500);
    }

    ~AnalyserComponent() override
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int, double) override {}
    void releaseResources() override          {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
     {
         if (bufferToFill.buffer->getNumChannels() > 0)
         {
             auto* channelData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
  
             for (auto i = 0; i < bufferToFill.numSamples; ++i)
                 pushNextSampleIntoFifo (channelData[i]);
         }
     }
    

    //==============================================================================
    /*void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        g.setOpacity (1.0f);
        g.setColour (juce::Colours::white);
        drawFrame (g);
    }*/

    //repaint callback
    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum();
            nextFFTBlockReady = false;
            
        }
    }

    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
        if (fifoIndex == fftSize)               // [11]
        {
            if (! nextFFTBlockReady)            // [12]
            {
                juce::zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
                nextFFTBlockReady = true;
            }
 
            fifoIndex = 0;
        }
 
        fifo[fifoIndex++] = sample;             // [12]
    }

   
    void drawNextFrameOfSpectrum()
    {
        // first apply a windowing function to our data
        window.multiplyWithWindowingTable (fftData, fftSize);       // [1]
 
        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform (fftData);  // [2]
 
        auto mindB = -100.0f;
        auto maxdB =    0.0f;
 
        for (int i = 0; i < scopeSize; ++i)                         // [3]
        {
            auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
            auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (fftData[fftDataIndex])
                                                               - juce::Decibels::gainToDecibels ((float) fftSize)),
                                     mindB, maxdB, 0.0f, 1.0f);
 
            scopeData[i] = level;                                   // [4]
        }
    }
    

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto width = bounds.getWidth();
            auto height = bounds.getHeight();
            g.drawLine ({ (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
                                  juce::jmap (scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                          (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
                                  juce::jmap (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) });
        }
    }


private:
    juce::dsp::FFT forwardFFT;                      // [4]
    juce::dsp::WindowingFunction<float> window;     // [5]
 
    float fifo [fftSize];                           // [6]
    float fftData [2 * fftSize];                    // [7]
    int fifoIndex = 0;                              // [8]
    bool nextFFTBlockReady = false;                 // [9]
    float scopeData [scopeSize];                    // [10]
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};




//==============================================================================
//==============================================================================

class FT : public OverlappingFFTProcessor, public AnalyserComponent {

public:
    FT() : OverlappingFFTProcessor(_fftOrder, hopSizeDividerAsPowOf2)
    {
        //startTimerHz (30);
        result.resize(pow(2, _fftOrder - hopSizeDividerAsPowOf2));
        result_decim.resize(nfilts);
    }
    ~FT() {}
    
    
    auto getFT(AudioBuffer<float>& input, int ch)  {
        process(input);
        result = getResult();
        if (decimated)
        result_decim = conv.mXv_mult(fbank_values, result);

        //drawNextFrameOfSpectrum();


        return decimated ? result_decim : result;
    }
    
    /*void timerCallback() override
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }*/

    

    void setFBank(FilterBank fb) {
        fbank_values = fb.getValues();
        decimated = true;
    }

    /*void drawNextFrameOfSpectrum()
    {
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(result[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)fftSize)),
                mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }        */
     
     
     //CHIAMARE SU PLUGIN EDITOR!!!
    /*
     void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
     {
         for (int i = 1; i < scopeSize; ++i)
         {
             auto width = bounds.getWidth();
             auto height = bounds.getHeight();

             g.drawLine({ (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width),
                                   juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                           (float)juce::jmap(i,     0, scopeSize - 1, 0, width),
                                   juce::jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
             }
        }
    */


    vector<float> result;
    vector<float> result_decim;

private:
    int scopeSize = npoints;
    float scopeData[npoints];
    float mindB = -100.0f;
    float maxdB = 0.0f;
    vector<vector<float>> fbank_values;
    Converter conv;
    bool decimated = false;
    bool nextFFTBlockReady = false;
  

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FT)
};


