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
    
    void prepareToPlay(float* frequencies, float* fCenters, double fs, int nSamples) 
    {
        // draw raw spectra
        inSpectrum.prepareToPlay(frequencies, in_colour);
        //scSpectrum.prepareToPlay(frequencies, sc_colour);
        outSpectrum.prepareToPlay(frequencies, out_colour);

        // draw masking curves
        //inSpectrum.prepareToPlay(fCenters, in_colour);
        scSpectrum.prepareToPlay(fCenters, sc_colour);  


        inSpectrum.resetSmoothingValues(fs, nSamples);
        scSpectrum.resetSmoothingValues(fs, nSamples);
        outSpectrum.resetSmoothingValues(fs, nSamples);

        deltaSpectrum.prepareToPlay(fCenters, delta_colour);
    }

    void drawNextFrameOfSpectrum(auto& curves, auto& gains_sm) 
    {
        if (numCh == 2) 
        {
            for (int i = 0; i < nfilts; i++) 
            {
                deltaScope[i] = Decibels::gainToDecibels(gains_sm[0][i].getCurrentValue());
            }

            // draw raw spectra
            averageValues(inScope, curves[0].inSpectrum, curves[1].inSpectrum, npoints);
            //averageValues(scScope, curves[0].scSpectrum, curves[1].scSpectrum, npoints);
            averageValues(outScope, curves[0].outSpectrum, curves[1].outSpectrum, npoints);

            // draw masking curves
            //averageValues(inScope, curves[0].inputDecimated, curves[1].inputDecimated, nfilts);
            averageValues(scScope, curves[0].scDecimated, curves[1].scDecimated, nfilts);
        } 
        else
        {
            for (int i = 0; i < nfilts; i++) 
            {
                deltaScope[i] = Decibels::gainToDecibels((gains_sm[0][i].getCurrentValue() + gains_sm[1][i].getCurrentValue()) * 0.5f);
            }

            // draw raw spectra
            FloatVectorOperations::copy(inScope.data(), curves[0].inSpectrum.data(), npoints);
            FloatVectorOperations::multiply(inScope.data(), 2.0f, npoints);
            //FloatVectorOperations::copy(scScope.data(), curves[0].scSpectrum.data(), npoints);
            //FloatVectorOperations::multiply(scScope.data(), 2.0f, npoints);
            FloatVectorOperations::copy(outScope.data(), curves[0].outSpectrum.data(), npoints);
            FloatVectorOperations::multiply(outScope.data(), 2.0f, npoints);

            // draw masking curves
            //FloatVectorOperations::copy(inScope.data(), curves[0].inputDecimated.data(), nfilts);
            FloatVectorOperations::copy(scScope.data(), curves[0].scDecimated.data(), nfilts);
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
    
        float bottomMapped = juce::jmap(float(_spectrumThresh - _spectrumKnee*2), float(_mindBFS), 0.f, 1.f, 0.f) * bounds.getHeight();
        float topMapped = juce::jmap(float(_spectrumThresh + _spectrumKnee*2), float(_mindBFS), 0.f, 1.f, 0.f) * bounds.getHeight();
        
        gradient = ColourGradient(Colour(_darkgrey).withAlpha(0.0f), bounds.getX(), topMapped, _darkgrey, bounds.getX(), bottomMapped, false);
        g.setGradientFill(gradient);
        g.fillRect(bounds);
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
    // draw raw spectra
    SpectrumDrawer inSpectrum;
    //SpectrumDrawer scSpectrum;
    SpectrumDrawer outSpectrum;

    // draw masking curves
    //DeciSpectrumDrawer inSpectrum;
    DeciSpectrumDrawer scSpectrum;
    
    DeltaDrawer deltaSpectrum;

    // draw raw spectra
    array<float, npoints> inScope;
    //array<float, npoints> scScope;
    array<float, npoints> outScope;
    
    // draw masking curves
    //array<float, nfilts> inScope;
    array<float, nfilts> scScope;
    
    array<float, nfilts> deltaScope;

    Converter conv;
    int numCh = 0;

    juce::Colour in_colour = Colour(Colours::white.withAlpha(0.7f));
    juce::Colour sc_colour = _yellow;
    juce::Colour out_colour = Colour(_green).withAlpha(0.7f);
    juce::Colour delta_colour = _purple;
    
    ColourGradient gradient{};
    
    bool hide_in = false;
    bool hide_sc = false;
    bool hide_d = false;
    bool hide_out = false;

};
