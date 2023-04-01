/*
  ==============================================================================

    Sliders.cpp
    Created: 31 Mar 2023 11:13:22pm
    Author:  Amedeo Fresia

  ==============================================================================
*/

#include "Sliders.h"
#include "Constants.h"


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
    auto sliderArea = bounds;
    auto center = bounds.getCentre();
    auto radius = width * 0.5;
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    
    //greyed part for amount indicator
    Path bkg;
    g.setColour(Colours::grey.withAlpha(0.8f));
    bkg.addCentredArc(center.getX(), center.getY(), radius, radius, 0, rotaryStartAngle, rotaryEndAngle, true);
    auto bkgStroke = PathStrokeType(4.0, juce::PathStrokeType::JointStyle::curved);
    bkgStroke.createStrokedPath(bkg, bkg);
    g.fillPath(bkg);
    
    //amount indicator
    Path amt;

    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
     
    if(auto* s = dynamic_cast<CustomRotarySlider*>(&slider)) {
        if(s->sliderName == NAME_CLEARF || s->sliderName == NAME_MASKEDF)
        {
            auto avg = (rotaryEndAngle+rotaryStartAngle)*0.5f;
            if(sliderAngRad<avg)
            {
                g.setColour(_purple);
                amt.addCentredArc(center.getX(), center.getY(), radius, radius, 0, sliderAngRad, avg, true);
            }
            else
            {
                g.setColour(Colour(64u, 200u, 64u));
                amt.addCentredArc(center.getX(), center.getY(), radius, radius, 0, avg, sliderAngRad, true);
            }
        }
        else
        {
            g.setColour(Colour(64u, 200u, 64u));
            amt.addCentredArc(center.getX(), center.getY(), radius, radius, 0, rotaryStartAngle, sliderAngRad, true);
        }
    }
    
    auto amtStroke = PathStrokeType(4.0, juce::PathStrokeType::JointStyle::curved);
    amtStroke.createStrokedPath(amt, amt);
    g.fillPath(amt);
}

void LnF::drawLinearSlider(Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const Slider::SliderStyle,
                           juce::Slider& slider)
{
    auto bounds = Rectangle<float>(x, y, width, height);
    auto center = bounds.getCentre();
    
    minSliderPos = x+12;
    maxSliderPos = width-24;
    
    //slider background
    Path bkg;
    g.setColour(Colours::white);
    bkg.addRoundedRectangle(minSliderPos, center.getY(), maxSliderPos, 6, 4.0f);
    auto bkgStroke = PathStrokeType(8.0);
    g.fillPath(bkg);
    
    //slider amount
    Path amt;
    g.setColour(_green);
    auto currentPos = jmap(sliderPos, minSliderPos, maxSliderPos);

    amt.addRoundedRectangle(minSliderPos, center.getY(), currentPos, 6, 4.0f);
    auto amtStroke = PathStrokeType(8.0);
    g.fillPath(amt);
}



//====================================================================================================
void CustomRotarySlider::paint(juce::Graphics& g)
{
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();
    auto bounds = getLocalBounds();
    
    //set slider value
    bounds.removeFromBottom(8);
    if(displayValue)
    {
        auto valueArea = bounds.removeFromBottom(20);
        g.setColour(Colours::white.withAlpha(0.7f));
        g.fillRoundedRectangle(valueArea.getCentreX()-24, valueArea.getY(), 48, 20,  8.0f);
        
        auto currentValue = getDisplayString();
        g.setColour(Colours::black);
        g.drawFittedText(currentValue, valueArea.toNearestInt(), juce::Justification::centred, 1);
    }
    
    //draw a square with margin for the slider
    auto sliderBounds = getSliderBounds(bounds);

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


juce::Rectangle<int> CustomRotarySlider::getSliderBounds(juce::Rectangle<int> bounds) const
{
    //auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight();
    
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), bounds.getCentreY());

    return r;
}


juce::String CustomRotarySlider::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    str = juce::String(juce::roundToInt(getValue()));

    return str;
}


//================================================================================
void CustomLinearSlider::paint(Graphics& g)
{
    auto bounds = getLocalBounds();
    auto range = getRange();
    
    getLookAndFeel().drawLinearSlider(g,
                                      bounds.getX(),
                                      bounds.getY(),
                                      bounds.getWidth(),
                                      bounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      0.0,
                                      1.0,
                                      SliderStyle::LinearHorizontal,
                                      *this);
    
}

