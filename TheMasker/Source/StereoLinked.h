/*
  ==============================================================================

    StereoLinked.h
    Created: 20 Jan 2023 6:18:09pm
    Author:  nikid

  ==============================================================================
*/
#include <JuceHeader.h>
#pragma once

class StereoLinked {

public:
    StereoLinked() {}
    ~StereoLinked() {}

    void setSL(float sl) {
        UIsl = sl;
    }

    void process(array<float,nfilts>& l, array<float,nfilts>& r) {
        getMono(l, r);
        scaleChannel(l);
        scaleChannel(r);
    }

private:
    float monoValues[nfilts];
    float UIsl;  
    //float silence = Decibels::gainToDecibels(0.0f);

    void scaleChannel(array<float, nfilts>& in) {
        int size = in.size();
        for (int i = 0; i < size; i++) {
            in[i] = UIsl * monoValues[i] + (1.0f - UIsl) * in[i];
        }
    }

    void getMono(array<float, nfilts>& l, array<float, nfilts>& r) {
        FloatVectorOperations::fill(monoValues, 0.0f, nfilts);
        FloatVectorOperations::addWithMultiply(monoValues, l.data(), 0.5f, nfilts);
        FloatVectorOperations::addWithMultiply(monoValues, r.data(), 0.5f, nfilts);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoLinked)

};

