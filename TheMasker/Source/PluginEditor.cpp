/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheMaskerAudioProcessorEditor::TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    for(auto* comp : getComponents())
    {
        addAndMakeVisible(comp);
    }
    setSize (600, 400);
    
}


TheMaskerAudioProcessorEditor::~TheMaskerAudioProcessorEditor()
{
}


//==============================================================================
void TheMaskerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight()*0.33);
    auto w = responseArea.getWidth();
    
    //aggiungi valori numerici
    
    auto sampleRate = audioProcessor.getSampleRate();
    std:vector<double> mags;
    
    mags.resize(w);
    
    for(int i = 0; i < w; i ++)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i)/double(w), 20.0, 20000.0);
        //qui viene modificata la curva
    }
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        //mapping db to screen coordinates
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for( size_t i=1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}


void TheMaskerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto in_area = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto out_area = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    inSlider.setBounds(in_area);
    
    compSlider.setBounds(out_area.removeFromTop(out_area.getHeight() * 0.33));
    expSlider.setBounds(out_area.removeFromTop(out_area.getHeight() * 0.5));
    cleanUpSlider.setBounds(out_area);
    
    outSlider.setBounds(bounds);
    
}


std::vector<juce::Component*> TheMaskerAudioProcessorEditor::getComponents()
{
    return
    {
        &inSlider, &outSlider,
        &scSlider, &compSlider, &expSlider, &cleanUpSlider
    };
}
