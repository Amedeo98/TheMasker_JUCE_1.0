/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DynamicEQ.h"

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

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

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
    
    double sampleRate = 0;
    int samplesPerBlock = 0;
    int inCh = 2;
    int scCh = 2;
    Converter conv;

    //float frequencies[npoints];
    array<float,npoints> frequencies;
    AudioBuffer<float> auxBuffer;
    
    UndoManager undoManager;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMaskerAudioProcessor)
};




