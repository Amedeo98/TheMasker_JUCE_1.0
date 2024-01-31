/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"




TheMaskerAudioProcessor::TheMaskerAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput   ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput  ("Output", juce::AudioChannelSet::stereo(), true)
                       .withInput("SideChain", juce::AudioChannelSet::stereo(), true)
     ),
parameters(*this, &undoManager, "TheMaskerCompressor", {
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_MASKEDF, PLUGIN_V}, "Masked Freqs", -1.0f, 1.0f, DEFAULT_MASKEDF),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_CLEARF, PLUGIN_V}, "Clear Freqs", -1.0f, 1.0f, DEFAULT_CLEARF),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_ATQ, PLUGIN_V}, "CleanUp", 0.0f, 1.0f, DEFAULT_ATQ),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_SL, PLUGIN_V}, "StereoLinked", 0.0f, 1.0f, DEFAULT_SL),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_MIX, PLUGIN_V}, "Mix", NormalisableRange<float>(0.0f, 2.0f, 0.001f),  DEFAULT_MIX),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_IN, PLUGIN_V}, "Input (dB)", -40.0f, 40.0f, DEFAULT_IN),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_OUT, PLUGIN_V}, "Output (dB)", -40.0f, 40.0f, DEFAULT_OUT),
    std::make_unique<AudioParameterFloat>(ParameterID {NAME_SC, PLUGIN_V},"Sidechain (dB)", -40.0f, 40.0f, DEFAULT_SC)
    })

    /*
    Maschera > IN -> IN Masked        ->  MASKED PARTS: -1 = reveal … +1 = bury
    Maschera < IN -> IN Intelligibile ->  CLEAR PARTS:  -1 = bury  ... +1 = emphasise
    */
{
    parameters.addParameterListener(NAME_MASKEDF, this);
    parameters.addParameterListener(NAME_CLEARF, this);
    parameters.addParameterListener(NAME_ATQ, this);
    parameters.addParameterListener(NAME_SL, this);
    parameters.addParameterListener(NAME_MIX, this);
    parameters.addParameterListener(NAME_IN, this);
    parameters.addParameterListener(NAME_OUT, this);
    parameters.addParameterListener(NAME_SC, this);

    //parameters.state = juce::ValueTree(JucePlugin_Name);

}

TheMaskerAudioProcessor::~TheMaskerAudioProcessor()
{
    //Analyser.stopThread
}


void TheMaskerAudioProcessor::prepareToPlay (double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;

    inCh = jmax(getMainBusNumInputChannels(), getMainBusNumOutputChannels());
    scCh = getChannelCountOfBus(true, 1);

    auxBuffer.setSize(2, samplesPerBlock);
    
    getFrequencies();
    dynEQ.prepareToPlay(frequencies, sampleRate, samplesPerBlock);
    dynEQ.numChannelsChanged(inCh, scCh);

    setLatencySamples(_fftSize);
}

void TheMaskerAudioProcessor::releaseResources()
{
    dynEQ.releaseResources();
    auxBuffer.setSize(0, 0);
}

bool TheMaskerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{

        if (layouts.inputBuses[1] != juce::AudioChannelSet::mono()
            && layouts.inputBuses[1] != juce::AudioChannelSet::stereo()
            && layouts.inputBuses[1] != juce::AudioChannelSet::disabled())
            return false;

        if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
            && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;

        //if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        // && layouts.inputBuses[1] == juce::AudioChannelSet::stereo())
        //    return false;

    return true;
}


void TheMaskerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    //auto totalNumInputChannels  = getTotalNumInputChannels();
    //auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto numSamples = buffer.getNumSamples();

    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {  
    //    buffer.clear(i, 0, buffer.getNumSamples());
    //}

    auto mainBuffer = getBusBuffer(buffer, true, 0);
    auto scBuffer = getBusBuffer(buffer, true, 1);
     
    const AudioBuffer<float>& scSource = scBuffer.getNumChannels() ? scBuffer : mainBuffer;

    int nScCh = scSource.getNumChannels();
    int nInCh = mainBuffer.getNumChannels();


    if (scCh != nScCh || inCh != nInCh) {
        scCh = nScCh;
        inCh = nInCh;
        dynEQ.numChannelsChanged(inCh, scCh);
    }


    auxBuffer.clear();

    // Fill auxBuffer with the sc data to be compared with the input (according to the num of ch of each buffer)
    for (int ch = 0; ch < jmax(inCh, scCh); ch++) {
        auxBuffer.addFrom(jmin(ch,inCh-1), 0, scSource, jmin(ch,scCh-1), 0, numSamples);
    }

    if (inCh == 1)
    {
        auxBuffer.copyFrom(1, 0, auxBuffer, 0, 0, numSamples);
        auxBuffer.applyGain(1.0f / sqrt(2.0f));
    }

    dynEQ.processBlock(mainBuffer, auxBuffer);

}



juce::AudioProcessorEditor* TheMaskerAudioProcessor::createEditor()
{
    return new Wrapper  (*this);
    //use this for simple sliders
    //return new juce::GenericAudioProcessorEditor(*this);
}



void TheMaskerAudioProcessor::parameterChanged(const String& paramID, float newValue)
{
    if (paramID == NAME_MASKEDF)
        dynEQ.setMaskedFreqs(newValue);

    if (paramID == NAME_CLEARF)
        dynEQ.setClearFreqs(newValue);

    if (paramID == NAME_ATQ)
        dynEQ.setAtq(newValue);

    if (paramID == NAME_SL)
        dynEQ.setStereoLinked(newValue);

    if (paramID == NAME_MIX)
        dynEQ.setMix(newValue);

    if (paramID == NAME_IN)
        dynEQ.setInGain(newValue);

    if (paramID == NAME_OUT)
        dynEQ.setOutGain(newValue);

    if (paramID == NAME_SC)
        dynEQ.setScGain(newValue);
}


//==============================================================================
void TheMaskerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    
}

void TheMaskerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
    
}

void TheMaskerAudioProcessor::getFrequencies() {

    float maxbark, minbark;
    conv.hz2bark(maxFreq, maxbark);
    conv.hz2bark(minFreq, minbark);
    float step_bark = (maxbark - minbark) / (npoints - 1);
    for (int i = 0; i < npoints; ++i){
        conv.bark2hz(minbark + step_bark * i, frequencies[i]);
    }
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheMaskerAudioProcessor();
}
