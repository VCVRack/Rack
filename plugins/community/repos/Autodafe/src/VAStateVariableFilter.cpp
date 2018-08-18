/*
  ==============================================================================

    VAStateVariableFilter.cpp
    Created: 10 May 2015 2:41:43pm
    Author:  Jordan Harris

  ==============================================================================
*/

#include "VAStateVariableFilter.h"

namespace rack_plugin_Autodafe {

//==============================================================================

VAStateVariableFilter::VAStateVariableFilter()
{
    sampleRate = 44100.0f;				// default sample rate when constructed
    filterType = SVFLowpass;			// lowpass filter by default

    gCoeff = 1.0f;
    RCoeff = 1.0f;
    KCoeff = 0.0f;

    cutoffFreq = 1000.0f;
    Q = static_cast<float>(resonanceToQ(0.5));

    z1_A[0] = z2_A[0] = 0.0f;
    z1_A[1] = z2_A[1] = 0.0f;

    //smoothTimeMs = 0.0;		        // 0.0 milliseconds
}

VAStateVariableFilter::~VAStateVariableFilter()
{
}

// Member functions for setting the filter's parameters (and sample rate).
//==============================================================================
void VAStateVariableFilter::setFilterType(const int& newType)
{
    filterType = newType;
}

void VAStateVariableFilter::setCutoffPitch(const float& newCutoffPitch)
{
    if (active) {
        cutoffFreq = static_cast<float>(pitchToFreq(newCutoffPitch));
        //cutoffLinSmooth.setValue(cutoffFreq);
        calcFilter();
    }
}

void VAStateVariableFilter::setCutoffFreq(const float& newCutoffFreq)
{
    if (active) {
        cutoffFreq = newCutoffFreq;
        calcFilter();
    }
}

void VAStateVariableFilter::setResonance(const float& newResonance)
{
    if (active) {
        Q = static_cast<float>(resonanceToQ(newResonance));
        calcFilter();
    }
}

void VAStateVariableFilter::setQ(const float& newQ)
{
    if (active) {
        Q = newQ;
        calcFilter();
    }
}

void VAStateVariableFilter::setShelfGain(const float& newGain)
{
    if (active) {
        shelfGain = newGain;
        calcFilter();
    }
}

void VAStateVariableFilter::setFilter(const int& newType, const float& newCutoffFreq,
                                      const float& newResonance, const float& newShelfGain)
{
    filterType = newType;
    cutoffFreq = newCutoffFreq;
    Q = static_cast<float>(resonanceToQ(newResonance));
    shelfGain = newShelfGain;
    calcFilter();
}

void VAStateVariableFilter::setSampleRate(const float& newSampleRate)
{
    sampleRate = newSampleRate;
    //cutoffLinSmooth.reset(sampleRate, smoothTimeMs);
    calcFilter();
}

/*void VAStateVariableFilter::setSmoothingTimeInMs(const float & newSmoothingTimeMs)
{
    smoothTimeMs = newSmoothingTimeMs;
}*/

void VAStateVariableFilter::setIsActive(bool isActive)
{
    active = isActive;
}

//==============================================================================
void VAStateVariableFilter::calcFilter()
{
    if (active) {

        // prewarp the cutoff (for bilinear-transform filters)
        float wd = static_cast<float>(cutoffFreq * 2.0f * M_PI);
        float T = 1.0f / (float)sampleRate;
        float wa = (2.0f / T) * tan(wd * T / 2.0f);

        // Calculate g (gain element of integrator)
        gCoeff = wa * T / 2.0f;			// Calculate g (gain element of integrator)

        // Calculate Zavalishin's R from Q (referred to as damping parameter)
        RCoeff = 1.0f / (2.0f * Q);		
        
        // Gain for BandShelving filter
        KCoeff = shelfGain;				
    }
}

float VAStateVariableFilter::processAudioSample(const float& input, const int& channelIndex)
{
    if (active) {

        // Do the cutoff parameter smoothing per sample.
        //cutoffFreq = cutoffLinSmooth.getNextValue();
        //calcFilter();

        // Filter processing:
        const float HP = (input - (2.0f * RCoeff + gCoeff) * z1_A[channelIndex] - z2_A[channelIndex])
            / (1.0f + (2.0f * RCoeff * gCoeff) + gCoeff * gCoeff);

        const float BP = HP * gCoeff + z1_A[channelIndex];

        const float LP = BP * gCoeff + z2_A[channelIndex];

        const float UBP = 2.0f * RCoeff * BP;

        const float BShelf = input + UBP * KCoeff;

        const float Notch = input - UBP;

        const float AP = input - (4.0f * RCoeff * BP);

        const float Peak = LP - HP;

        z1_A[channelIndex] = gCoeff * HP + BP;		// unit delay (state variable)
        z2_A[channelIndex] = gCoeff * BP + LP;		// unit delay (state variable)

        // Selects which filter type this function will output.
        switch (filterType) {
        case SVFLowpass:
            return LP;
            break;
        case SVFBandpass:
            return BP;
            break;
        case SVFHighpass:
            return HP;
            break;
        case SVFUnitGainBandpass:
            return UBP;
            break;
        case SVFBandShelving:
            return BShelf;
            break;
        case SVFNotch:
            return Notch;
            break;
        case SVFAllpass:
            return AP;
            break;
        case SVFPeak:
            return Peak;
            break;
        default:
            return 0.0f;
            break;
        }
    }
    else {	// If not active, return input
        return input;
    }
}

void VAStateVariableFilter::processAudioBlock(float* const samples,  const int& numSamples, 
                                              const int& channelIndex)
{
    // Test if filter is active. If not, bypass it
    if (active) {

        // Loop through the sample block and process it
        for (int i = 0; i < numSamples; ++i) {
            
            // Do the cutoff parameter smoothing per sample.
            //cutoffFreq = cutoffLinSmooth.getNextValue();
            //calcFilter();       // calculate the coefficients for the smoother

            // Filter processing:
            const float input = samples[i];

            const float HP = (input - (2.0f * RCoeff + gCoeff) * z1_A[channelIndex] - z2_A[channelIndex])
                       / (1.0f + (2.0f * RCoeff * gCoeff) + gCoeff * gCoeff);
            
            const float BP = HP * gCoeff + z1_A[channelIndex];
            
            const float LP = BP * gCoeff + z2_A[channelIndex];

            const float UBP = 2.0f * RCoeff * BP;

            const float BShelf = input + UBP * KCoeff;

            const float Notch = input - UBP;

            const float AP = input - (4.0f * RCoeff * BP);

            const float Peak = LP - HP;

            z1_A[channelIndex] = gCoeff * HP + BP;		// unit delay (state variable)
            z2_A[channelIndex] = gCoeff * BP + LP;		// unit delay (state variable)

            // Selects which filter type this function will output.
            switch (filterType) {
            case SVFLowpass:
                samples[i] = LP;
                break;
            case SVFBandpass:
                samples[i] = BP;
                break;
            case SVFHighpass:
                samples[i] = HP;
                break;
            case SVFUnitGainBandpass:
                samples[i] = UBP;
                break;
            case SVFBandShelving:
                samples[i] = BShelf;
                break;
            case SVFNotch:
                samples[i] = Notch;
                break;
            case SVFAllpass:
                samples[i] = AP;
                break;
            case SVFPeak:
                samples[i] = Peak;
                break;
            default:
                samples[i] = 0.0f;
                break;
            }
        }
    }
}

//==============================================================================

} // namespace rack_plugin_Autodafe
