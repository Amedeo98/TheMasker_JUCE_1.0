/*
  ==============================================================================

    PSY.h
    Created: 20 Jan 2023 6:17:42pm
    Author:  nikid

  ==============================================================================
*/



#pragma once
class PSY {
public:
    vector<vector<float>> getSpreadingMtx() {
        float fadB = 14.5 + 12;
        float fbdb = 7.5;
        float fbbdb = 26.0;
        float maxbark = conv.hz2bark(maxFreq);
        float alphaScaled = spread_exp / 20.f;
        vector<float> spreadFuncBarkdB(2 * nfilts);
        spreadingMtx.resize(nfilts, vector<float>(nfilts));
        vector<float> spreadFuncBarkVoltage(2 * nfilts);
        vector<float> ascendent = conv.linspace(-maxbark * fbdb, -2.5f, nfilts);
        FloatVectorOperations::add(ascendent.data(), -fadB, nfilts);
        vector<float> descendent = conv.linspace(1.0f, -maxbark * fbbdb, nfilts);
        FloatVectorOperations::add(descendent.data(), -fadB, nfilts);
        copy(ascendent.begin(), ascendent.end(), spreadFuncBarkdB.begin());
        copy(descendent.begin(), descendent.end(), spreadFuncBarkdB.begin() + nfilts);

        FloatVectorOperations::multiply(spreadFuncBarkdB.data(), alphaScaled, 2 * nfilts);
        for (int i = 0; i < 2 * nfilts; i++)
        {
            spreadFuncBarkVoltage[i] = std::pow(10.0f, spreadFuncBarkdB[i]);
        }
        for (int i = 0; i < nfilts; i++) {

            vector<float> temp(nfilts);
            vector<float>::const_iterator first = spreadFuncBarkVoltage.begin() + nfilts - i - 1;
            vector<float>::const_iterator last = spreadFuncBarkVoltage.begin() + 2 * nfilts - i - 2;
            copy(spreadFuncBarkVoltage.begin() + nfilts - i - 1, spreadFuncBarkVoltage.begin() + 2.0f * nfilts - i - 1, temp.begin());
            spreadingMtx[i] = temp;
        }
        return spreadingMtx;

    }

    vector<float> spread(vector<float> input) {
        return conv.mXv_mult(spreadingMtx, input);
    }

    vector<float> compareWithAtq(vector<float> rel_t, vector<float> atq) {
        return jmax(rel_t, atq);
    }
private:
    vector<vector<float>> spreadingMtx;
    Converter conv;
    float spread_exp = 0.6f;
};