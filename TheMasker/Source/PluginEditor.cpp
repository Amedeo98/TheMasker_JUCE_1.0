/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DynamicEQ.h"
#include "CustomButton.h"
#include "Sliders.h"

//==============================================================================
TheMaskerComponent::TheMaskerComponent (TheMaskerAudioProcessor& p)
    : audioProcessor (p), 
    inSlider(*audioProcessor.parameters.getParameter(NAME_IN), NAME_IN, true, false),
    outSlider(*audioProcessor.parameters.getParameter(NAME_OUT), NAME_OUT, true, false),
    scSlider(*audioProcessor.parameters.getParameter(NAME_SC), NAME_SC, true, false),
    compSlider(*audioProcessor.parameters.getParameter(NAME_MASKEDF), NAME_MASKEDF, false, false),
    expSlider(*audioProcessor.parameters.getParameter(NAME_CLEARF), NAME_CLEARF, false, false),
    mixSlider(*audioProcessor.parameters.getParameter(NAME_MIX), NAME_MIX, false, true),
    cleanUpSlider(*audioProcessor.parameters.getParameter(NAME_ATQ), NAME_ATQ, false, false),
    stereoLinkedSlider(*audioProcessor.parameters.getParameter(NAME_SL), NAME_SL, false, false),


    inSliderAttachment(audioProcessor.parameters, NAME_IN, inSlider),
    outSliderAttachment(audioProcessor.parameters, NAME_OUT, outSlider),
    scSliderAttachment(audioProcessor.parameters, NAME_SC, scSlider),
    mixSliderAttachment(audioProcessor.parameters, NAME_MIX, mixSlider),

    compSliderAttachment(audioProcessor.parameters, NAME_MASKEDF, compSlider),
    expSliderAttachment(audioProcessor.parameters, NAME_CLEARF, expSlider),
    stereoLinkedSliderAttachment(audioProcessor.parameters, NAME_SL, stereoLinkedSlider),
    cleanUpSliderAttachment(audioProcessor.parameters, NAME_ATQ, cleanUpSlider)

{
    
    startTimerHz(_editorRefreshRate);
    
    undoButton.setButtonText("undo");
    redoButton.setButtonText("redo");
    loadButton.setButtonText("load");
    saveButton.setButtonText("save");
    toggleIn.setButtonText("in");
    toggleSc.setButtonText("sc");
    toggleD.setButtonText("delta");
    toggleOut.setButtonText("out");
    resetInButton.setButtonText("resetIn");
    resetOutButton.setButtonText("resetOut");
    
    
    for(auto* comp : getComponents())
    {
        addAndMakeVisible(comp);
    }
    
    for(auto* btn: getButtons())
    {
        addAndMakeVisible(btn);
        btn->addListener(this);
    }

#ifdef fineTuneCoeff
        {
            auto w = 60;
            auto h = 350;
            auto tbw = 60;
            auto wr = 1.1;

            auto r = 24;

            // Bottom end
            tsIndx[0] = 1;
            tsIndx[1] = 2;
            tsIndx[2] = 3;
            tsIndx[3] = 4;
            tsIndx[4] = 5;

            // Top end
            //tsIndx[0] = -5;
            //tsIndx[1] = -4;
            //tsIndx[2] = -3;
            //tsIndx[3] = -2;
            //tsIndx[4] = -1;

            corrInv[0] = false;
            corrInv[1] = false;
            corrInv[2] = false;
            corrInv[3] = false;
            corrInv[4] = false;

            auto i = 0;

            TS1.setBounds(300 + 0 * wr * w, 60, w, h);
            TS1.setRange(-r, r, 0.001);
            TS1.setValue(audioProcessor.dynEQ.getCorrectionGain(tsIndx[0]), dontSendNotification);
            TS1.setSliderStyle(juce::Slider::LinearVertical);
            TS1.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbw, 20);
            TS1.addListener(this);
            addAndMakeVisible(TS1);
            i++;
    
            TS2.setBounds(300 + i * wr * w, 60, w, h);
            TS2.setRange(-r, r, 0.001);
            TS2.setValue(audioProcessor.dynEQ.getCorrectionGain(tsIndx[i]), dontSendNotification);
            TS2.setSliderStyle(juce::Slider::LinearVertical);
            TS2.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbw, 20);
            TS2.addListener(this);
            addAndMakeVisible(TS2);
            i++;

            TS3.setBounds(300 + i * wr * w, 60, w, h);
            TS3.setRange(-r, r, 0.001);
            TS3.setValue(audioProcessor.dynEQ.getCorrectionGain(tsIndx[i]), dontSendNotification);
            TS3.setSliderStyle(juce::Slider::LinearVertical);
            TS3.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbw, 20);
            TS3.addListener(this);
            addAndMakeVisible(TS3);
            i++;

            TS4.setBounds(300 + i * wr * w, 60, w, h);
            TS4.setRange(-r, r, 0.001);
            TS4.setValue(audioProcessor.dynEQ.getCorrectionGain(tsIndx[i]), dontSendNotification);
            TS4.setSliderStyle(juce::Slider::LinearVertical);
            TS4.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbw, 20);
            TS4.addListener(this);
            addAndMakeVisible(TS4);
            i++;

            TS5.setBounds(300 + i * wr * w, 60, w, h);
            TS5.setRange(-r, r, 0.001);
            TS5.setValue(audioProcessor.dynEQ.getCorrectionGain(tsIndx[i]), dontSendNotification);
            TS5.setSliderStyle(juce::Slider::LinearVertical);
            TS5.setTextBoxStyle(juce::Slider::TextBoxBelow, false, tbw, 20);
            TS5.addListener(this);
            addAndMakeVisible(TS5);
            i++;
        }
