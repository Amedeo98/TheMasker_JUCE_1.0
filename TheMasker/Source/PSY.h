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
    void getSpreadingMtx() {
        float fadB = 14.5 + 12;
        float fbdb = 7.5;
        float fbbdb = 26.0;
        float maxbark;
        conv.hz2bark(maxFreq, maxbark);
        float alphaScaled = spread_exp / 20.f;
        float spreadFuncBarkdB[2 * nfilts];
        
        float spreadFuncBarkVoltage[2 * nfilts];
        vector<float> temp(nfilts);
        temp = conv.linspace(-maxbark * fbdb, -2.5f, nfilts);
        copy(temp.begin(), temp.end(), ascendent);
        FloatVectorOperations::add(ascendent, -fadB, nfilts);
        temp = conv.linspace(1.0f, -maxbark * fbbdb, nfilts);
        copy(temp.begin(), temp.end(), descendent);
        FloatVectorOperations::add(descendent, -fadB, nfilts);
        copy(ascendent, ascendent + nfilts, spreadFuncBarkdB);
        copy(descendent, descendent+nfilts, spreadFuncBarkdB+nfilts);

        FloatVectorOperations::multiply(spreadFuncBarkdB, alphaScaled, 2 * nfilts);
        for (int i = 0; i < 2 * nfilts; i++)
        {
            spreadFuncBarkVoltage[i] = std::pow(10.0f, spreadFuncBarkdB[i]);
        }
        for (int i = 0; i < nfilts; i++) {
            copy(spreadFuncBarkVoltage+nfilts - i - 1, spreadFuncBarkVoltage+2 * nfilts - i - 1, spreadingMtx[i].begin());
        }
    }

    void spread(array<float, nfilts>& input) {
        conv.mXv_mult(spreadingMtx, input, input.size(), input);
    }

    
private:
    array<array<float,nfilts>,nfilts> spreadingMtx;
    float ascendent[nfilts];
    float descendent[nfilts];
    Converter conv;
    float spread_exp = _spreadExp;
    array<float, nfilts> ATQclip;
    array<float, nfilts> atq_gated;
    float gateKnee_inv = pow(_gateKnee, 1);
    float gateThresh = _gateThresh;

};