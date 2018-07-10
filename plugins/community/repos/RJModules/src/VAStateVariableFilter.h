/*
  ==============================================================================

    VAStateVariableFilter.h
    Created: 10 May 2015 2:41:43pm
    Author:  Jordan Harris

    Notes:
    From The Art of VA Filter Design, by Vadim Zavalishin

    TPT State Variable Filter:

    TPT -> Topology Preserving Transform

    This filter is based off of the trapezoidal integrator, which produces
    a bilinear transformation. This filter does the proper cutoff prewarping
    needed for the TPT structure, as explained by Zavalishin. It also handles
    the problem of delayless feedback loops that the trapezoidal integrators
    introduce.

    The SVF has two feedback paths sharing a common path segment. In order to
    obtain a single feedback equation (rather than equation system), a signal
    on the common path should be selected as the unknown variable. The HP filter
    path is chosen as the zero-delay feedback equation's unknown in this filter.
    Using the HP filter path, the rest of the filter's signals in the structure
    can be defined.

  ==============================================================================
*/

#ifndef VASTATEVARIABLEFILTER_H
#define VASTATEVARIABLEFILTER_H

#define _USE_MATH_DEFINES       // to use M_PI

//==============================================================================

#include <cmath>
#include "DSPUtilities.h"
//==============================================================================

using std::pow;
using std::tan;
using std::sqrt;

namespace rack_plugin_RJModules {

//==============================================================================

/** The type of filter that the State Variable Filter will output. */
enum SVFType {
    SVFLowpass = 0,
    SVFBandpass,
    SVFHighpass,
    SVFUnitGainBandpass,
    SVFBandShelving,
    SVFNotch,
    SVFAllpass,
    SVFPeak
};

//==============================================================================
class VAStateVariableFilter {
public:
    /** Create and initialize the filter with default values defined in constructor. */
    VAStateVariableFilter();

    //------------------------------------------------------------------------------

    ~VAStateVariableFilter();

    //------------------------------------------------------------------------------

    /** Sets the type of the filter that processAudioSample() or processAudioBlock() will
        output. This filter can choose between 8 different types using the enums listed
        below or the int given to each.
        0: SVFLowpass
        1: SVFBandpass
        2: SVFHighpass
        3: SVFUnitGainBandpass
        4: SVFBandShelving
        5: SVFNotch
        6: SVFAllpass
        7: SVFPeak
    */
    void setFilterType(const int& newType);

    //------------------------------------------------------------------------------
    /** Used for changing the filter's cutoff parameter logarithmically by
        pitch (MIDI note #)
    */
    void setCutoffPitch(const float& newCutoff);

    //------------------------------------------------------------------------------
    /** Used for changing the filter's cutoff parameter linearly by frequency (Hz) */
    void setCutoffFreq(const float& newCutoff);

    //------------------------------------------------------------------------------
    /** Used for setting the resonance amount. This is then converted to a Q
        value, which is used by the filter.
        Range: (0-1)
    */
    void setResonance(const float& newResonance);

    //------------------------------------------------------------------------------
    /** Used for setting the filter's Q amount. This is then converted to a
        damping parameter called R, which is used in the original filter.
    */
    void setQ(const float& newQ);

    //------------------------------------------------------------------------------
    /** Sets the gain of the shelf for the BandShelving filter only. */
    void setShelfGain(const float& newGain);

    //------------------------------------------------------------------------------
    /** Statically set the filters parameters. */
    void setFilter(const int& newType, const float& newCutoff,
                   const float& newResonance, const float& newShelfGain);

    //------------------------------------------------------------------------------
    /** Set the sample rate used by the host. Needs to be used to accurately
        calculate the coefficients of the filter from the cutoff.
        Note: This is often used in AudioProcessor::prepareToPlay
    */
    void setSampleRate(const float& newSampleRate);

    //------------------------------------------------------------------------------
    /** Sets the time that it takes to interpolate between the previous value and
        the current value. For this filter, the smoothing is only happening for
        the filters cutoff frequency.
    */
    //void setSmoothingTimeInMs(const float& newSmoothingTimeMs);

    //------------------------------------------------------------------------------
    /** Sets whether the filter will process data or not.
        - If (isActive = true) then the filter will process data
        - If (isActive = false) then the filter will be bypassed
    */
    void setIsActive(bool isActive);

    //------------------------------------------------------------------------------
    /** Performs the actual processing for one sample of data, on 2 channels.
        If 2 channels are needed (stereo), use channel index (channelIdx) to
        specify which channel is being processed (i.e. 0 for left, 1 for right).
    */
    float processAudioSample(const float& input, const int& channelIndex);

    //------------------------------------------------------------------------------
    /** Performs the actual processing for a block of samples, on 2 channels.
        If 2 channels are needed (stereo), use channel index (channelIdx) to
        specify which channel is being processed (i.e. 0 for left, 1 for right).
        Note:
        This processes the information sent to the samples argument and
        does it through a pointer. Therefore, no value needs to be
        returned.
    */
    void processAudioBlock(float* const samples, const int& numSamples,
                           const int& channelIndex);

    //------------------------------------------------------------------------------


    double getCutoff(){ return cutoffFreq; }

    double getFilterType(){ return filterType; }

    double getQ(){ return Q; }

    double getShelfGain(){ return shelfGain; }

private:
    //==============================================================================
    //  Calculate the coefficients for the filter based on parameters.
    void calcFilter();

    //  Parameters:
    int filterType;
    float cutoffFreq;
    float Q;
    float shelfGain;

    float sampleRate;
    bool active = true; // is the filter processing or not

    //  Coefficients:
    float gCoeff;       // gain element
    float RCoeff;       // feedback damping element
    float KCoeff;       // shelf gain element

    float z1_A[2], z2_A[2];     // state variables (z^-1)

    // Parameter smoothers:
    //LinearSmoothedValue cutoffLinSmooth;
    //double smoothTimeMs;
};

//==============================================================================
#endif  // VASTATEVARIABLEFILTER_H_INCLUDED


/*
 ==============================================================================

 ParameterSmoother.h
 Created: 1 May 2015 12:43:46am
 Author:  Jordan Harris

 ==============================================================================
 */

/*
 Useful DSP utilities. For instance, calculating frequency from a given
 pitch (MIDI) value, or vice versa.
 */

//==============================================================================

} // namespace rack_plugin_RJModules

