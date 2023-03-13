/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DynamicEQ.h"
#include "Converters.h"

#define PLUGIN_V 1

#define NAME_COMP "compAmt"
#define NAME_EXP  "expAmt"
#define NAME_ATQ "atqWeight"
#define NAME_SL "stereoLinked"
#define NAME_MIX "mix"
#define NAME_IN "inGain"
#define NAME_OUT "outGain"
#define NAME_SC "scGain"

/*
 Maschera > IN -> IN Masked        ->  MASKED PARTS: -1 = reveal … +1 = bury
 Maschera < IN -> IN Intelligibile ->  CLEAR PARTS:  -1 = bury  ... +1 = emphasise
 */

class TheMaskerAudioProcessor : public juce::AudioProcessor, public AudioProcessorValueTreeState::Listener

{
public:
    //==============================================================================
    TheMaskerAudioProcessor();
    ~TheMaskerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; };
    void changeProgramName(int index, const juce::String& newName) override {};

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    AudioProcessorValueTreeState parameters;
    void getFrequencies();
    
    DynamicEQ dynEQ;

private:
    
    int sampleRate = 0;
    int samplesPerBlock = 0;
    int numScChannels;
    int numInChannels;
    Converter conv;

    //float frequencies[npoints];
    array<float,npoints> frequencies;
    AudioBuffer<float> auxBuffer;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerAudioProcessor)
};




