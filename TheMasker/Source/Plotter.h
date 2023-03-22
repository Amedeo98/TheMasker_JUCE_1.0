/*
  ==============================================================================

    Plotter.h
    Created: 7 Mar 2023 2:55:18pm
    Author:  nikid

  ==============================================================================
*/

#include "SpectrumDrawer.h"
#include "DeltaDrawer.h"
#include <JuceHeader.h>


#pragma once
class Plotter {
public:
    
    void prepareToPlay(float* frequencies, float* fCenters) {
        inSpectrum.prepareToPlay(frequencies, in_colour);
        scSpectrum.prepareToPlay(frequencies, sc_colour);
        outSpectrum.prepareToPlay(frequencies, out_colour);
        deltaSpectrum.prepareToPlay(fCenters, delta_colour);
    }

    void drawNextFrameOfSpectrum(array<float,npoints> in, array<float, npoints> sc, array<float, npoints> out, array<SmoothedValue<float, ValueSmoothingTypes::Linear>, nfilts> delta) {
        inSpectrum.drawNextFrameOfSpectrum(in);
        scSpectrum.drawNextFrameOfSpectrum(sc);
        outSpectrum.drawNextFrameOfSpectrum(out);
        deltaSpectrum.drawNextFrameOfSpectrum(delta);
    }
    
    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds)
    {
        inSpectrum.drawFrame(g, bounds);
        scSpectrum.drawFrame(g, bounds);
        outSpectrum.drawFrame(g, bounds);
        deltaSpectrum.drawFrame(g, bounds);
    }


private:
    SpectrumDrawer inSpectrum;
    SpectrumDrawer scSpectrum;
    SpectrumDrawer outSpectrum;
    DeltaDrawer deltaSpectrum;

    juce::Colour in_colour = Colour(0.5f, 1.0f, 1.0f, 1.0f);
    juce::Colour sc_colour = Colour(0.07f, 1.0f, 1.0f, 1.0f);
    juce::Colour out_colour = Colour(0.3f, 1.0f, 1.0f, 1.0f);
    juce::Colour delta_colour = Colour(1.0f, 1.0f, 1.0f, 1.0f);

};