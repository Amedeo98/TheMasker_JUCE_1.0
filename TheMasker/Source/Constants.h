/*
  ==============================================================================

    Constants.h
    Created: 28 Mar 2023 10:28:56pm
    Author:  nikid

  ==============================================================================
*/

#pragma once

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
#define DEFAULT_ATQ 0.0f
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
#define _maxGain 10
#define _gateThresh -40
#define _gateKnee 10

#define _atkSmoothingSeconds 0.030f
#define _relSmoothingSeconds 0.100f
#define _atkSpectrumSeconds 0.030f
#define _relSpectrumSeconds 2.000f
#define _overlapRatio 0.5f
#define _mindBFS -100
#define _relThreshLift -12.0f
#define _atqLift 1.6f
#define _spreadExp 0.6f
#define _outExtraGain 7.6f

#define _spectrumPaddingHighFreq 0.5f
#define _spectrumPaddingLowFreq 0.5f
#define _spectrumSkew 1.0f  // moltiplicatore dell'esponenziale che mappa le freq sulle ascisse. 
// se = 1.0f -> basse "strette", se 0.1f basse "larghe", (default 0.2f)

#define _purple Colour(200u, 64u, 164u)
#define _green Colour(40u, 220u, 0u)
#define _yellow Colour(255u, 200u, 100u)
#define _lightGrey Colours::lightgrey
#define _grey Colours::grey


