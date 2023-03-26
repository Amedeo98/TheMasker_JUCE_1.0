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
    auto sliderArea = bounds;
    auto center = bounds.getCentre();
    auto radius = width * 0.5;
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    
    //greyed part for amount indicator
    Path bkg;
    g.setColour(Colours::black.withAlpha(0.2f));
    bkg.addCentredArc(center.getX(), center.getY(), radius, radius, 0, rotaryStartAngle, rotaryEndAngle, true);
    auto bkgStroke = PathStrokeType(8.0, juce::PathStrokeType::JointStyle::curved);
    bkgStroke.createStrokedPath(bkg, bkg);
    g.fillPath(bkg);
    
    //amount indicator
    Path amt;
    g.setColour(Colour(64u, 200u, 64u));
    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
    amt.addCentredArc(center.getX(), center.getY(), radius, radius, 0, rotaryStartAngle, sliderAngRad, true);
    auto amtStroke = PathStrokeType(8.0, juce::PathStrokeType::JointStyle::curved);
    amtStroke.createStrokedPath(amt, amt);
    g.fillPath(amt);
    
    //knob
    g.setColour(Colour(120u, 18u, 167u));
    g.fillEllipse(bounds);
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
                           Slider &)
{
    auto bounds = Rectangle<float>(x, y, width, height);
    auto center = bounds.getCentre();
    
    minSliderPos = x+8;
    maxSliderPos = width-16;
    
    //slider background
    Path bkg;
    g.setColour(Colours::white.withAlpha(0.8f));
    bkg.addRoundedRectangle(minSliderPos, center.getY(), maxSliderPos, 6, 4.0f);
    auto bkgStroke = PathStrokeType(8.0);
    g.fillPath(bkg);
    
    //slider amount
    Path amt;
    g.setColour(Colour(40u, 220u, 0u));
    auto currentPos = jmap(sliderPos, minSliderPos, maxSliderPos);
    amt.addRoundedRectangle(minSliderPos, center.getY(), currentPos, 6, 4.0f);
    auto amtStroke = PathStrokeType(8.0);
    g.fillPath(amt);
    
}


