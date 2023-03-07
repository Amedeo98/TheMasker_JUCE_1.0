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


    FilterBank() {}
    ~FilterBank() {}



    void getFrequencies(array<float,nfilts>& fCenters) {
        fCenters = centerF;
    }  
    
    void getValues(array<array<float,npoints>,nfilts>& dest) {
        for (int i = 0; i < nfilts; i++)
            FloatVectorOperations::copy(dest[i].data(), values[i].data(), npoints);
            //dest[i] = values[i].data();
    }




    FilterBank getFilterBank(float* freqs) {
        const int fftSize = npoints;
        const int memorySize = fftSize / nfilts;
        FloatVectorOperations::copy(frequencies.data(), freqs, npoints);
        //frequencies = freqs;
        //centerF.resize(nfilts);
        //values.resize(nfilts,vector<float>(fftSize));
        for (int i = 0; i < nfilts; i++)
            fill(values[i].begin(), values[i].end(), 0);
        int nb = nfilts;
        float low, high;
        conv.hz2bark(frequencies.at(0), low);
        conv.hz2bark(frequencies.at(fftSize -1), high);
        float bw = (high - low) / nfilts;
        vector<float> temp(nfilts);
        temp = conv.linspace(1.f, (float)nfilts, nfilts);
        copy(temp.begin(), temp.end(), centerF.begin());
        FloatVectorOperations::multiply(centerF.data(), bw, nfilts);
        FloatVectorOperations::add(centerF.data(), low-(bw/2), nfilts);


        array<float,nfilts> infr = centerF;
        array<float,nfilts> supr = centerF;
        //float infr[nfilts], supr[nfilts];
        FloatVectorOperations::add(infr.data(), -bw, nfilts);
        FloatVectorOperations::add(supr.data(), bw, nfilts);
        
        for (int i = 0; i < centerF.size(); i++) {
            conv.bark2hz(infr[i], infr[i]);
            conv.bark2hz(supr[i], supr[i]);
            conv.bark2hz(centerF[i], centerF[i]);
        }

        infr[0] = (float) frequencies.at(0);
        supr[nfilts-1] = (float) frequencies.at(frequencies.size()-1);

        int m = 1;
        for (int b = 0; b < nfilts; b++) {
            vector<float> xw = {infr[b], centerF[b], supr[b]};
            vector<float> yw = { (b==0 ? 1.0f : 0.0f), 1.0f, (b==(nfilts-1) ? 1.0f : 0.0f) };
            int il = findx(frequencies, infr[b]);
            int ih = findx(frequencies, supr[b]);
            int n = ih - il + 1;
            vector<float> buffer, partOfFreqs;
            buffer.resize(n);
            partOfFreqs.resize(n);
            for (int i = 0; i < n; i++) {
                partOfFreqs[i] = frequencies[il + i];
            }
            conv.interpolateYvector(xw, yw, partOfFreqs, false, buffer);
            copy(buffer.begin(), buffer.end(), values[b].begin() + il);
            
        }


        return *this;
    }


    array<float,nfilts> centerF;



private:
    array<float, npoints> frequencies;
    array<array<float,npoints>,nfilts> values;
    Converter conv;

    int findx(array<float,npoints> X, float val) {
        int minDistIndex=0;
        int dim = X.size();
        FloatVectorOperations::add(X.data(), -val, dim);
        FloatVectorOperations::abs(X.data(), X.data(), dim);
        for (int i = 0; i < dim; i++) {
            //X[i] = abs(X[i] - val);
            if (X[i] < X[minDistIndex]) minDistIndex = i;
        }
        
        return minDistIndex;
    }

    

    


};