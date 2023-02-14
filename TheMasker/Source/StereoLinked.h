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
        //monoValues.resize(nfilts);
    }
private:
    float monoValues[nfilts];
    float UIsl;  
    
    void scaleChannel(vector<float>& in) {
        for (int i = 0; i < in.size(); i++) {
            in[i] = UIsl * monoValues[i] + (1 - UIsl) * in[i];
        }
    }

    void  getMono(vector<float> l, vector<float> r) {
        FloatVectorOperations::fill(monoValues, 0, nfilts);
        //std::fill(monoValues.begin(), monoValues.end(), 0);
        FloatVectorOperations::addWithMultiply(monoValues, l.data(), 0.5f, nfilts);
        FloatVectorOperations::addWithMultiply(monoValues, r.data(), 0.5f, nfilts);
    }

};

