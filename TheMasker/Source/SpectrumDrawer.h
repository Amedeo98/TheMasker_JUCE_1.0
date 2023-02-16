/*
  ==============================================================================

    SpectrumDrawer.h
    Created: 15 Feb 2023 1:29:59pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

class SpectrumDrawer {
public:


    void prepareToPlay(vector<float> freqs, vector<float> fCents, juce::Colour col, float fs) {
        colour = col;
        frequencies.resize(_fftSize);
        fCents.resize(nfilts);
        freqAxis.resize(_fftSize);
        frequencies = freqs;
        fCenters = fCents;
        sampleRate = fs;
        //logAxis = conv.linspace(1.0f, (float) _fftSize, (float) _fftSize);

        


        //p.startNewSubPath(bounds.getX() + factor * indexToX(0, minFreq), binToY(fftData[0], bounds));
        //for (int i = 0; i < averager.getNumSamples(); ++i)
        //    p.lineTo(bounds.getX() + factor * indexToX(float(i), minFreq), binToY(fftData[i], bounds));
       

        //float normalizer = pow((float)maxFreq,-1);
        //float log_normalizer = pow(log10(_fftSize), -1);
        for (int i = 0; i < _fftSize; i++)
        {
            freqAxis[i] = juce::mapFromLog10(frequencies[i], (float)minFreq, (float)maxFreq);
        }

    }

    vector<float> getXs(const std::vector<float>& freqs, float left, float width)
    {
        std::vector<float> xs;
        for (auto f : freqs)
        {
            auto normX = juce::mapFromLog10(f, (float) minFreq, (float) maxFreq);
            //xs.push_back(left + width * normX);
            xs.push_back(normX);
        }

        return xs;
    }



  

    void drawNextFrameOfSpectrum(vector<float> result)
    {


        for (int i = 0; i < scopeSize; ++i)                         // [3]
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, _fftSize / 2, (int)(skewedProportionX * (float)_fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(result[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)_fftSize)
            ), mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;                                   // [4]
        }
    }



    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        auto left = bounds.getX();
        //freqAxis = getXs(frequencies, left , width);
        for (int i = 1; i < scopeSize; ++i)
        {

            g.setColour(colour);
            //vector<float> xVal{ , };
            vector<float> xVal = { jmap( freqAxis[i-1] , 0.f, 1.f, (float) left, (float)width),
                                   jmap( freqAxis[i] , 0.f, 1.f, (float) left, (float)width)};
            
            
            /*   vector<float> xVal = { (float)juce::jmap( frequencies[i-1], (float) minFreq, (float) maxFreq, 0.0f, (float) width),
                                     (float)juce::jmap(  frequencies[i], (float) minFreq, (float) maxFreq, 0.0f, (float) width) };*/
            g.drawLine( xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                          xVal[1], jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) );
        }
    }

    float toLog(float x) {
        return log10(x);
        //return mapFromLog10<float>(x, log10(minFreq), log10(maxFreq));
    }
private:
    vector<float> freqAxis;
    vector<float> frequencies;
    vector<float> fCenters;
    float scope_step = pow(_fftSize,-1);
    Converter conv;
    float mindB = -100.0f;
    float sampleRate;
    float maxdB = 0.0f;
    juce::Colour colour;
    int scopeSize = _fftSize;
    float scopeData[_fftSize];
};