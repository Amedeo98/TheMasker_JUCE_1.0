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
                temp = temp > 0.0f ? (temp - avg) * UIclearFreqs : (avg - temp) * UImaskedFreqs;
                temp = temp * UImix;
                temp = tanh(temp / maxGain) * maxGain;
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

            std::transform(curves[ch].delta.begin(), curves[ch].delta.end(),
                THclip.begin(), newValues[ch].data(),
                std::multiplies<float>());

            std::transform(newValues[ch].begin(), newValues[ch].end(),
                SCclip.begin(), newValues[ch].data(),
                std::multiplies<float>());

            FloatVectorOperations::copy(curves[ch].delta.data(), newValues[ch].data(), nfilts);
        }
    }

private:
    float maxGain = _maxGain;
    int gateThresh = _gateThresh;
    int gateKnee_inv = pow(_gateKnee,-1);
    float nfilts_inv = pow(nfilts, -1);
    int nCh = 0;
    array<float, nfilts> THclip;
    array<float, nfilts> SCclip;
    array<array<float, nfilts>, 2> newValues;
};


