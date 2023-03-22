/*
  ==============================================================================

    VolumeMeter.h
    Created: 22 Mar 2023 12:22:11pm
    Author:  nikid

  ==============================================================================
*/
#include <JuceHeader.h>

#pragma once

class VolumeMeter {
public:
    VolumeMeter() {
    }

    void draw(Graphics& g, juce::Rectangle<int>& bounds) {

        //g.fillAll(Colours::white);

        g.setColour(Colours::black);

        auto height = bounds.getHeight();
        auto width = bounds.getWidth();
        // Draw the left channel meter
        float leftLevel = jmax(0.0f, jmin(1.0f, currentLevel[0]));
        g.fillRect(Rectangle<int>(bounds.getWidth() * 0.25f - 50, height - leftLevel * height, 100, leftLevel * height));
        // Draw the right channel meter
        float rightLevel = jmax(0.0f, jmin(1.0f, currentLevel[1]));
        g.fillRect(Rectangle<int>(width * 0.75f - 50, height - rightLevel * height, 100, rightLevel * height));
    }

    //void resized() override {
    //    // Nothing to do here, since we're not adding any child components
    //}

    void setLevel(float left, float right) {
        currentLevel[0] = left;
        currentLevel[1] = right;
    }

private:
    float currentLevel[2] = { 0.0f, 0.0f };

};
