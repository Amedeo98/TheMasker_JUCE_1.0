/*
  ==============================================================================

    Plotter.h
    Created: 7 Mar 2023 2:55:18pm
    Author:  nikid

  ==============================================================================
*/

#include "SpectrumDrawer.h"
#include "DeltaDrawer.h"
#include "Constants.h"



#pragma once
class Plotter {
public:
    
    void prepareToPlay(float* frequencies, float* fCenters) {
        inSpectrum.prepareToPlay(frequencies, in_colour);
        scSpectrum.prepareToPlay(frequencies, sc_colour);
        outSpectrum.prepareToPlay(frequencies, out_colour);
        deltaSpectrum.prepareToPlay(fCenters, delta_colour);
    }

    void drawNextFrameOfSpectrum(auto& curves, auto& gains_sm) {
        
        if (numCh == 2) {
            
            for (int i = 0; i < nfilts; i++) {
                deltaScope[i] = Decibels::gainToDecibels(gains_sm[0][i].getCurrentValue());

            }
            //averageValues(deltaScope, gains_sm[0], gains_sm[1], nfilts);
            averageValues(inScope, curves[0].inSpectrum, curves[1].inSpectrum, npoints);
            averageValues(scScope, curves[0].scSpectrum, curves[1].scSpectrum, npoints);
            averageValues(outScope, curves[0].outSpectrum, curves[1].outSpectrum, npoints);
        } 
        else
        {
            for (int i = 0; i < nfilts; i++) {
                deltaScope[i] = Decibels::gainToDecibels((gains_sm[0][i].getCurrentValue() + gains_sm[1][i].getCurrentValue()) * 0.5f);
            }
            //FloatVectorOperations::copy(deltaScope.data(), gains_sm[0].data(), nfilts);
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

    Converter conv;

    array<float, npoints> inScope;
    array<float, npoints> scScope;
    array<float, npoints> outScope;
    array<float, nfilts> deltaScope;

    int numCh = 0;

    juce::Colour in_colour = Colour(Colours::white.withAlpha(0.7f));
    juce::Colour sc_colour = _yellow;
    juce::Colour out_colour = _green;
    juce::Colour delta_colour = _purple;
    
    bool hide_in = false;
    bool hide_sc = false;
    bool hide_d = false;
    bool hide_out = false;

};
