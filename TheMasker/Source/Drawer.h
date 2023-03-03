/*
  ==============================================================================

    Drawer.h
    Created: 3 Mar 2023 10:24:17am
    Author:  nikid

  ==============================================================================
*/

#pragma once

class Drawer {
public:

    Drawer(const int nPoints, const int nValues) {
        scopeSize = nPoints;
        resultSize = nValues;
    }

    void prepareToPlay(float* freqs, juce::Colour col) {
        colour = col;
        freqAxis.resize(scopeSize);
        frequencies.resize(scopeSize);
        FloatVectorOperations::copy(frequencies.data(), freqs, scopeSize);
        scope_step = pow(scopeSize, -1);
        scopeData.resize(scopeSize);
        
        for (int i = 0; i < scopeSize; i++)
        {
            freqAxis[i] = juce::mapFromLog10(frequencies[i], (float)minFreq, (float)maxFreq);
            //freqAxis[i] = jmap(frequencies[i], (float)minFreq, (float)maxFreq, 0.0f, 1.0f); //lineare (brutto)
        }


    }






    virtual void drawNextFrameOfSpectrum(vector<float> result)
    {


    }



    virtual void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {

    }


    float mindB;
    float maxdB;

    juce::Colour colour;

    int scopeSize;
    int resultSize;

    vector<float> scopeData;
    vector<float> freqAxis;
    array<float, 2> xVal;


private:

    vector<float> frequencies;
    float scope_step;
    Converter conv;



};