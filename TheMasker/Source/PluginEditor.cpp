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
    g.setColour(Colours::grey.withAlpha(0.8f));
    bkg.addCentredArc(center.getX(), center.getY(), radius, radius, 0, rotaryStartAngle, rotaryEndAngle, true);
    auto bkgStroke = PathStrokeType(4.0, juce::PathStrokeType::JointStyle::curved);
    bkgStroke.createStrokedPath(bkg, bkg);
    g.fillPath(bkg);
    
    //amount indicator
    Path amt;
    g.setColour(Colour(64u, 200u, 64u));
    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
    amt.addCentredArc(center.getX(), center.getY(), radius, radius, 0, rotaryStartAngle, sliderAngRad, true);
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
                           Slider &)
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


//==============================================================================
TheMaskerAudioProcessorEditor::TheMaskerAudioProcessorEditor (TheMaskerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    inSlider(*audioProcessor.parameters.getParameter(NAME_IN), NAME_IN, true),
    outSlider(*audioProcessor.parameters.getParameter(NAME_OUT), NAME_OUT, true),
    scSlider(*audioProcessor.parameters.getParameter(NAME_SC), NAME_SC, true),
    compSlider(*audioProcessor.parameters.getParameter(NAME_MASKEDF), NAME_MASKEDF, false),
    expSlider(*audioProcessor.parameters.getParameter(NAME_CLEARF), NAME_CLEARF, false),
    mixSlider(*audioProcessor.parameters.getParameter(NAME_MIX), NAME_MIX, false),
    cleanUpSlider(*audioProcessor.parameters.getParameter(NAME_ATQ), NAME_ATQ, false),
    stereoLinkedSlider(*audioProcessor.parameters.getParameter(NAME_SL), NAME_SL, false),


    inSliderAttachment(audioProcessor.parameters, NAME_IN, inSlider),
    outSliderAttachment(audioProcessor.parameters, NAME_OUT, outSlider),
    scSliderAttachment(audioProcessor.parameters, NAME_SC, scSlider),
    mixSliderAttachment(audioProcessor.parameters, NAME_MIX, mixSlider),

    compSliderAttachment(audioProcessor.parameters, NAME_MASKEDF, compSlider),
    expSliderAttachment(audioProcessor.parameters, NAME_CLEARF, expSlider),
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
    
    
    // Carica il file SVG dal disco
    svgDrawable = Drawable::createFromImageData (BinaryData::TheMasker_bg_svg, BinaryData::TheMasker_bg_svgSize);
    setSize (824, 476);
    
}


TheMaskerAudioProcessorEditor::~TheMaskerAudioProcessorEditor()
{
    redoButton.removeListener(this);
    undoButton.removeListener(this);
    saveButton.removeListener(this);
    loadButton.removeListener(this);
}


//==============================================================================
void TheMaskerAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Disegna il contenuto del file SVG
    if (svgDrawable != nullptr)
    {
        auto boundsFloat = getLocalBounds().toFloat();
        auto drawableBounds = svgDrawable->getDrawableBounds();
        auto scale = std::min(boundsFloat.getWidth() / drawableBounds.getWidth(),
                              boundsFloat.getHeight() / drawableBounds.getHeight());
        auto translation = boundsFloat.getCentre() - drawableBounds.getCentre() * scale;
        auto transform = juce::AffineTransform::scale(scale).translated(translation);
        svgDrawable->setTransform(transform);
        svgDrawable->draw(g, 1.0f);
    }
    
    //draw input area
    auto in_area = bounds.removeFromLeft(74);
    
    //load, save, ecc
    auto settings_area = in_area.removeFromTop(80);
    settings_area.removeFromTop(20);
    settings_area.removeFromBottom(12);
    settings_area.removeFromLeft(8);
    settings_area.removeFromRight(8);
    
    g.setColour(Colours::red.withAlpha(0.5f));
    undoButton.setBounds(settings_area.getX(), settings_area.getY(), 28, 24);
    redoButton.setBounds(settings_area.getX(), settings_area.getY() + 26, 28, 24);
    saveButton.setBounds(settings_area.getX() + 30, settings_area.getY(), 28, 24);
    loadButton.setBounds(settings_area.getX() + 30, settings_area.getY() + 26, 28, 24);
    
    auto in_slider_area = in_area.removeFromBottom(100);
    in_slider_area.removeFromTop(20);
    inSlider.setBounds(in_slider_area);
    
    
    //draw controls area
    auto controls_area = bounds.removeFromLeft(174);
    controls_area.removeFromTop(104);
    auto sc_area = controls_area.removeFromTop(110);
    scSlider.setBounds(sc_area);
    
    controls_area.removeFromTop(8);
    auto comp_area = controls_area.removeFromTop(66);
    compSlider.setBounds(comp_area);
    
    controls_area.removeFromTop(14);
    auto exp_area = controls_area.removeFromTop(66);
    expSlider.setBounds(exp_area);
    
    controls_area.removeFromBottom(16);
    stereoLinkedSlider.setBounds(controls_area.removeFromBottom(44));
    cleanUpSlider.setBounds(controls_area.removeFromBottom(44));
    
    //draw output area
    auto out_area = bounds.removeFromRight(74);
    out_area.removeFromTop(24);
    mixSlider.setBounds(out_area.removeFromTop(60));
    
    auto out_slider_area = out_area.removeFromBottom(100);
    out_slider_area.removeFromTop(20);
    outSlider.setBounds(out_slider_area);
    
    //draw spectrum area
    bounds.removeFromTop(24);
    bounds.removeFromBottom(24);
    auto responseArea = bounds;

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
