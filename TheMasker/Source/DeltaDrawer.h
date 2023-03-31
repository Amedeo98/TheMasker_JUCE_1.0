/*
  ==============================================================================

    DeltaDrawer.h
    Created: 3 Mar 2023 10:09:56am
    Author:  nikid

  ==============================================================================
*/

#pragma once
#include "Drawer.h"
#include "Converters.h"
//#include <CustomSmoothedValue.h>

class DeltaDrawer : public Drawer {
public:
    DeltaDrawer() : Drawer(nfilts, nfilts)
    {
        mindB = (float) -_maxGain;
        maxdB = (float) _maxGain;
    }
    ~DeltaDrawer() {}


    void drawNextFrameOfSpectrum(auto result)
    {

        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i * scope_step) * _spectrumSkew);
            auto fftDataIndex = juce::jlimit(0, resultSize, (int)(_spectrumPaddingLowFreq + skewedProportionX * (float)resultSize * (0.5f + _spectrumPaddingHighFreq)));
            auto level = juce::jmap(result[fftDataIndex], mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }

    void drawFrame(juce::Graphics& g, juce::Rectangle<int>& bounds) override {

        auto width = bounds.getX() + bounds.getWidth() + 4;
        auto height = bounds.getHeight() + bounds.getY();
        auto left = bounds.getX();
        
        //linea sotto la prima frequenza di fCenters (60Hz circa)
        xVal = {(float)left, 
                jmap(freqAxis[0] , 0.f, 1.f, (float)left, (float)width)
        };
        
        yVal = { (float)height*0.5f,
                 jmap(scopeData[0], 0.0f, 1.0f, (float)height, 0.0f)
        };
        

        /*g.setColour(colour
                    .withAlpha(jlimit(0.0f, 1.0f, 40*(abs(scopeData[i]-0.516f))))
        );*/
        
        g.setColour(colour);
        
        juce::Line<float> line (xVal[0], yVal[1], xVal[1], yVal[1]);
        
        //g.drawLine (line, 3.0f);
        Path p;
        p.startNewSubPath (xVal[0], yVal[1]);
        p.quadraticTo (xVal[1], yVal[1], xVal[1], yVal[1]);
        g.strokePath (p, PathStrokeType (2.0));
        
        
        //g.fillRect(xVal[0], yVal[1], xVal[1]-xVal[0], float(height)-yVal[1]);

        //altre linee
        for (int i = 1; i < scopeSize; ++i)
        {
            
            xVal = { jmap(freqAxis[i - 1] , 0.f, 1.f, (float)left, (float)width),
                     jmap(freqAxis[i] , 0.f, 1.f, (float)left, (float)width) 
            };
            
            /*g.setColour(colour
                        .withAlpha(jlimit(0.0f, 1.0f, 40*(abs(scopeData[i]-0.516f))))
            );*/
            
            g.setColour(colour);
            
            yVal = { jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                          jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f)
            };
            
            juce::Line<float> line (xVal[0], yVal[0], xVal[1], yVal[1]);
            //g.drawLine (line, 3.0f);
            
            Path p;
            p.startNewSubPath (xVal[0], yVal[0]);
            p.quadraticTo (xVal[1], yVal[1], xVal[1], yVal[1]);
            g.strokePath (p, PathStrokeType (2.0));
            
            
            //g.fillRect(xVal[0], yVal[0], xVal[1]-xVal[0], float(height)-yVal[0]);
        }
    }

};
