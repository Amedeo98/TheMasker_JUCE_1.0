/*
  ==============================================================================

    FilterBank.h
    Created: 15 Dec 2022 3:06:00pm
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Converters.h"
#include "PluginProcessor.h"
using namespace std;



class FilterBank {
public:
    //FilterBank(std::vector<double>* freqs) {
    //    frequencies = freqs;
    //    centerF.resize(nfilts);
    //    values.resize(nfilts, sizeof(freqs));
    //}

    FilterBank() {}
    ~FilterBank() {}


    std::vector<float> centerF ;

    std::vector<float> getFrequencies() {
        return centerF;
    }  
    
    std::vector<vector<float>> getValues() {
        return values;
    }

    void setConverter(Converter converter) {
        conv = converter;
    }

    Converter getConverter() {
        return conv;
    }



    FilterBank getFilterBank(vector<float> freqs) {
        const int fftSize = 1 << _fftOrder;
        const int memorySize = fftSize / nfilts;
        frequencies = freqs;
        centerF.resize(nfilts);
        values.resize(nfilts,vector<float>(fftSize));
        int nb = nfilts;
        float low = conv.hz2bark(frequencies.at(0));
        float high =  conv.hz2bark(frequencies.at(fftSize -1));
        float bw = (high - low) / nfilts;
        centerF = conv.linspace(1.f, (float)nfilts, nfilts);
        FloatVectorOperations::multiply(centerF.data(), bw, nfilts);
        FloatVectorOperations::add(centerF.data(), low-(bw/2), nfilts);


        vector<float> infr = centerF;
        vector<float> supr = centerF;
        FloatVectorOperations::add(infr.data(), -bw, nfilts);
        FloatVectorOperations::add(supr.data(), bw, nfilts);
        
        for (int i = 0; i < centerF.size(); i++) {
            infr[i] = conv.bark2hz(infr[i]);
            supr[i] = conv.bark2hz(supr[i]);
            centerF[i] = conv.bark2hz(centerF[i]);
        }

        infr[0] = (float) frequencies.at(0);
        //supr[supr.size()-1] = (float) frequencies.at(supr.size()-1);
        int m = 1;
        for (int b = 0; b < nfilts; b++) {
            vector<float> xw = {infr[b], centerF[b], supr[b]};
            vector<float> yw = { (b==0 ? 1.0f : 0.0f), 1.0f, (b==(nfilts-1) ? 1.0f : 0.0f) };
            int il = findx(frequencies, infr[b]);
            int ih = findx(frequencies, supr[b]);
            int n = ih - il + 1;
            vector<float> buffer, partOfFreqs;
            buffer.resize(nfilts);
            partOfFreqs.resize(n);
            for (int i = 0; i < n; i++) {
                partOfFreqs[i] = frequencies[il + i];
            }
            buffer = conv.interpolateYvector(xw, yw, partOfFreqs, 0);
            vector<float> ptr = values.at(b);
            copy(buffer.begin(), buffer.end(), values.at(b).begin()+il);
        }


        return *this;
    }





private:
    vector<float> frequencies;
    vector<vector<float>> values;
    Converter conv;

    const int findx(vector<float> X, float val) {
        int minDistIndex=0;
        for (int i = 0; i < X.size(); i++) {
            X[i] = abs(X[i] - val);
            if (X[i]<X[minDistIndex]) minDistIndex = i;
        }
        return minDistIndex;
    }

    

    


};