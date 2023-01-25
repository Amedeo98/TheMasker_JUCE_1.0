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
        const int memorySize = npoints / nfilts;
        //auto interp = GenericInterpolator< LinearInterpolator, memorySize >::GenericInterpolator();
        //LagrangeInterpolator interp;

        //int npts = freqs->size();
        frequencies = freqs;
        centerF.resize(nfilts);
        values.resize(nfilts,vector<float>(npoints));
        int nb = nfilts;
        float low = conv.hz2bark(frequencies.at(0));
        float high =  conv.hz2bark(frequencies.at(npoints-1));
        float bw = (high - low) / (nfilts + 1);
        centerF = conv.linspace(1.f, (float)nfilts, nfilts);
        FloatVectorOperations::multiply(centerF.data(), bw, nfilts);
        FloatVectorOperations::add(centerF.data(), low, nfilts);
       // FloatVectorOperations::multiply(centerF.data(), 100, nfilts);


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
        supr[supr.size()-1] = (float) frequencies.at(npoints-1);
        int m = 1;
        //vector<float> frequencies = (vector<float>) *frequencies.data();

        for (int b = 0; b < nfilts; b++) {

       /*     interp.process(memorySize,
                frequencies.data(),
                values[b,].data(),
                values[b].size());*/

            vector<float> xw = {infr[b], centerF[b], supr[b]};
            vector<float> yw = { (b==0 ? 1.0f : 0.0f), 1.0f, (b==(nfilts-1) ? 1.0f : 0.0f) };
            int il = findx(frequencies, infr[b]);
            int ih = findx(frequencies, supr[b]);
            int n = ih - il + 1;
            vector<float> buffer, partOfFreqs;
            buffer.resize(nfilts);
            partOfFreqs.resize(nfilts);
            for (int i = 0; i < n; i++) {
                partOfFreqs[i] = frequencies[il + i];
            }
            //copy(frequencies.at(il), frequencies.at(ih), partOfFreqs);
            buffer = interpolateYvector(xw, yw, partOfFreqs, 0);
            vector<float> ptr = values.at(b);
            copy(buffer.begin(), buffer.end(), values.at(b).begin()+il);
            //values[b] = buffer;
            //copy(buffer.at(0), buffer.at(nfilts-1), std::back_inserter(values[b][]));


          /* 
            for (int i = il; ++i <= ih;) {
                    interp.process( memorySize,
                        frequencies.data(),
                        buffer.data(), 
                        buffer.size())
                    ;
                    //values[b - 1, il:ih] = buffer;
            }*/
        }

        //values = newValues;

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

    

    vector<float>interpolateYvector(vector<float>xData, vector<float>yData, vector<float>xx, bool extrapolate)
    {
        vector<float> y_int;
        for (float x : xx) {

            int size = xData.size();

            int i = 0;                                                                  // find left end of interval for interpolation
            if (x >= xData[size - 2]) {                                                 // special case: beyond right end
                i = size - 2;
            }
            else {
                while (x > xData[i + 1]) i++;
            }
            float xL = xData[i], yL = yData[i], xR = xData[i + 1], yR = yData[i + 1];      // points on either side (unless beyond ends)
            if (!extrapolate) {                                                         // if beyond ends of array and not extrapolating
                if (x < xL) yR = yL;
                if (x > xR) yL = yR;
            }

            float dydx = (yR - yL) / (xR - xL);                                    // gradient

            y_int.push_back(yL + dydx * (x - xL));
            //cout << y_int.back() << endl;
        }

        return y_int;
    }


};