/*
  ==============================================================================

    StereoLinked.h
    Created: 20 Jan 2023 6:18:09pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

class StereoLinked {

public:

    void setSL(float sl) {
        UIsl = sl;
    }

    void process(vector<float>& l, vector<float>& r) {
        getMono(l, r);
        scaleChannel(l);
        scaleChannel(r);
    }

private:
    float monoValues[nfilts];
    float UIsl;  
    float silence = Decibels::gainToDecibels(0.0f);

    void scaleChannel(vector<float>& in) {
        int size = in.size();
        for (int i = 0; i < size; i++) {
            in[i] = UIsl * monoValues[i] + (1 - UIsl) * in[i];
        }
    }

    void getMono(vector<float> l, vector<float> r) {
        FloatVectorOperations::fill(monoValues, silence, nfilts);
        FloatVectorOperations::addWithMultiply(monoValues, l.data(), 0.5f, nfilts);
        FloatVectorOperations::addWithMultiply(monoValues, r.data(), 0.5f, nfilts);
    }

};