#endif // fineTuneCoeff

    
        svgDrawable = Drawable::createFromImageData (BinaryData::TheMasker_bg_svg, BinaryData::TheMasker_bg_svgSize);
    }

#ifdef fineTuneCoeff
    void TheMaskerComponent::sliderValueChanged(Slider* slider)
    {
        audioProcessor.dynEQ.setCorrectionGain(tsIndx[0], TS1.getValue(), corrInv[0]);
        audioProcessor.dynEQ.setCorrectionGain(tsIndx[1], TS2.getValue(), corrInv[1]);
        audioProcessor.dynEQ.setCorrectionGain(tsIndx[2], TS3.getValue(), corrInv[2]);
        audioProcessor.dynEQ.setCorrectionGain(tsIndx[3], TS4.getValue(), corrInv[3]);
        audioProcessor.dynEQ.setCorrectionGain(tsIndx[4], TS5.getValue(), corrInv[4]);
    }
#endif // fineTuneCoeff

TheMaskerComponent::~TheMaskerComponent()
{
    for(auto* btn: getButtons())
    {
        btn->removeListener(this);
    }
}

//==============================================================================
void TheMaskerComponent::paint (juce::Graphics& g)
{
    // Disegna il contenuto del file SVG
    if (svgDrawable != nullptr)
    {
        svgDrawable->draw(g, 1.0f);
    }

    g.setFont(fontHeight);

    audioProcessor.dynEQ.drawFrame(g, responseArea, in_area, out_area);

    g.setColour(Colour(_grey).withAlpha(0.2f));
    for (auto x : xs)
    {
        g.drawVerticalLine(x, responseArea.getY(), responseArea.getHeight());
    }

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(responseArea.getHeight()), float(responseArea.getY()));
        g.setColour(gDb == 0.f ? Colour(_lightgrey).withAlpha(0.2f) : Colour(_grey).withAlpha(0.2f));
        g.drawHorizontalLine(y, responseArea.getX(), responseArea.getX() + responseArea.getWidth());
    }

    g.setColour(_yellow);

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
        auto y = jmap(gDb, float(-_maxDeltaSpectrum), float(_maxDeltaSpectrum), float(responseArea.getHeight()), float(responseArea.getY()));
        
        String str;
        if (gDb > 0)
            str << "+";
        
        str << gDb;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(responseArea.getX());
        r.setCentre(r.getCentreX(), y);
        
        g.setColour(gDb == 0.f ? _grey : _lightgrey);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
        
        str.clear();
        str << (gDb - 24.f);
    }
    

}

