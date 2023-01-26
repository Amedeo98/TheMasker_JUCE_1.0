/*
  ==============================================================================

    Converters.h
    Created: 16 Dec 2022 11:04:43am
    Author:  nikid

  ==============================================================================
*/

#define nfilts 32
#define maxFreq 22000
#define minFreq 20
#define _fftOrder 10
#define npoints 512
#define blockSize 64
#define hopSizeDividerAsPowOf2 1

#pragma once
class Converter {

public:
    Converter() {};
    ~Converter() {};

    float hz2bark(float f) {

        return 6 * asinh(f / 600);
    };

    float bark2hz(float b) {

        return 600 * sinh(b / 6);
    };

    float db2amp(float db) {
        return (float) pow(10, db / 20);

    };

    float amp2db(float amp) {
        return 20 * log10(amp);

    };

    void toMagnitudeDb(vector<float>& res) {
        for (int i = 0; i < res.size(); i++) {
            res[i] = abs(res[i]);
            res[i] = amp2db(res[i]);
        }
    }

    

    vector<float> mXv_mult(vector<vector<float>> in1, vector<float> in2) {
        
        vector<float> res;
        size_t length = in1.size();
        res.resize(length);
        for (int i = 0; i < length; i++) {
            for (int k = 0; k < in2.size();k++) {
                res[i] = res[i] + in1[i][k] * in2[k];
            }
        }
        return res;
    }

    template <typename T>
    vector<T> linspace(T a, T b, size_t N) {
        T h = (b - a) / static_cast<T>(N - 1);
        std::vector<T> xs(N);
        typename std::vector<T>::iterator x;
        T val;
        for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
            *x = val;
        return xs;
    };



    
};

class Values {
public:
    enum signal {
        input,
        output,
        sidechain
    };
};

