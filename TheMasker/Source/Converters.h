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
#define _fftSize (1 << _fftOrder)
#define npoints 512
#define blockSize 64

#pragma once
class Converter {

public:
    Converter() {};
    ~Converter() {};

    void hz2bark(float f, float& res) {

        res = 6 * asinh(f / 600);
    };

    void bark2hz(float b, float& res) {

        res = 600 * sinh(b / 6);
    };

    void db2amp(float db, float& res) {
        res = (float) pow(10, db / 20);

    };

    void amp2db(float amp, float& res) {
        res = 20 * log10(amp);

    };

    void toMagnitudeDb(vector<float>& res) {
        for (int i = 0; i < res.size(); i++) {
            amp2db(abs(res[i]), res[i]);
        }
    }

    

    void mXv_mult(vector<vector<float>> in1, vector<float> in2, vector<float>& dest) {
        
        size_t length = in1.size();
        dest.resize(length);
        for (int i = 0; i < length; i++) {
            for (int k = 0; k < in2.size();k++) {
                dest[i] = dest[i] + in1[i][k] * in2[k];
            }
        }
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


    void interpolateYvector(vector<float>xData, vector<float>yData, vector<float>xx, bool extrapolate, vector<float>& y_int)
    {
        y_int.clear();
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
        }

    }
    
};


