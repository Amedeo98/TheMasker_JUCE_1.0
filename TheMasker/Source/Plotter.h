/*
  ==============================================================================

    Plotter.h
    Created: 7 Mar 2023 2:55:18pm
    Author:  nikid

  ==============================================================================
*/

#include "SpectrumDrawer.h"
#include "DeltaDrawer.h"



#pragma once
class Plotter {
public:
    
    void prepareToPlay(float* frequencies, float* fCenters) {
        inSpectrum.prepareToPlay(frequencies, in_colour);
        scSpectrum.prepareToPlay(frequencies, sc_colour);
        outSpectrum.prepareToPlay(frequencies, out_colour);
        deltaSpectrum.prepareToPlay(fCenters, delta_colour);
    }

    void drawNextFrameOfSpectrum(auto& curves) {
        
        if (numCh == 2) {
            averageValues(deltaScope, curves[0].delta, curves[1].delta, nfilts);
            averageValues(inScope, curves[0].inSpectrum, curves[1].inSpectrum, npoints);
            averageValues(scScope, curves[0].scSpectrum, curves[1].scSpectrum, npoints);
            averageValues(outScope, curves[0].outSpectrum, curves[1].outSpectrum, npoints);
        } 
        else 
        {
            FloatVectorOperations::copy(deltaScope.data(), curves[0].delta.data(), nfilts);
            FloatVectorOperations::copy(inScope.data(), curves[0].inSpectrum.data(), npoints);
            FloatVectorOperations::copy(scScope.data(), curves[0].scSpectrum.data(), npoints);
            FloatVectorOperations::copy(outScope.data(), curves[0].outSpectrum.data(), npoints);
        }
                
        inSpectrum.drawNextFrameOfSpectrum(inScope);
        scSpectrum.drawNextFrameOfSpectrum(scScope);
        outSpectrum.drawNextFrameOfSpectrum(outScope);
        deltaSpectrum.drawNextFrameOfSpectrum(deltaScope);
    }
    
    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        inSpectrum.drawFrame(g, bounds);
        scSpectrum.drawFrame(g, bounds);
        outSpectrum.drawFrame(g, bounds);
        deltaSpectrum.drawFrame(g, bounds);
    }

    void setNumChannels(int nCh) {
        numCh = nCh;
    }

    void averageValues(auto& dest, auto& left, auto& right, int size) {
        FloatVectorOperations::fill(dest.data(), 0.0f, size);
        FloatVectorOperations::addWithMultiply(dest.data(), left.data(), 0.5f, size);
        FloatVectorOperations::addWithMultiply(dest.data(), right.data(), 0.5f, size);
    }


private:
    SpectrumDrawer inSpectrum;
    SpectrumDrawer scSpectrum;
    SpectrumDrawer outSpectrum;
    DeltaDrawer deltaSpectrum;

    array<float, npoints> inScope;
    array<float, npoints> scScope;
    array<float, npoints> outScope;
    array<float, nfilts> deltaScope;

    int numCh = 0;

    juce::Colour in_colour = Colour(0.5f, 1.0f, 1.0f, 1.0f);
    juce::Colour sc_colour = Colour(0.07f, 1.0f, 1.0f, 1.0f);
    juce::Colour out_colour = Colour(0.3f, 1.0f, 1.0f, 1.0f);
    juce::Colour delta_colour = Colour(1.0f, 1.0f, 1.0f, 1.0f);

};