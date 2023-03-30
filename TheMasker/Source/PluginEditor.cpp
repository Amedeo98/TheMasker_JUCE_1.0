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

    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
     
    if(auto* s = dynamic_cast<CustomRotarySlider*>(&slider)) {
        if(s->sliderName == NAME_CLEARF || s->sliderName == NAME_MASKEDF)
        {
            auto avg = (rotaryEndAngle+rotaryStartAngle)*0.5f;
            if(sliderAngRad<avg)
            {
                g.setColour(Colour(200u, 64u, 164u));
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
    stereoLinkedSliderAttachment(audioProcessor.parameters, NAME_SL, stereoLinkedSlider),
    cleanUpSliderAttachment(audioProcessor.parameters, NAME_ATQ, cleanUpSlider)

{
    
    startTimerHz(25);
    
    undoButton.setButtonText("undo");
    redoButton.setButtonText("redo");
    loadButton.setButtonText("load");
    saveButton.setButtonText("save");
    toggleIn.setButtonText("in");
    toggleSc.setButtonText("sc");
    toggleD.setButtonText("delta");
    toggleOut.setButtonText("out");
    
    
    for(auto* comp : getComponents())
    {
        addAndMakeVisible(comp);
    }
    
    for(auto* btn: getButtons())
    {
        addAndMakeVisible(btn);
        btn->addListener(this);
    }

    
    // Carica il file SVG dal disco
    svgDrawable = Drawable::createFromImageData (BinaryData::TheMasker_bg_svg, BinaryData::TheMasker_bg_svgSize);
    setSize (824, 476);
    
}


TheMaskerAudioProcessorEditor::~TheMaskerAudioProcessorEditor()
{
    for(auto* btn: getButtons())
    {
        btn->removeListener(this);
    }
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

    undoButton.setBounds(settings_area.getX(), settings_area.getY(), 28, 24);
    redoButton.setBounds(settings_area.getX(), settings_area.getY() + 26, 28, 24);
    saveButton.setBounds(settings_area.getX() + 30, settings_area.getY(), 28, 24);
    loadButton.setBounds(settings_area.getX() + 30, settings_area.getY() + 26, 28, 24);
    
    auto in_slider_area = in_area.removeFromBottom(100);
    in_slider_area.removeFromTop(20);
    inSlider.setBounds(in_slider_area);
    
    //draw controls area
    auto controls_area = bounds.removeFromLeft(174);
    controls_area.removeFromTop(100);
    auto sc_area = controls_area.removeFromTop(110);
    scSlider.setBounds(sc_area);
    
    controls_area.removeFromTop(12);
    auto comp_area = controls_area.removeFromTop(66);
    compSlider.setBounds(comp_area);
    
    controls_area.removeFromTop(14);
    auto exp_area = controls_area.removeFromTop(66);
    expSlider.setBounds(exp_area);
    
    controls_area.removeFromBottom(16);
    cleanUpSlider.setBounds(controls_area.removeFromBottom(44));
    stereoLinkedSlider.setBounds(controls_area.removeFromBottom(44));
    
    //draw output area
    auto out_area = bounds.removeFromRight(74);
    out_area.removeFromTop(24);
    mixSlider.setBounds(out_area.removeFromTop(62));
    
    auto out_slider_area = out_area.removeFromBottom(100);
    out_slider_area.removeFromTop(20);
    outSlider.setBounds(out_slider_area);
    
    //draw spectrum area
    bounds.removeFromBottom(18);
    auto legend_area = bounds.removeFromBottom(18);
    toggleIn.setBounds(legend_area.getX()+24, legend_area.getY(), 48, 24);
    toggleSc.setBounds(legend_area.getX()+76, legend_area.getY(), 48, 24);
    toggleD.setBounds(legend_area.getX()+legend_area.getWidth()-128, legend_area.getY(), 58, 24);
    toggleOut.setBounds(legend_area.getX()+legend_area.getWidth()-72, legend_area.getY(), 58, 24);
    
    bounds.removeFromTop(28);
    bounds.removeFromLeft(16);
    bounds.removeFromRight(16);
    auto responseArea = bounds;
    
    //grid
    Array<float> freqs
    {
        20, /*30, 40,*/  50, 100,
        200, /*300, 400,*/  500, 1000,
        2000, /*3000, 4000, */ 5000, 10000,
        20000
    };
    
    Array<float> xs;
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(responseArea.getX() + responseArea.getWidth() * normX);
    }

    g.setColour(Colours::darkgrey.withAlpha(0.5f));
    for (auto x : xs)
    {
        g.drawVerticalLine(x, responseArea.getY(), responseArea.getHeight());
    }
    
    Array<float> gain
    {
        -24, -18, -12, -6, 0, 6, 12, 18, 24
    };

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(responseArea.getHeight()), float(responseArea.getY()));
        g.setColour(gDb == 0.f ? Colours::grey : Colours::darkgrey.withAlpha(0.3f));
        g.drawHorizontalLine(y, responseArea.getX(), responseArea.getX() + responseArea.getWidth());
    }

    g.setColour(Colour(255u, 200u, 100u));
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;

        if (f>999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(4);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(responseArea.getHeight()), float(responseArea.getY()));
        
        String str;
        if (gDb > 0)
            str << "+";
        
        str << gDb;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(responseArea.getX());
        r.setCentre(r.getCentreX(), y);
        
        g.setColour(gDb == 0.f ? Colours::grey : Colours::lightgrey);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
        
        str.clear();
        str << (gDb - 24.f);
    
    }
    

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
    else {
        audioProcessor.dynEQ.toggleSpectrumView(button->getButtonText());
        if(button->getButtonText() == toggleIn.getButtonText())
            toggleIn.toggle();
        if(button->getButtonText() == toggleSc.getButtonText())
            toggleSc.toggle();
        if(button->getButtonText() == toggleD.getButtonText())
            toggleD.toggle();
        if(button->getButtonText() == toggleOut.getButtonText())
            toggleOut.toggle();
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
        & outSlider,& stereoLinkedSlider
    };
}


std::vector<CustomButton*> TheMaskerAudioProcessorEditor::getButtons()
{
    return
    {
        & undoButton, &redoButton, & loadButton, & saveButton,
        &toggleIn, &toggleSc, &toggleD, &toggleOut
    };
}

