/*
  ==============================================================================

    Drawer.h
    Created: 3 Mar 2023 10:24:17am
    Author:  nikid

  ==============================================================================
*/

#pragma once

class Drawer {
//public:
//    Drawer(const int _scopeSize, const int nValues)
//    {
//        const int scopeSize = _scopeSize;
//        float scopeData[_scopeSize];
//        scope_step = pow(scopeSize, -1);
//
//    }
//
//    void prepareToPlay(juce::Colour col) {
//        colour = col;
//        fCenters = fCents;
//        //freqAxis.resize(npoints);
//        //frequencies = freqs;
//
//        
//
//    }
//
//
//
//
//
//
//    void drawNextFrameOfSpectrum(vector<float> result)
//    {
//
//
//        for (int i = 0; i < scopeSize; ++i)
//        {
//            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
//            auto fftDataIndex = juce::jlimit(0, _fftSize / 2, (int)(skewedProportionX * (float)_fftSize * 0.5f));
//            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(result[fftDataIndex])
//                - juce::Decibels::gainToDecibels((float)_fftSize)
//            ), mindB, maxdB, 0.0f, 1.0f);
//
//            scopeData[i] = level;
//        }
//    }
//
//
//
//    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
//    {
//        auto width = bounds.getWidth();
//        auto height = bounds.getHeight();
//        auto left = bounds.getX();
//        for (int i = 1; i < scopeSize; ++i)
//        {
//
//            g.setColour(colour);
//            vector<float> xVal = { jmap(freqAxis[i - 1] , 0.f, 1.f, (float)left, (float)width),
//                                   jmap(freqAxis[i] , 0.f, 1.f, (float)left, (float)width) };
//
//
//            /*   vector<float> xVal = { (float)juce::jmap( frequencies[i-1], (float) minFreq, (float) maxFreq, 0.0f, (float) width),
//                                     (float)juce::jmap(  frequencies[i], (float) minFreq, (float) maxFreq, 0.0f, (float) width) };*/
//            g.drawLine(xVal[0], jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
//                xVal[1], jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f));
//        }
//    }
//
//    float mindB = _mindBFS;
//    float maxdB = 0.0f;
//    vector<float> freqAxis;
//    array<float, npoints> frequencies;
//private:
//
//    //array<float, nfilts> fCenters;
//    //array<float,nfilts> fCenters;
//    float scope_step;
//    //int scopeSize;
//
//
//    juce::Colour colour;

};