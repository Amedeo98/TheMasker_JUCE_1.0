#pragma once
#include <JuceHeader.h>

#define MAX_CH 2
#define CORRECTION_FILTERS 8

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
        std::vector<bandParameters> corrections = getCorrectionForSampleRate(sampleRate);

        for (int f = 0; f < CORRECTION_FILTERS; ++f)
            filters.getUnchecked(f)->prepareToPlay(sampleRate, corrections[f].frequency, corrections[f].quality, corrections[f].gain, numSamples, outCh);
    }

    void processBlock(juce::AudioBuffer<float>& buffer, const int numSamples, int outCh)
    {
        dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), outCh, 0, numSamples);

        for (int f = 0; f < filters.size(); ++f)
            filters.getUnchecked(f)->processBlock(block, outCh);
    }


private:
    // Struttura per definire i parametri di equalizzazione
    struct bandParameters {
        float frequency;
        float quality;
        float gain;
    };

    // Vettore di bande di equalizzazione
    std::vector<bandParameters> bande44100{
        {71.0, 1.40, -0.670},
        {123.7, 2.45, +4.070},
        {189.8, 1.80, -3.930},
        {274.4, 1.45, +0.850},
        {15460, 5.00, +1.022},
        {15600, 1.00, +3.450},
        {17520, 0.99, -8.050},
        {20000, 1.0, 0.0}
    };
    std::vector<bandParameters> bande48000{
        {71.0, 1.40, -0.67},
        {123.7, 2.45, 4.07},
        {189.8, 1.8, -3.93},
        {274.4, 1.45, 0.850},
        {7600.0, 0.7, 0.30},
        {17000.0, 1.2, 0.6 },
        {17705.0, 1.0, -7.4},
        {20000, 1.0, 0.0}
    };
    std::vector<bandParameters> bande96000{
        {71.0, 1.4, -0.67},
        {123.7, 2.41, 3.9},
        {189.8, 1.8, -3.93},
        {274.4, 1.45, 0.85},
        {6000, 0.92, 3.2 },
        {11500.0, 1.1, -1.6},
        {16940.0, 0.69, -11.3},
        {31740.0, 0.8, 1.6}
    };
    std::vector<bandParameters> bande192000{
        {71.0, 1.40, -0.67},
        {123.7, 2.45, 4.0},
        {189.8, 1.8, -3.93},
        {274.4, 1.45, 0.850},
        {5800, 0.80, 6.2},
        {16152.0, 0.50, -14.85},
        {29000.0, 1, 0.8},
        {41500.0, 0.66,3.6}
    };

    const std::vector<bandParameters>& getCorrectionForSampleRate(double sampleRate) const 
    {
        switch (roundToInt(sampleRate))
        {
        case 44100:
            return bande44100;
        case 48000:
            return bande48000;
        case 96000:
            return bande96000;
        case 192000:
            return bande192000;
        default:
            // Trova la frequenza più vicina
            double minDiff = std::abs(sampleRate - 44100);
            double closestFreq = 44100;
            if (std::abs(sampleRate - 48000) < minDiff) {
                minDiff = std::abs(sampleRate - 48000);
                closestFreq = 48000;
            }
            if (std::abs(sampleRate - 96000) < minDiff) {
                minDiff = std::abs(sampleRate - 96000);
                closestFreq = 96000;
            }
            if (std::abs(sampleRate - 192000) < minDiff) {
                closestFreq = 192000;
            }
            return getCorrectionForSampleRate(closestFreq);
        }
    }

    OwnedArray<MultiChFilter> filters;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticEqCorrector)
};