void CustomLinearSlider::paint(Graphics& g)
{
    auto bounds = getLocalBounds();
    auto range = getRange();
    
    g.setColour(Colours::black);
    g.drawFittedText(sliderName, bounds.toNearestInt(), juce::Justification::centredTop, 1);
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

//====================================================================================================
void CustomRotarySlider::paint(juce::Graphics& g)
{
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();
    auto bounds = getLocalBounds();
    
    //set slider name
    g.setColour(Colours::black);
    g.drawFittedText(sliderName, bounds.toNearestInt(), juce::Justification::centredTop, 1);
    bounds.removeFromTop(12);
    
    //set slider value
    bounds.removeFromBottom(8);
    auto valueArea = bounds.removeFromBottom(20);
    if(displayValue)
    {
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


//==============================================================================
TheMaskerAudioProcessorEditor::TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    inSlider(*audioProcessor.parameters.getParameter(NAME_IN), NAME_IN, true),
    outSlider(*audioProcessor.parameters.getParameter(NAME_OUT), NAME_OUT, true),
    scSlider(*audioProcessor.parameters.getParameter(NAME_SC), NAME_SC, true),
    compSlider(*audioProcessor.parameters.getParameter(NAME_COMP), NAME_COMP, false),
    expSlider(*audioProcessor.parameters.getParameter(NAME_EXP), NAME_EXP, false),
    mixSlider(*audioProcessor.parameters.getParameter(NAME_MIX), NAME_MIX, false),
    stereoLinkedSlider(*audioProcessor.parameters.getParameter(NAME_SL), NAME_SL, false),
    cleanUpSlider(*audioProcessor.parameters.getParameter(NAME_ATQ), NAME_ATQ, false),

    inSliderAttachment(audioProcessor.parameters, NAME_IN, inSlider),
    outSliderAttachment(audioProcessor.parameters, NAME_OUT, outSlider),
    scSliderAttachment(audioProcessor.parameters, NAME_SC, scSlider),
    mixSliderAttachment(audioProcessor.parameters, NAME_MIX, mixSlider),
    compSliderAttachment(audioProcessor.parameters, NAME_COMP, compSlider),
    expSliderAttachment(audioProcessor.parameters, NAME_EXP, expSlider),
    cleanUpSliderAttachment(audioProcessor.parameters, NAME_ATQ, cleanUpSlider),
    stereoLinkedSliderAttachment(audioProcessor.parameters, NAME_SL, stereoLinkedSlider)
{
    
    startTimerHz(25);
    
    for(auto* comp : getComponents())
    {
        addAndMakeVisible(comp);
    }
    
    undoButton.setButtonText("undo");
    undoButton.addListener(this);
    redoButton.setButtonText("redo");
    redoButton.addListener(this);
    loadButton.setButtonText("load");
    loadButton.addListener(this);
    saveButton.setButtonText("save");
    saveButton.addListener(this);
    
    setSize (800, 500);
    
}


TheMaskerAudioProcessorEditor::~TheMaskerAudioProcessorEditor()
{
    undoButton.removeListener(this);
    redoButton.removeListener(this);
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
    in_area.removeFromTop(8);
    inSlider.setBounds(in_area.removeFromTop(in_area.getWidth()*1.4f));
    //stereoLinkedSlider.setBounds(in_area.removeFromBottom(in_area.getWidth()*1.4f));
    
    undoButton.setBounds(in_area.removeFromBottom(in_area.getWidth()*0.25f));
    redoButton.setBounds(in_area.removeFromBottom(in_area.getWidth()*0.25f));
    loadButton.setBounds(in_area.removeFromBottom(in_area.getWidth()*0.25f));
    saveButton.setBounds(in_area.removeFromBottom(in_area.getWidth()*0.25f));
    
    //draw controls area
    auto controls_area = bounds.removeFromLeft(getWidth() * 0.15);
    g.setColour (bgColorLight);
    g.fillRect (controls_area);
    
    auto nameBox = controls_area.removeFromTop(getHeight() * 0.15);
    g.setColour (Colours::white);
    g.fillRoundedRectangle (nameBox.getX()+8, nameBox.getY()+8,nameBox.getWidth()-16, nameBox.getHeight()-16, 8.0);
    g.setFont(20.f);
    g.setColour (Colours::black);
    g.drawFittedText(PLUGIN_NAME, nameBox.toNearestInt(), juce::Justification::centred, 1);
    
    scSlider.setBounds(controls_area.removeFromTop(controls_area.getWidth()));
    compSlider.setBounds(controls_area.removeFromTop(getHeight() * 0.2));
    expSlider.setBounds(controls_area.removeFromTop(getHeight() * 0.2));
    
    controls_area.removeFromBottom(getHeight() * 0.1);
    cleanUpSlider.setBounds(controls_area.removeFromBottom(getHeight() * 0.1));
    
    
    //draw output area
    auto out_area = bounds.removeFromRight(getWidth() * 0.08);
    g.setColour (bgColorDark);
    g.fillRect (out_area);
    out_area.removeFromTop(8);
    mixSlider.setBounds(out_area.removeFromTop(out_area.getWidth()*1.4f));
    outSlider.setBounds(out_area.removeFromBottom(out_area.getWidth()*1.4f));
    
    
    //draw spectrum area
    auto responseArea = bounds;
    g.setColour (juce::Colours::black);
    g.fillRect (responseArea);
    
    /*g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));*/
    audioProcessor.dynEQ.drawFrame(g, responseArea, in_area, out_area);
    
}

void TheMaskerAudioProcessorEditor::buttonClicked (Button*button)// [2]
{
    if (button->getButtonText() == undoButton.getButtonText()) {
        audioProcessor.parameters.undoManager->undo();
    }
    else if (button->getButtonText() == redoButton.getButtonText()) {
        audioProcessor.parameters.undoManager->redo();
    }
    else if (button->getButtonText() == loadButton.getButtonText())
    {
        auto defaultPresetLocation = File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory);
        juce::FileChooser chooser("Select preset to load...", defaultPresetLocation, "*.xml");
        if (chooser.browseForFileToOpen()) {
            auto fileToLoad = chooser.getResult();
            MemoryBlock sourceData;
            fileToLoad.loadFileAsData(sourceData); processor.setStateInformation(sourceData.getData(), sourceData.getSize());
        }
    }
    else if (button->getButtonText() == saveButton.getButtonText()) {
        auto defaultPresetLocation = File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory);
        juce::FileChooser chooser("Select save position...", defaultPresetLocation, "*.xml");
        if (chooser.browseForFileToSave(true)) {
            auto file = chooser.getResult();
            if (file.exists())
                file.deleteFile();
            
            juce::FileOutputStream outputStream(file);
            if (outputStream.openedOk())
            {
                MemoryBlock destData; processor.getStateInformation(destData); outputStream.write(destData.getData(), destData.getSize());
            }
        }
    }


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
        & outSlider,& stereoLinkedSlider,
        & undoButton, & redoButton, & loadButton, & saveButton
    };
}
