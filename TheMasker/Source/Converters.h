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
#define npoints 256
#define _maxGain 20
#define _mindBFS -64
#define _relThreshLift 0.0f
#define _atqLift 1.6f
#define _spreadExp 0.6f
#define _outExtraGain 8.0f
#define _spectrumSkew 1.0f  // moltiplicatore dell'esponenziale che mappa le freq sulle ascisse. 
                            // se = 1.0f -> basse "strette", se 0.1f basse "larghe", (default 0.2f)

#pragma once
class Converter {

public:
    Converter() {};
    ~Converter() {};

    void hz2bark(float f, float& res) {

        res = 6 * asinh(f / 600);
    };

    void bark2hz(float b, float& dest) {

        dest = 600 * sinh(b / 6);
    };

    void db2amp(float db, float& res) {
        res = (float) pow(10, db / 20);

    };

    void amp2db(float amp, float& res) {
        res = 20 * log10(amp);

    };

    void toMagnitudeDb(vector<float>& res) {
        for (int i = 0; i < res.size(); i++) {
            res[i]=Decibels::gainToDecibels(abs(res[i]));
            //amp2db(abs(res[i]), res[i]);
        }
    }

    

    void mXv_mult(auto in1, auto in2, int in2Size, auto& dest) {
        
        size_t length = nfilts;
        //dest.resize(length);
        for (int i = 0; i < length; i++) {
            for (int k = 0; k < in2Size;k++) {
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

    template <typename T>
    vector<T> linspaceArray(T a, T b, size_t N) {
        T h = (b - a) / static_cast<T>(N - 1);
        //size_t const dim = static_cast<size_t>(N);
        vector<T> xs(N);
        for (size_t i = 0; i < N; i++) {
            xs[i] = a + h * i;
        }
        /*T* x;
        T val;
        for (x = *xs.begin(), val = a; x != *xs.end(); ++x, val += h)
            *x = val;*/
        return xs;
    };


    void interpolateYvector(vector<float>xData, vector<float>yData, auto xx, bool extrapolate, vector<float>& y_int)
    {
        int ySize=y_int.size();
        float ySize_inv = pow(ySize, -1);
        FloatVectorOperations::fill(y_int.data(), 0.0f, ySize);
        int xSize = xx.size();
        for (int point = 0; point < ySize; point++) {
            int indx = floor(point * xSize * ySize_inv);
            float x = xx[indx];
            
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

            y_int[point] = yL + dydx * (x - xL);
        }
        //return y_int;

    }
    
};


