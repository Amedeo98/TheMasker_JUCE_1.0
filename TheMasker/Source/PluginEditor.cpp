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
    auto bounds = Rectangle<float>(x, y, width, height);
    
    g.setColour(Colour(97u, 18u, 167u));
    g.fillEllipse(bounds);

    g.setColour(Colour(64u, 200u, 64u));
    g.drawEllipse(bounds, 5.f);


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
    setSize (800, 500);
    
}


TheMaskerAudioProcessorEditor::~TheMaskerAudioProcessorEditor()
{
}


//==============================================================================
void TheMaskerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::black);
    auto bounds = getLocalBounds();
    
    //draw input area
    auto in_area = bounds.removeFromLeft(getWidth() * 0.08);
    g.setColour (bgColorDark);
    g.fillRect (in_area);
    
    inSlider.setBounds(in_area.removeFromTop(in_area.getWidth()));
    stereoLinkedSlider.setBounds(in_area.removeFromBottom(in_area.getWidth()));
    
    
    //draw controls area
    auto controls_area = bounds.removeFromLeft(getWidth() * 0.15);
    g.setColour (bgColorLight);
    g.fillRect (controls_area);
    
    auto nameBox = controls_area.removeFromTop(getHeight() * 0.15);
    g.setColour (Colours::white);
    g.fillRect (nameBox);
    
    scSlider.setBounds(controls_area.removeFromTop(controls_area.getWidth()));
    compSlider.setBounds(controls_area.removeFromTop(getHeight() * 0.15));
    expSlider.setBounds(controls_area.removeFromTop(getHeight() * 0.15));
    
    
    //draw output area
    auto out_area = bounds.removeFromRight(getWidth() * 0.08);
    g.setColour (bgColorDark);
    g.fillRect (out_area);
    
    mixSlider.setBounds(out_area.removeFromTop(out_area.getWidth()));
    outSlider.setBounds(out_area.removeFromBottom(out_area.getWidth()));
    
    
    //draw spectrum area
    auto responseArea = bounds;
    g.setColour (juce::Colours::black);
    g.fillRect (responseArea);
    
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

