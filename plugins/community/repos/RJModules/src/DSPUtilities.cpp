//
//  DSPUtilities.cpp

//
//

#include "DSPUtilities.h"

#ifndef DSP_UTILITIES
#define DSP_UTILITIES

#include <cmath>

namespace rack_plugin_RJModules {

//==============================================================================

// Calculates the frequency of a given pitch (MIDI) value.
double pitchToFreq(double pitch)
{
    return pow(2, (pitch - 69) / 12) * 440;
}

//==============================================================================
// Calculates the pitch (MIDI) of a given frequency value
double freqToPitch(double freq)
{
    return 69 + 12 * log2(freq / 440);
}

//==============================================================================

/**
 Takes a value as input and clips it according to the min and max values.
 Returns the input if (minValue <= in <= maxValue).
 If (in < minValue), then return minValue.
 If (in > maxValue), then return maxValue.
 */
double clipMinMax(double in, double minValue, double maxValue)
{
    if (in < minValue)
        return minValue;
    else if (in > maxValue)
        return maxValue;
    else
        return in;
}

//==============================================================================

/**
 Takes a value as input and clips it according to the min value.
 Returns the input if (minValue <= in).
 If (in < minValue), then return minValue.
 */
double clipMin(double in, double minValue)
{
    if (in < minValue)
        return minValue;
    else
        return in;
}

//==============================================================================

/**
 Crossfades linearly between two values (in0, in1). The value returned is
 determined by the value of the xFadeCtrl argument.
 xFadeCtrl Range: 0->1
 - xFadeCtrl = 0    (only in0 comes through)
 - xFadeCtrl = 0.5  (equal mix of in0 and in1)
 - xfadeCtrl = 1    (only in1 comes through)
 */
double xFadeLin(double xFadeCtrl, double in0, double in1)
{
    // Clip the xFade parameter to only have range of 0->1
    xFadeCtrl = clipMinMax(xFadeCtrl, 0.0, 1.0);
    // Perform crossfading and return the value
    return (in0 * (1.0 - xFadeCtrl) + in1 * xFadeCtrl);
}

//==============================================================================

/**
 Parabolic Controller Shaper:
 "Bends" the controller curve torwards the X or Y axis.
 input range: (-1..0..1) maps to output range: (-1..0..1).
 bend range: (-1..0..1)
 - bend = -1 (max bend towards X axis)
 - bend = 0 (don't bend)
 - bend = 1 (max bend towards Y axis)
 */
double parCtrlShaper(double input, double bend)
{
    // clip input and bend because the shaper only works in that range.
    input = clipMinMax(input, -1.0, 1.0);
    bend = clipMinMax(bend, -1.0, 1.0);
    return input * ((bend + 1) - fabs(input) * bend);
}

//==============================================================================

/**
 Normalizes a range of values to the range 0->1.
 (start/end should probably be the range of a parameter)
 - input: the value to be normalized
 - start: the start of the input's range
 - end: the end of the input's range
 Note: (start < end) and (start > end) are both valid.
 */
double normalizeRange(double input, double start, double end)
{
    return (input - start) / (end - start);
}


double resonanceToQ(double resonance)
{
    return 1.0 / (2.0 * (1.0 - resonance));
}

//==============================================================================

} // namespace rack_plugin_RJModules

#endif  // DSP_UTILITIES