void TheMaskerComponent::buttonClicked (Button*button)// [2]
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
            fileToLoad.loadFileAsData(sourceData); audioProcessor.setStateInformation(sourceData.getData(), sourceData.getSize());
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
                MemoryBlock destData; audioProcessor.getStateInformation(destData); outputStream.write(destData.getData(), destData.getSize());
            }
        }
    }
    else if (button->getButtonText() == resetInButton.getButtonText())
    {
        audioProcessor.dynEQ.resetMaxVolume("in");
    }
    else if (button->getButtonText() == resetOutButton.getButtonText())
    {
        audioProcessor.dynEQ.resetMaxVolume("out");
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


void TheMaskerComponent::resized()
{
    Rectangle<int> bounds,
        settings_area,
        in_slider_area,
        controls_area,
        sc_area,
        comp_area,
        exp_area,
        out_slider_area,
        legend_area;

    bounds = getLocalBounds();

    if (svgDrawable != nullptr)
    {
        auto boundsFloat = bounds.toFloat();
        auto drawableBounds = svgDrawable->getDrawableBounds();
        auto scale = std::min(boundsFloat.getWidth() / drawableBounds.getWidth(),
            boundsFloat.getHeight() / drawableBounds.getHeight());
        auto translation = boundsFloat.getCentre() - drawableBounds.getCentre() * scale;
        auto transform = juce::AffineTransform::scale(scale).translated(translation);
        svgDrawable->setTransform(transform);
        //svgDrawable->draw(g, 1.0f);
    }

    //draw input area
    in_area = bounds.removeFromLeft(74);
    settings_area = in_area.removeFromTop(80);

    //load, save, ecc
    settings_area.removeFromTop(20);
    settings_area.removeFromBottom(12);
    settings_area.removeFromLeft(8);
    settings_area.removeFromRight(8);

    undoButton.setBounds(settings_area.getX(), settings_area.getY(), 28, 24);
    redoButton.setBounds(settings_area.getX(), settings_area.getY() + 26, 28, 24);
    saveButton.setBounds(settings_area.getX() + 30, settings_area.getY(), 28, 24);
    loadButton.setBounds(settings_area.getX() + 30, settings_area.getY() + 26, 28, 24);

    in_slider_area = in_area.removeFromBottom(100);
    //g.setFont(fontHeight);
    in_slider_area.removeFromTop(20);
    inSlider.setBounds(in_slider_area);

    //draw controls area
    controls_area = bounds.removeFromLeft(174);
    controls_area.removeFromTop(100);
    sc_area = controls_area.removeFromTop(110);
    scSlider.setBounds(sc_area);

    controls_area.removeFromTop(12);
    comp_area = controls_area.removeFromTop(66);
    compSlider.setBounds(comp_area);

    controls_area.removeFromTop(14);
    exp_area = controls_area.removeFromTop(66);
    expSlider.setBounds(exp_area);

    controls_area.removeFromBottom(16);
    cleanUpSlider.setBounds(controls_area.removeFromBottom(44));
    stereoLinkedSlider.setBounds(controls_area.removeFromBottom(44));

    //draw output area
    out_area = bounds.removeFromRight(74);
    out_area.removeFromTop(24);
    mixSlider.setBounds(out_area.removeFromTop(62));

    out_slider_area = out_area.removeFromBottom(100);
    out_slider_area.removeFromTop(20);
    outSlider.setBounds(out_slider_area);

    //draw spectrum area
    bounds.removeFromBottom(18);
    legend_area = bounds.removeFromBottom(18);
    toggleIn.setBounds(legend_area.getX() + 24, legend_area.getY(), 48, 24);
    toggleSc.setBounds(legend_area.getX() + 76, legend_area.getY(), 48, 24);
    toggleD.setBounds(legend_area.getX() + legend_area.getWidth() - 128, legend_area.getY(), 58, 24);
    toggleOut.setBounds(legend_area.getX() + legend_area.getWidth() - 72, legend_area.getY(), 58, 24);

    bounds.removeFromTop(28);
    bounds.removeFromLeft(16);
    bounds.removeFromRight(16);
    responseArea = bounds;

    resetInButton.setBounds(in_area);
    resetOutButton.setBounds(out_area);
    //audioProcessor.dynEQ.drawFrame(g, responseArea, in_area, out_area);

    //Array<float> xs;
    xs.clear();
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(responseArea.getX() + responseArea.getWidth() * normX);
    }
}


std::vector<juce::Component*> TheMaskerComponent::getComponents()
{
    return 
    {
       &inSlider,& outSlider,& mixSlider,
        & scSlider,& compSlider,& expSlider,& cleanUpSlider,
        & outSlider,& stereoLinkedSlider
    };
}


std::vector<CustomButton*> TheMaskerComponent::getButtons()
{
    return
    {
        & undoButton, &redoButton, & loadButton, & saveButton,
        &toggleIn, &toggleSc, &toggleD, &toggleOut, &resetInButton, &resetOutButton
    };
}



Wrapper::Wrapper(TheMaskerAudioProcessor& p) : AudioProcessorEditor(p), theMaskerComponent(p)
{
    addAndMakeVisible(theMaskerComponent);
    
    if(auto * constrainer = getConstrainer())
    {
        constrainer->setFixedAspectRatio(static_cast<double>(originalWidth)/static_cast<double>(originalHeight));
        constrainer->setSizeLimits(originalWidth/2, originalHeight/2,
                                   originalWidth * 1.5, originalHeight * 1.5);
    }
    
    setResizable(true, true);
    setSize(originalWidth, originalHeight);
}

void Wrapper::resized()
{
    const auto scaleFactor = static_cast<float>(getWidth()) / static_cast<float> (originalWidth);
    theMaskerComponent.setTransform(AffineTransform::scale(scaleFactor));
    theMaskerComponent.setBounds(0, 0, originalWidth, originalHeight);
}





