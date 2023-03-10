/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DynamicEQ.h"

//==============================================================================
TheMaskerAudioProcessorEditor::TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    inSliderAttachment(audioProcessor.parameters, NAME_IN, inSlider),
    outSliderAttachment(audioProcessor.parameters, NAME_OUT, outSlider),
    scSliderAttachment(audioProcessor.parameters, NAME_SC, scSlider),
    mixSliderAttachment(audioProcessor.parameters, NAME_MIX, mixSlider),
    compSliderAttachment(audioProcessor.parameters, NAME_COMP, compSlider),
    expSliderAttachment(audioProcessor.parameters, NAME_EXP, expSlider),
    cleanUpSliderAttachment(audioProcessor.parameters, NAME_ATQ, cleanUpSlider)

{
    
    startTimerHz(25);
    
    for(auto* comp : getComponents())
    {
        addAndMakeVisible(comp);
    }
    setSize (700, 500);
    
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
    audioProcessor.dynEQ.drawFrame(g, responseArea);
    
}


void TheMaskerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    bounds.removeFromTop(bounds.getHeight() * 0.15);
    auto in_area = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto out_area = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    inSlider.setBounds(in_area.removeFromTop(out_area.getHeight() * 0.5));
    scSlider.setBounds(in_area);
    compSlider.setBounds(out_area.removeFromTop(out_area.getHeight() * 0.33));
    expSlider.setBounds(out_area.removeFromTop(out_area.getHeight() * 0.5));
    mixSlider.setBounds(out_area);
    outSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    cleanUpSlider.setBounds(bounds);

    inLabel.setText("IN", juce::dontSendNotification);
    inLabel.attachToComponent(&inSlider, false);
    inLabel.setJustificationType(Justification::centred);
    outLabel.setText("OUT", juce::dontSendNotification);
    outLabel.attachToComponent(&outSlider, false);
    outLabel.setJustificationType(Justification::centred);
    scLabel.setText("SC", juce::dontSendNotification);
    scLabel.setJustificationType(Justification::centred);
    scLabel.attachToComponent(&scSlider, false);
    compLabel.setText("COMP", juce::dontSendNotification);
    compLabel.setJustificationType(Justification::centred);
    compLabel.attachToComponent(&compSlider, false);
    expLabel.setText("EXP", juce::dontSendNotification);
    expLabel.setJustificationType(Justification::centred);
    expLabel.attachToComponent(&expSlider, false);
    cleanUpLabel.setText("Clean Up", juce::dontSendNotification);
    cleanUpLabel.attachToComponent(&cleanUpSlider, false);
    cleanUpLabel.setJustificationType(Justification::centred);
    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider, false);
    mixLabel.setJustificationType(Justification::centred);
    
}


std::vector<juce::Component*> TheMaskerAudioProcessorEditor::getComponents()
{
    return
    {
       &inSlider,& outSlider,& mixSlider,
        & scSlider,& compSlider,& expSlider,& cleanUpSlider,
        & inLabel,& outSlider,& mixLabel,
        & scLabel,& compLabel,& expLabel,& cleanUpLabel
    };
}
