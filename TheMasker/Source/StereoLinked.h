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

    auto process(vector<float> l, vector<float> r) {
        getMono(l,r, monoValues);
        scaleChannel(l, newL);
        scaleChannel(r, newR);
        return result{ newL, newR };
    }
private:
    vector<float> monoValues;
    vector<float> newL;
    vector<float> newR;
    float UIsl;  
    struct result { vector<float> left;  vector<float> right; };

    void scaleChannel(vector<float>& in, vector<float>& dest) {
        for (int i = 0; i < in.size(); i++) {
            dest[i] = UIsl * monoValues[i] + (1 - UIsl) * in[i];
        }
    }

    void  getMono(vector<float> const& l, vector<float> const& r, vector<float>& dest) {
        dest.clear();
        FloatVectorOperations::addWithMultiply(dest.data(), l.data(), 2.0f, l.size());
        FloatVectorOperations::addWithMultiply(dest.data(), r.data(), 2.0f, r.size());
    }

};

