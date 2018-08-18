//
//  DSPUtilities.h
//
//
//

#ifndef DSPUtilities_h
#define DSPUtilities_h

namespace rack_plugin_Autodafe {

//==============================================================================

// Calculates the frequency of a given pitch (MIDI) value.
double pitchToFreq(double pitch);

//==============================================================================
// Calculates the pitch (MIDI) of a given frequency value
double freqToPitch(double freq);


//==============================================================================

/**
 Takes a value as input and clips it according to the min and max values.
 Returns the input if (minValue <= in <= maxValue).
 If (in < minValue), then return minValue.
 If (in > maxValue), then return maxValue.
 */
double clipMinMax(double in, double minValue, double maxValue);


//==============================================================================

/**
 Takes a value as input and clips it according to the min value.
 Returns the input if (minValue <= in).
 If (in < minValue), then return minValue.
 */
double clipMin(double in, double minValue);

//==============================================================================

/**
 Crossfades linearly between two values (in0, in1). The value returned is
 determined by the value of the xFadeCtrl argument.
 xFadeCtrl Range: 0->1
 - xFadeCtrl = 0    (only in0 comes through)
 - xFadeCtrl = 0.5  (equal mix of in0 and in1)
 - xfadeCtrl = 1    (only in1 comes through)
 */
double xFadeLin(double xFadeCtrl, double in0, double in1);

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
double parCtrlShaper(double input, double bend);
//==============================================================================

/**
 Normalizes a range of values to the range 0->1.
 (start/end should probably be the range of a parameter)
 - input: the value to be normalized
 - start: the start of the input's range
 - end: the end of the input's range
 Note: (start < end) and (start > end) are both valid.
 */
double normalizeRange(double input, double start, double end);


double resonanceToQ(double resonance);

//==============================================================================

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

#endif /* DSPUtilities_h */
