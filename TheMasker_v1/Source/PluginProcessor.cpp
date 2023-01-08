/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheMasker_v1AudioProcessor::TheMasker_v1AudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       //.withInput("SideChain", juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
     ),
    parameters(*this, nullptr, "TheMaskerCompressor", {
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_COMP, PLUGIN_V}, "Comp", -1.0f, 1.0f, DEFAULT_COMP),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_EXP, PLUGIN_V}, "Exp", -1.0f, 1.0f, DEFAULT_EXP),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_ATQ, PLUGIN_V}, "CleanUp", 0.0f, 1.0f, DEFAULT_ATQ),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_SL, PLUGIN_V}, "StereoLinked", 0.0f, 1.0f, DEFAULT_SL),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_MIX, PLUGIN_V}, "Mix", 0.0f, 1.0f, DEFAULT_MIX),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_IN, PLUGIN_V}, "Input (dB)", -40.0f, 20.0f, DEFAULT_IN),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_OUT, PLUGIN_V}, "Output (dB)", -40.0f, 20.0f, DEFAULT_OUT),
        std::make_unique<AudioParameterFloat>(ParameterID {NAME_SC, PLUGIN_V},"Sidechain (dB)", -40.0f, 20.0f, DEFAULT_SC)
        })

{
    parameters.addParameterListener(NAME_COMP, this);
    parameters.addParameterListener(NAME_EXP, this);
    parameters.addParameterListener(NAME_ATQ, this);
    parameters.addParameterListener(NAME_SL, this);
    parameters.addParameterListener(NAME_MIX, this);
    parameters.addParameterListener(NAME_IN, this);
    parameters.addParameterListener(NAME_OUT, this);
    parameters.addParameterListener(NAME_SC, this);


    //state.state = juce::ValueTree(JucePlugin_Name);

}

TheMasker_v1AudioProcessor::~TheMasker_v1AudioProcessor()
{
}


//==============================================================================
void TheMasker_v1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void TheMasker_v1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TheMasker_v1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{

    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    /*
     if (layouts.inputBuses[1] != juce::AudioChannelSet::mono()
     && layouts.inputBuses[1] != juce::AudioChannelSet::stereo()
     && layouts.inputBuses[1] != juce::AudioChannelSet::disabled())
        return false;
    */

    
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layoutZ
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    
    
    return true;
}
#endif

void TheMasker_v1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
juce::AudioProcessorEditor* TheMasker_v1AudioProcessor::createEditor()
{
    return new TheMasker_v1AudioProcessorEditor (*this);
}


void TheMasker_v1AudioProcessor::parameterChanged(const String& paramID, float newValue)
{
}

//==============================================================================
void TheMasker_v1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TheMasker_v1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheMasker_v1AudioProcessor();
}
