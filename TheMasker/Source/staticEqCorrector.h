#pragma once
#include <JuceHeader.h>

#define MAX_CH 2
#define CORRECTION_FILTERS 7

class MultiChFilter
{
public:
    MultiChFilter() 
    {
        for (int f = 0; f < MAX_CH; ++f)
            filters.add(new dsp::IIR::Filter<float>);
    }

    ~MultiChFilter() {}

    void prepareToPlay(double sampleRate, double freq, float q, float gain, uint32 numSamples, uint32 outCh)
    {
        // Prepara le specifiche di elaborazione audio
        dsp::ProcessSpec spec{};
        spec.sampleRate = sampleRate;
        spec.numChannels = outCh;
        spec.maximumBlockSize = numSamples;

        // Prepara i filtri sinistro e destro
        for (int f = 0; f < MAX_CH; ++f)
        {
            filters.getUnchecked(f)->prepare(spec);
            filters.getUnchecked(f)->coefficients = dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, q, Decibels::decibelsToGain(gain));
            filters.getUnchecked(f)->reset();
        }
    }

    void processBlock(dsp::AudioBlock<float>& block, int outCh)
    {
        for (int f = 0; f < outCh; ++f)
        {
            auto bl = block.getSingleChannelBlock(f);
            dsp::ProcessContextReplacing<float> ctx(bl);
            filters.getUnchecked(f)->process(ctx);
        }
    }
    
    OwnedArray<dsp::IIR::Filter<float>> filters;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiChFilter)
};

class StaticEqCorrector
{
public:

    StaticEqCorrector()
    {
        for (int f = 0; f < CORRECTION_FILTERS; ++f)
            filters.add(new MultiChFilter);
    }
    
    ~StaticEqCorrector() {}

    void prepareToPlay(double sampleRate, int numSamples, int outCh)
    {
        for (int f = 0; f < CORRECTION_FILTERS; ++f)
            filters.getUnchecked(f)->prepareToPlay(sampleRate, bande[f].frequency, bande[f].quality, bande[f].gain, numSamples, outCh);
    }

    void processBlock(juce::AudioBuffer<float>& buffer, const int numSamples, int outCh)
    {
        dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), outCh, 0, numSamples);

        for (int f = 0; f < CORRECTION_FILTERS; ++f)
            filters.getUnchecked(f)->processBlock(block, outCh);
    }

private:
    // Struttura per definire i parametri di equalizzazione
    struct banda {
        float frequency;
        float quality;
        float gain;
    };

    // Vettore di bande di equalizzazione
    std::vector<banda> bande{
        {71.0, 1.40, -0.670},
        {123.7, 2.45, +4.070},
        {189.8, 1.80, -3.930},
        {274.4, 1.45, +0.850},

        {15460, 5.00, +1.022},
        {15600, 1.00, +3.450},
        {17520, 0.99, -8.050}
    };

    OwnedArray<MultiChFilter> filters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticEqCorrector)
};