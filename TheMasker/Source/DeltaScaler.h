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

    void prepareToPlay() {
        newValues.resize(nfilts);
        THclip.resize(nfilts);
    }

    void scale(vector<float>& delta, float UIcomp, float UIexp, float UImix) {
        for (int i = 0; i < delta.size(); i++) {
            float temp = delta[i];
            temp = temp > 0.0f ? temp * UIcomp : temp * UIexp;
            temp = temp * UImix;
            temp = tanh(temp / maxGain) * maxGain;
            delta[i] = temp;
        }
    }

    void clip(vector<float>& delta, vector<float> threshold) {
        for (int i = 0; i < threshold.size(); i++) {
            THclip[i] = (1.0f + tanh((threshold[i] - gateThresh) / gateKnee)) / 2.0f;
        }
        //conv.mXv_mult(yValues, THclip);
        std::transform(delta.begin(), delta.end(),
            THclip.begin(), newValues.data(),
            std::multiplies<float>());
        delta = newValues;
    }

private:
    float maxGain = 20;
    int gateThresh = -40;
    int gateKnee = 10;

    vector<float> THclip;
    vector<float> newValues;
};


