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
        //spreadingMtx.resize(nfilts, vector<float>(nfilts));
        
        float spreadFuncBarkVoltage[2 * nfilts];
        vector<float> temp(nfilts);
        temp = conv.linspaceArray(-maxbark * fbdb, -2.5f, nfilts);
        copy(temp.begin(), temp.end(), ascendent);
        FloatVectorOperations::add(ascendent, -fadB, nfilts);
        temp = conv.linspaceArray(1.0f, -maxbark * fbbdb, nfilts);
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

            //vector<float> temp(nfilts);
            //vector<float>::const_iterator first = spreadFuncBarkVoltage.begin() + nfilts - i - 1;
            //vector<float>::const_iterator last = spreadFuncBarkVoltage.begin() + 2 * nfilts - i - 2;
            copy(spreadFuncBarkVoltage+nfilts - i - 1, spreadFuncBarkVoltage+2 * nfilts - i - 1, spreadingMtx[i].begin());
            //spreadingMtx[i] = temp;
        }

    }

    void spread(vector<float>& input) {
        conv.mXv_mult(spreadingMtx, input.data(), input.size(), input);
    }

    void compareWithAtq(vector<float>& rel_t, array<float,nfilts> atq) {
        FloatVectorOperations::max(rel_t.data(), rel_t.data(), atq.data(), nfilts);
        //rel_t = jmax(rel_t, atq);
    }
private:
    array<array<float,nfilts>,nfilts> spreadingMtx;
    float ascendent[nfilts];
    float descendent[nfilts];
    Converter conv;
    float spread_exp = 0.6f;
};