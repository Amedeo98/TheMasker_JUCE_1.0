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
        //vector<float> monoValues = mean(l,r);
        //vector<float> temp_l = UIsl * monoValues + (1-UIsl) * l;
        //vector<float> temp_r = UIsl * monoValues + (1-UIsl) * r;
        //return result{ temp_l, temp_r };
    }
private:
    float UIsl;  
    struct result { vector<float> left;  vector<float> right; };

};

