/*
  ==============================================================================

    Plotter.h
    Created: 7 Mar 2023 2:55:18pm
    Author:  nikid

  ==============================================================================
*/

#include "SpectrumDrawer.h"
#include "DeltaDrawer.h"
//#include <JuceHeader.h>
//#include "CustomSmoothedValue.h"



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
        if(!hide_in){
            inSpectrum.drawFrame(g, bounds);
        }
        if(!hide_sc){
            scSpectrum.drawFrame(g, bounds);
        }
        if(!hide_d){
            deltaSpectrum.drawFrame(g, bounds);
        }
        if(!hide_out){
            outSpectrum.drawFrame(g, bounds);
        }
    }

    void setNumChannels(int nCh) {
        numCh = nCh;
    }

    void averageValues(auto& dest, auto& left, auto& right, int size) {
        FloatVectorOperations::fill(dest.data(), 0.0f, size);
        FloatVectorOperations::addWithMultiply(dest.data(), left.data(), 0.5f, size);
        FloatVectorOperations::addWithMultiply(dest.data(), right.data(), 0.5f, size);
    }
    
    void toggleSpectrumView(juce::String btn)
    {
        if(btn == "in"){
            hide_in = !hide_in;
        }
        else if(btn == "sc"){
            hide_sc = !hide_sc;
        }
        else if(btn == "delta"){
            hide_d = !hide_d;
        }
        else if(btn == "out"){
            hide_out = !hide_out;
        }
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

    juce::Colour in_colour = Colour(Colours::white.withAlpha(0.7f));
    juce::Colour sc_colour = Colour(255u, 200u, 100u);
    juce::Colour out_colour = Colour(40u, 220u, 0u);
    juce::Colour delta_colour = Colour(200u, 40u, 160u);
    
    bool hide_in = false;
    bool hide_sc = false;
    bool hide_d = false;
    bool hide_out = false;

};
