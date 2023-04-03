/*
  ==============================================================================

    DeltaScaler.h
    Created: 20 Jan 2023 6:18:47pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

class DeltaScaler {
public:

    void prepareToPlay(float* fCenters) {
        getATQ(fCenters, atq);
        //setATQ(DEFAULT_ATQ);
    }

    void setNumChannels(int numChannels) {
        nCh = numChannels;
    }

    void scale(auto& curves, float UImaskedFreqs, float UIclearFreqs, float UImix) {
        for (int ch = 0; ch < nCh; ch++) {
            float avg = 0.0f;
            for (int i = 0; i < nfilts; i++) {
                avg += curves[ch].delta[i] * nfilts_inv;
            }
            for (int i = 0; i < nfilts; i++) {
                float temp = curves[ch].delta[i];
                //float cleaFValue = (temp - avg) * UIclearFreqs;
                //float maskedFValue = (avg - temp) * UImaskedFreqs;
                float clearFValue = UIclearFreqs > 0 ? jmax((temp - avg) * UIclearFreqs, 0.0f) * posDeltasAlpha : jmin((temp - avg) * UIclearFreqs, 0.0f);
                float maskedFValue = UImaskedFreqs > 0 ? jmax((avg - temp) * UImaskedFreqs, 0.0f) * posDeltasAlpha : jmin((avg - temp) * UImaskedFreqs, 0.0f);
                temp = temp > 0.0f ? clearFValue : maskedFValue;
                temp *= current_atq[i];
                temp = tanh(temp / (float) maxGain) * (float) maxGain;
                temp = temp * UImix * _mixMultiplier;
                curves[ch].delta[i] = temp;
            }
        }
    }

    void clip(auto& curves) {
        for (int ch = 0; ch < nCh; ch++) {
            for (int i = 0; i < nfilts; i++) {
                THclip[i] = (1.0f + tanh((curves[ch].inputDecimated[i] - gateThresh) * gateKnee_inv)) * 0.5f;
                SCclip[i] = (1.0f + tanh((curves[ch].scDecimated[i] - gateThresh) * gateKnee_inv)) * 0.5f;
            }

            FloatVectorOperations::multiply(curves[ch].delta.data(), THclip.data(), nfilts);
            FloatVectorOperations::multiply(curves[ch].delta.data(), SCclip.data(), nfilts);
        }
    }

    void setATQ(float UIatqWeight) {
        current_atq = atq;
        for (int i = 0; i < nfilts; i++) {
            //current_atq[i] = jmap(current_atq[i], 0.0f, max, 1.0f, bottomAtqMin + (1.0f - UIatqWeight) * bottomAtqRange);
            //current_atq[i] = mapFromLog10(current_atq[i], 1.0f, max);
            current_atq[i] = jmap(current_atq[i], atqBottom, atqTop, 1.0f, bottomAtqMin + (1.0f - UIatqWeight) * (bottomAtqMax - bottomAtqMin));
            current_atq[i] = jlimit(0.0f, 1.0f, current_atq[i]);
        }

        //juce::FloatVectorOperations::multiply(current_atq.data(), UIatqWeight, nfilts);
        //juce::FloatVectorOperations::multiply(current_atq.data(), atqLift, nfilts);
        //juce::FloatVectorOperations::add(current_atq.data(), minDBFS+5.0f, nfilts);
        //FloatVectorOperations::clip(current_atq.data(), current_atq.data(), minDBFS, 0.0f, nfilts);
    }

private:
    float maxGain = _maxGain;
    int gateThresh = _gateThresh;
    float gateKnee_inv = pow(_gateKnee,-1);
    float nfilts_inv = pow(nfilts, -1);
    float posDeltasAlpha = 0.5f;
    int nCh = 0;
    array<float, nfilts> THclip;
    array<float, nfilts> SCclip;

    array<float, nfilts> current_atq, atq;
    float atqBottom = 0.0f;
    float atqTop = 1.0f;
    float bottomAtqMin = -2.8f;
    float bottomAtqMax = 0.5f;

    void getATQ(float* f, array<float, nfilts>& dest)
    {
        for (int i = 0; i < nfilts; i++)
        {
            //   matlab function: absThresh=3.64*(f./1000).^-0.8-6.5*exp(-0.6*(f./1000-3.3).^2)+.00015*(f./1000).^4; % edited function (reduces the threshold in high freqs)
            dest[i] = 8.5f * pow((f[i] / 1000), -0.5f) - 6.5 * exp(-0.6f * pow(f[i] / 1000 - 3.3f, 2)) + 0.57f * pow(f[i] / 1000, 1.61f);
        }

        atqBottom = FloatVectorOperations::findMinimum(atq.data(), nfilts);
        atqTop = FloatVectorOperations::findMaximum(atq.data(), nfilts);


    }

    //void scaleWithAtq(array<float, nfilts>& rel_t, array<float, nfilts>& atq) {
    //    //FloatVectorOperations::max(rel_t.data(), rel_t.data(), atq.data(), nfilts);
    //    for (int i = 0; i < nfilts; i++) {
    //        rel_t[i] *= atq[i];
    //    }
    //}

};


