/*
  ==============================================================================

    MultiBandMod.h
    Created: 20 Jan 2023 6:18:30pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Converters.h"
#include "LinkwitzRileyFilters.h"
#include "DynamicEQ.h"

#define MIDDLEBAND_OFF -0.32f
#define LPHP_OFF -17.526f
//#define BPEXT_OFF -11.52f

#define CORR_K_LEN 11

class MultiBandMod {
public:
    MultiBandMod() {}
    ~MultiBandMod() {}

    void prepareToPlay(double sampleRate, int newSamplesPerBlock, float* fCenters) {
        fs = sampleRate;
        samplesPerBlock = newSamplesPerBlock;
       
        for (int ch = 0; ch < 2; ++ch) {
            medGain[ch].reset(fs, _atkSmoothingSeconds, _relSmoothingSeconds);
        }

        for (int i = 0; i < nfilts; i++) {
            freqs[i].fCenter = fCenters[i];
        }

        getBandFreqs();

        populateCorrKernel();
        
        for (int i = 0; i < nfilts; i++) {
            filters[i].prepareToPlay(sampleRate, samplesPerBlock, freqs[i].f_lc, freqs[i].f_hc);
            staticGainAdjustments[i] = Decibels::decibelsToGain(MIDDLEBAND_OFF); //0.0; // (i % 2) ? -1.0f : 1.0f;
            adjustedGain[0][i] = 0;
            adjustedGain[1][i] = 0;
        }

        ///////////////// BOTTOM END
        setCorrectionGain(0, LPHP_OFF);
        setCorrectionGain(1, -15.276f);
        setCorrectionGain(2, -3.418f);
        setCorrectionGain(3, -1.460f);
        setCorrectionGain(4, -0.643f);
        setCorrectionGain(5, -0.441f);
        setCorrectionGain(6, -0.316f);
        setCorrectionGain(7, -0.341f);
        setCorrectionGain(8, -0.297f);
        setCorrectionGain(9, -0.340f);
        setCorrectionGain(10, -0.328f);

        ///////////////// TOP END
        setCorrectionGain(-11, -0.377f);
        setCorrectionGain(-10, -0.338f);
        setCorrectionGain(-9, -0.232f);
        setCorrectionGain(-8, -0.472f);
        setCorrectionGain(-7, -0.176f);
        setCorrectionGain(-6, -0.291f);
        setCorrectionGain(-5, -0.341f);
        setCorrectionGain(-4, -0.396f);
        setCorrectionGain(-3, -1.177f);
        setCorrectionGain(-2, -3.307f);
        setCorrectionGain(-1, LPHP_OFF);
    }

    void setCorrectionGain(int fIndex, float gainDb, bool invert = false)
    {
        if (fIndex < 0)
            fIndex += nfilts;

        jassert(fIndex < nfilts);
        jassert(fIndex >= 0);

        staticGainAdjustments[fIndex] = invert ? -Decibels::decibelsToGain(gainDb) : Decibels::decibelsToGain(gainDb);
    }

    float getCorrectionGain(int fIndex)
    {
        if (fIndex < 0)
            fIndex += nfilts;

        jassert(fIndex < nfilts);
        jassert(fIndex >= 0);

        return Decibels::gainToDecibels(staticGainAdjustments[fIndex]);
    }

    void setNumChannels(int nCh) {
        numCh = nCh;
        inputBuffer_copy.setSize(numCh, samplesPerBlock);
        tempOutput.setSize(numCh, samplesPerBlock);
        for (int i = 0; i < nfilts; i++) {
            filters[i].setNumChannels(numCh);
        }

    }
    
    void filterBlock(AudioBuffer<float>& buffer, auto& curves, auto& gains_sm, auto& gains_vs, bool& processFFTresult) {
        int numSamples = buffer.getNumSamples();
        //inputBuffer_copy.clear();
        inputBuffer_copy.setSize(numCh, numSamples, true, false, true); // This is soooo bad

        for (int ch = 0; ch < numCh; ch++) 
        {
            inputBuffer_copy.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }

        buffer.clear();

        if (processFFTresult)
        {
            for (int ch = 0; ch < numCh; ++ch)
                dynamicGainCorrection(ch, curves[ch].delta);
        }

        for (int f = 0; f < nfilts; f++) 
        {
            tempOutput.setSize(numCh, numSamples, true, false, true); // This is soooo bad
            tempOutput.clear();
            filters[f].process(inputBuffer_copy, tempOutput);

            for (int ch = 0; ch < numCh; ++ch) {

                if (processFFTresult)
                {
#ifdef fineTuneCoeff
                    if (fineTuneCoeff)
                        gains_sm[ch][f].setTargetValue(Decibels::decibelsToGain(adjustedGain[ch][f]));
                    else
                        gains_sm[ch][f].setTargetValue(Decibels::decibelsToGain(curves[ch].delta[f]));

                    gains_vs[ch][f].setTargetValue(gains_sm[ch][f].getTargetValue());
#else
                    gains_sm[ch][f].setTargetValue(Decibels::decibelsToGain(adjustedGain[ch][f]));
                    gains_vs[ch][f].setTargetValue(Decibels::decibelsToGain(curves[ch].delta[f]));
#endif // fineTuneCoeff

                }

                for (int sample = 0; sample < numSamples; ++sample) {
                    tempOutput.setSample(ch, sample, tempOutput.getSample(ch, sample) * gains_sm[ch][f].getNextValue());
                }

                gains_vs[ch][f].skip(numSamples);

                buffer.addFrom(ch, 0, tempOutput, ch, 0, numSamples, staticGainAdjustments[f]);

            }
            
        }

        if (OFFSET_CORRECTION)
            for (int ch = 0; ch < numCh; ++ch)
                medGain[ch].applyGain(buffer.getWritePointer(ch), numSamples);

    }
    

    void getBandFreqs() {
        freqs[0].f_lc = minFreq;
        freqs[0].f_hc = (freqs[0].fCenter + freqs[1].fCenter) * 0.5f;
        for (int i = 1; i < nfilts-1; i++) {
            freqs[i].f_lc = freqs[i - 1].f_hc;
            freqs[i].f_hc = (freqs[i].fCenter + freqs[i + 1].fCenter) *0.5f ;
        }
        freqs[nfilts-1].f_lc = freqs[nfilts - 2].f_hc;
        freqs[nfilts-1].f_hc = maxFreq;
    }


private:

    void populateCorrKernel()
    {
        if (ENABLE_CORRECTION)
        {
            // +5.0
            //dynamicAdjustmentKernel[1][0] = dynamicAdjustmentKernel[1][10] = 0.0;
            //dynamicAdjustmentKernel[1][1] = dynamicAdjustmentKernel[1][9] = 0.07500;
            //dynamicAdjustmentKernel[1][2] = dynamicAdjustmentKernel[1][8] = 0.16340;
            //dynamicAdjustmentKernel[1][3] = dynamicAdjustmentKernel[1][7] = -0.11260;
            //dynamicAdjustmentKernel[1][4] = dynamicAdjustmentKernel[1][6] = -0.21200;
            ///*                        */    dynamicAdjustmentKernel[1][5] = 0.79280;

            // +10
            dynamicAdjustmentKernel[1][0] = dynamicAdjustmentKernel[1][10] =  0.00770;
            dynamicAdjustmentKernel[1][1] = dynamicAdjustmentKernel[1][9] =   0.12950;
            dynamicAdjustmentKernel[1][2] = dynamicAdjustmentKernel[1][8] =  -0.02030; 
            dynamicAdjustmentKernel[1][3] = dynamicAdjustmentKernel[1][7] =   0.09970;
            dynamicAdjustmentKernel[1][4] = dynamicAdjustmentKernel[1][6] =  -0.12750;
            /*                           */ dynamicAdjustmentKernel[1][5] =   0.67900;

            // -10
            //dynamicAdjustmentKernel[0][0] = dynamicAdjustmentKernel[0][10] = 0.04060;
            //dynamicAdjustmentKernel[0][1] = dynamicAdjustmentKernel[0][9]  = 0.07280;
            //dynamicAdjustmentKernel[0][2] = dynamicAdjustmentKernel[0][8]  = -0.00610;
            //dynamicAdjustmentKernel[0][3] = dynamicAdjustmentKernel[0][7]  = 0.00930;
            //dynamicAdjustmentKernel[0][4] = dynamicAdjustmentKernel[0][6]  = 0.08340;
            ///*                        */    dynamicAdjustmentKernel[0][5]  = 0.23290;

            dynamicAdjustmentKernel[0][0] = dynamicAdjustmentKernel[0][10] =  0.02180;
            dynamicAdjustmentKernel[0][1] = dynamicAdjustmentKernel[0][9] =   0.01490;
            dynamicAdjustmentKernel[0][2] = dynamicAdjustmentKernel[0][8] =   0.05240; 
            dynamicAdjustmentKernel[0][3] = dynamicAdjustmentKernel[0][7] =   0.03850;
            dynamicAdjustmentKernel[0][4] = dynamicAdjustmentKernel[0][6] =  -0.07520;
            /*                        */    dynamicAdjustmentKernel[0][5] =   0.54150;
        }
        else
        {
            dynamicAdjustmentKernel[0][0] = dynamicAdjustmentKernel[0][10] = 0.0;
            dynamicAdjustmentKernel[0][1] = dynamicAdjustmentKernel[0][9] = 0.0;
            dynamicAdjustmentKernel[0][2] = dynamicAdjustmentKernel[0][8] = 0.0;
            dynamicAdjustmentKernel[0][3] = dynamicAdjustmentKernel[0][7] = 0.0;
            dynamicAdjustmentKernel[0][4] = dynamicAdjustmentKernel[0][6] = 0.0;
            dynamicAdjustmentKernel[0][5] = 1.0;

            dynamicAdjustmentKernel[1][0] = dynamicAdjustmentKernel[1][10] = 0.0;
            dynamicAdjustmentKernel[1][1] = dynamicAdjustmentKernel[1][9] = 0.0;
            dynamicAdjustmentKernel[1][2] = dynamicAdjustmentKernel[1][8] = 0.0;
            dynamicAdjustmentKernel[1][3] = dynamicAdjustmentKernel[1][7] = 0.0;
            dynamicAdjustmentKernel[1][4] = dynamicAdjustmentKernel[1][6] = 0.0;
            dynamicAdjustmentKernel[1][5] = 1.0;
        }

    }

    void dynamicGainCorrection(int ch, const std::array<float,nfilts>& target)
    {
        auto med = 0.0f;

        if (OFFSET_CORRECTION)
        {
            for (int t = 0; t < nfilts; ++t)
                med += target[t] / nfilts;

            medGain[ch].setTargetValue(Decibels::decibelsToGain(med));
        }

        for (int t = 0; t < nfilts; ++t)
            adjustedGain[ch][t] = 0.0f;

        const auto kh = (CORR_K_LEN - 1) / 2;

        for (int t = 0; t < nfilts; ++t)
        {
            for (int k = -kh; k <= kh; ++k)
            {
                const auto tk = t + k;
                const auto boundary = (tk < 0 || tk >= nfilts);
                const auto tmp = boundary ? 0 : (target[tk] - med);
                const auto tBoost = boundary ? 0 : (target[tk] >= 0);
                adjustedGain[ch][t] += tmp * dynamicAdjustmentKernel[tBoost][k + kh];
            }
        }
    }

    int numCh = 0;
    double fs;
    int samplesPerBlock;

    array<LinkwitzRileyFilters, nfilts> filters;
    AudioBuffer<float> inputBuffer_copy;
    AudioBuffer<float> tempOutput;

    float staticGainAdjustments[nfilts];
    float adjustedGain[2][nfilts];
    float dynamicAdjustmentKernel[2][CORR_K_LEN];

    array<CustomSmoothedValue<float, ValueSmoothingTypes::Linear>, 2> medGain;

    struct freq
    {
        float f_lc;
        float fCenter;
        float f_hc;
    };

    array<freq, nfilts> freqs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiBandMod)

};
