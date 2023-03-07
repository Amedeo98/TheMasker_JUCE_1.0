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

    void prepareToPlay(int numChannels) {
        //newValues.resize(nfilts);
        //THclip.resize(nfilts);
        setNumChannels(numChannels);
    }

    void setNumChannels(int numChannels) {
        nCh = numChannels;
    }

    void scale(auto& curves, float UIcomp, float UIexp, float UImix) {
        for (int ch = 0; ch < nCh; ch++) {
            for (int i = 0; i < curves[ch].delta.size(); i++) {
                float temp = curves[ch].delta[i];
                temp = temp > 0.0f ? temp * UIcomp : temp * UIexp;
                temp = temp * UImix;
                temp = tanh(temp / maxGain) * maxGain;
                curves[ch].delta[i] = temp;
            }
        }
    }

    void clip(auto& curves) {
        for (int ch = 0; ch < nCh; ch++) {
            int size = curves[ch].threshold.size();
            for (int i = 0; i < size; i++) {
                THclip[i] = (1.0f + tanh((curves[ch].threshold[i] - gateThresh) * gateKnee_inv)) * 0.5f;
            }

            std::transform(curves[ch].delta.begin(), curves[ch].delta.end(),
                THclip.begin(), newValues.data(),
                std::multiplies<float>());
            FloatVectorOperations::copy(curves[ch].delta.data(), newValues.data(), nfilts);
        }
    }

private:
    float maxGain = _maxGain;
    int gateThresh = _gateThresh;
    int gateKnee_inv = pow(_gateKnee,-1);
    int nCh;
    array<float, nfilts> THclip;
    array<float, nfilts> newValues;
};


