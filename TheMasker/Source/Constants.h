/*
  ==============================================================================

    Constants.h
    Created: 28 Mar 2023 10:28:56pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

// if defined shows controllers for fine tuning and actual filter gain (instead of delta), 
// if false tests static correction
// if true tests dynamic correction
//#define fineTuneCoeff true
#define symmetricEdit false

#define ENABLE_CORRECTION true
#define OFFSET_CORRECTION true

#define PLUGIN_NAME "TheMasker"
#define PLUGIN_V 1

#define NAME_MASKEDF "MASKED FREQS"
#define NAME_CLEARF  "CLEAR FREQS"
#define NAME_ATQ "ATQ"
#define NAME_SL "STEREO"
#define NAME_MIX "MIX"
#define NAME_IN "IN"
#define NAME_OUT "OUT"
#define NAME_SC "SIDECHAIN"

#define DEFAULT_MASKEDF 0.0f
#define DEFAULT_CLEARF 0.0f
#define DEFAULT_ATQ 1.0f
#define DEFAULT_SL 0.0f
#define DEFAULT_MIX 1.0f
#define DEFAULT_IN 0.0f
#define DEFAULT_OUT 0.0f
#define DEFAULT_SC 0.0f


#define nfilts 32
#define _fftOrder 11
#define _fftSize (1 << _fftOrder)
#define npoints 256

#define maxFreq 20000
#define minFreq 20

#define _maxDeltaSpectrum 24
#define _maxGain 18
#define _mixMultiplier 1.05f
#define _gateThresh -60
#define _gateKnee 5

#define _atkSmoothingSeconds 0.080f
#define _relSmoothingSeconds 0.250f
#define _atkSpectrumSeconds 0.030f
#define _relSpectrumSeconds 2.000f
#define _overlapRatio 0.5f
#define _mindBFS -100
#define _relThreshLift -9.0f
#define _relThreshBoost +4.0f
#define _atqLift 1.6f
#define _spreadExp 0.6f
#define _outExtraGain 7.52f

#define _spectrumPaddingHighFreq 0.5f
#define _spectrumPaddingLowFreq 0.5f
#define _spectrumSkew 1.0f
#define _spectrumThresh -85
#define _spectrumKnee 10
#define _editorRefreshRate 25

#define _purple Colour(154u, 61u, 157u)
#define _green Colour(40u, 220u, 0u)
#define _yellow Colour(255u, 200u, 100u)
#define _lightgrey Colours::lightgrey
#define _grey Colours::grey
#define _darkgrey Colour(10u, 10u, 10u)

#define _snrWindow 20000
