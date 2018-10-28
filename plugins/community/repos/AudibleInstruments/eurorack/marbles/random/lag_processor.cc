// Copyright 2015 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Lag processor for the STEPS control.

#include "marbles/random/lag_processor.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"

#include "marbles/resources.h"

namespace marbles {

using namespace stmlib;

void LagProcessor::Init() {
 ramp_start_ = 0.0f;
 ramp_value_ = 0.0f;
 lp_state_ = 0.0f;
 previous_phase_ = 0.0f;
}

float LagProcessor::Process(float value, float smoothness, float phase) {
  float frequency = phase - previous_phase_;
  if (frequency < 0.0f) {
    frequency += 1.0f;
  }
  previous_phase_ = phase;
  
  // The frequency of the portamento/glide LP filter follows an exponential
  // scale, with a minimum frequency corresponding to half the clock pulse
  // frequency (giving a roughly linear glide), and a maximum value 7 octaves
  // above.
  //
  // When smoothness approaches 0, the response curve is tweaked to give
  // immediate voltage changes, without any lag.
  frequency *= 0.25f;
  frequency *= SemitonesToRatio(84.0f * (1.0f - smoothness));
  if (frequency >= 1.0f) {
    frequency = 1.0f;
  }
  if (smoothness <= 0.05f) {
    frequency += 20.f * (0.05f - smoothness) * (1.0f - frequency);
  }

  ONE_POLE(lp_state_, value, frequency);
  
  // The final output is a crossfade between a variable shape interpolation and
  // the low-pass glide/lag.
  float interp_amount = (smoothness - 0.6f) * 5.0f;
  CONSTRAIN(interp_amount, 0.0f, 1.0f);
  
  float interp_linearity = (1.0f - smoothness) * 5.0f;
  CONSTRAIN(interp_linearity, 0.0f, 1.0f);
  float warped_phase = Interpolate(lut_raised_cosine, phase, 256.0f);
  
  float interp_phase = Crossfade(warped_phase, phase, interp_linearity);
  float interp = Crossfade(ramp_start_, value, interp_phase);
  ramp_value_ = interp;
  
  return Crossfade(lp_state_, interp, interp_amount);
}

}  // namespace marbles