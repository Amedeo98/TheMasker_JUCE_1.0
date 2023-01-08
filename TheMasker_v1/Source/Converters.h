/*
  ==============================================================================

    Converters.h
    Created: 8 Jan 2023 3:47:51pm
    Author:  Amedeo Fresia

  ==============================================================================
*/

#pragma once

#define nfilts 32
#define maxFreq 22000
#define minFreq 20
#define npoints 512
#define blockSize 64

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
    void printVector(vector<T> input) {

        size_t n = sizeof(input) / sizeof(input[0]);
        std::cout << "vector printing : ";
        // loop through the array elements
        for (size_t i = 0; i < n; i++) {
            std::cout << input[i] << ' ';
        }

    }

    
};

class Values {
public:
    enum signal {
        input,
        output,
        sidechain
    };
};


