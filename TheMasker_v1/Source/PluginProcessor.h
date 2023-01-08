/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DynamicEQ.h"

#define PLUGIN_V 1


#define NAME_COMP "compAmt"
#define NAME_EXP  "expAmt"
#define NAME_ATQ "atqWeight"
#define NAME_SL "stereoLinked"
#define NAME_MIX "mix"
#define NAME_IN "inGain"
#define NAME_OUT "outGain"
#define NAME_SC "scGain"

//==============================================================================
/**
*/
class TheMasker_v1AudioProcessor  : public juce::AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    TheMasker_v1AudioProcessor();
    ~TheMasker_v1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void parameterChanged(const String& paramID, float newValue) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; };

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; };


    bool acceptsMidi() const override { return false; };
    bool producesMidi() const override { return false; };
    bool isMidiEffect() const override { return false; };
    double getTailLengthSeconds() const override { return 0.0; };

    //==============================================================================
    int getNumPrograms() override { return 1; };
    int getCurrentProgram() override { return 0; };
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName(int index) override { return {}; };
    void changeProgramName (int index, const juce::String& newName) override {};

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    std::vector<float> getFrequencies();
    

    
private:
    //==============================================================================
    AudioProcessorValueTreeState parameters;
    DynamicEQ dynEQ;
    
    int sampleRate = 0;

    Converter conv;

    std::vector<float> frequencies;
    AudioBuffer<float> auxBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMasker_v1AudioProcessor)
};
