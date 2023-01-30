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


    void prepareToPlay() {
        monoValues.resize(nfilts);
    }
private:
    vector<float> monoValues;
    float UIsl;  
    
    void scaleChannel(vector<float>& in) {
        for (int i = 0; i < in.size(); i++) {
            in[i] = UIsl * monoValues[i] + (1 - UIsl) * in[i];
        }
    }

    void  getMono(vector<float> l, vector<float> r) {
        std::fill(monoValues.begin(), monoValues.end(), 0);
        FloatVectorOperations::addWithMultiply(monoValues.data(), l.data(), 0.5f, l.size());
        FloatVectorOperations::addWithMultiply(monoValues.data(), r.data(), 0.5f, r.size());
    }

};

