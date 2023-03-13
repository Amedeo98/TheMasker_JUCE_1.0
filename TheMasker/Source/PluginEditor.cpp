/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DynamicEQ.h"


void LnF::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    const float rotaryStartAngle,
    const float rotaryEndAngle,
    juce::Slider& slider)
{
    int radius = jmin(width, height);
    float centreX = float(sliderImg.getWidth()) * .5f;
    float centreY = float(sliderImg.getHeight()) * .5f;
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    auto rotation = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    sliderImg = sliderImg.rescaled(radius, radius, Graphics::ResamplingQuality::highResamplingQuality);

    AffineTransform transform;

    transform = transform.rotation(rotation, centreX, centreY);
    g.drawImageTransformed(sliderImg, transform, false);


}

//====================================================================================================
void CustomRotarySlider::paint(juce::Graphics& g)
{
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
}


juce::Rectangle<int> CustomRotarySlider::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}



//==============================================================================
TheMaskerAudioProcessorEditor::TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    inSlider(*audioProcessor.parameters.getParameter(NAME_IN), NAME_IN),
    outSlider(*audioProcessor.parameters.getParameter(NAME_OUT), NAME_OUT),
    scSlider(*audioProcessor.parameters.getParameter(NAME_SC), NAME_SC),
    compSlider(*audioProcessor.parameters.getParameter(NAME_COMP), NAME_COMP),
    expSlider(*audioProcessor.parameters.getParameter(NAME_EXP), NAME_EXP),
    mixSlider(*audioProcessor.parameters.getParameter(NAME_MIX), NAME_MIX),
    stereoLinkedSlider(*audioProcessor.parameters.getParameter(NAME_SL), NAME_SL),
    cleanUpSlider(*audioProcessor.parameters.getParameter(NAME_ATQ), NAME_ATQ),


    inSliderAttachment(audioProcessor.parameters, NAME_IN, inSlider),
    outSliderAttachment(audioProcessor.parameters, NAME_OUT, outSlider),
    scSliderAttachment(audioProcessor.parameters, NAME_SC, scSlider),
    mixSliderAttachment(audioProcessor.parameters, NAME_MIX, mixSlider),
    compSliderAttachment(audioProcessor.parameters, NAME_COMP, compSlider),
    expSliderAttachment(audioProcessor.parameters, NAME_EXP, expSlider),
    stereoLinkedSliderAttachment(audioProcessor.parameters, NAME_SL, stereoLinkedSlider),
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
    g.fillAll (Colours::orange);
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto w = responseArea.getWidth();
    
    g.setColour (juce::Colours::black);
    g.fillRect (responseArea);
    
    bounds.removeFromTop(bounds.getHeight() * 0.15);
    auto in_area = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto out_area = bounds.removeFromRight(bounds.getWidth() * 0.5);

    inSlider.setBounds(in_area.removeFromTop(out_area.getHeight() * 0.5));
    scSlider.setBounds(in_area);
    compSlider.setBounds(out_area.removeFromTop(out_area.getHeight() * 0.33));
    expSlider.setBounds(out_area.removeFromTop(out_area.getHeight() * 0.5));
    mixSlider.setBounds(out_area);
    outSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    cleanUpSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    stereoLinkedSlider.setBounds(bounds);

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
    stereoLabel.setText("STEREO", juce::dontSendNotification);
    stereoLabel.attachToComponent(&stereoLinkedSlider, false);
    stereoLabel.setJustificationType(Justification::centred);
    

    
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
    
    //responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for( size_t i=1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    /*g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));*/
    audioProcessor.dynEQ.drawFrame(g, responseArea);
    
}


void TheMaskerAudioProcessorEditor::resized()
{
}


std::vector<juce::Component*> TheMaskerAudioProcessorEditor::getComponents()
{
    return
    {
       &inSlider,& outSlider,& mixSlider,
        & scSlider,& compSlider,& expSlider,& cleanUpSlider,
        & inLabel,& outSlider,& mixLabel,
        & scLabel,& compLabel,& expLabel,& cleanUpLabel,& stereoLinkedSlider
    };
}

