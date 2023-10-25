/*
  ==============================================================================

    Converters.h
    Created: 16 Dec 2022 11:04:43am
    Author:  nikid

  ==============================================================================
*/

#include "Constants.h"

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

    void magnitudeToDb(array<float, nfilts>& res) {
        for (int i = 0; i < res.size(); i++) {
            res[i]=Decibels::gainToDecibels(abs(res[i]));
        }
    }

    void mXv_mult(auto& in1, auto& in2, int in2Size, auto& dest) 
    {
        size_t length = nfilts;

        for (int i = 0; i < length; ++i)
        {
            //float nrg = 0;

            //for (int n = 0; n < in2Size; ++n)
            //{
            //    nrg += in1[i][n];
            //}

            //nrg = 1.0f / nrg;

            for (int k = 0; k < in2Size; ++k)
            {
                dest[i] = dest[i] + (in1[i][k] * in2[k]); // *nrg);
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




    void interpolateYvector(auto& xData, auto& yData, auto& xx, bool extrapolate, auto& y_int)
    {
        int ySize=y_int.size();
        float ySize_inv = pow(ySize, -1);
        FloatVectorOperations::fill(y_int.data(), 0.0f, ySize);
        int xSize = ySize;
        for (int point = 0; point < ySize; point++) {
            int indx = floor(point * xSize * ySize_inv);
            float x = xx[indx];
            
            int size = xData.size();

            int i = 0;                                                                  // find left end of interval for interpolation
            if (x >= xData[size - 2]) {                                                 // special case: beyond right end
                i = size - 2;
            }
            else {
                while (x > xData[i + 1]) i++; // Davvero non c'è un modo più veloce per farlo che usare un while?
            }
            float xL = xData[i], yL = yData[i], xR = xData[i + 1], yR = yData[i + 1];      // points on either side (unless beyond ends)
            if (!extrapolate) {                                                         // if beyond ends of array and not extrapolating
                if (x < xL) yR = yL;
                if (x > xR) yL = yR;
            }

            float dydx = (yR - yL) / (xR - xL);                                    // gradient

            y_int[point] = yL + dydx * (x - xL);
        }

    }
    
};


