// Copyright 2014 Olivier Gillet.
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
// Simple waveguide tube.

#include "elements/dsp/tube.h"

#include <cstdio>

#include "stmlib/dsp/dsp.h"
#include "stmlib/utils/random.h"

namespace elements {

void Tube::Init() {
  zero_state_ = 0.0f;
  pole_state_ = 0.0f;
  delay_ptr_ = 0;
}

void Tube::Process(
    float frequency,
    float envelope,
    float damping,
    float timbre,
    float* input_output,
    float gain,
    size_t size) {
  float delay = 1.0f / frequency;
  while (delay >= float(kTubeDelaySize)) {
    delay *= 0.5f;
  }
  MAKE_INTEGRAL_FRACTIONAL(delay);
  
  if (envelope >= 1.0f) envelope = 1.0f;
  
  damping = 3.6f - damping * 1.8f;
  float lpf_coefficient = frequency * (1.0f + timbre * timbre * 256.0f);
  if (lpf_coefficient >= 0.995f) lpf_coefficient = 0.995f;
  
  int32_t d = delay_ptr_;;
  while (size--) {
    float breath = *input_output * damping + 0.8f;
    float a = delay_line_[(d + delay_integral) % kTubeDelaySize];
    float b = delay_line_[(d + delay_integral + 1) % kTubeDelaySize];
    float in = a + (b - a) * delay_fractional;
    float pressure_delta = -0.95f * (in * envelope + zero_state_) - breath;
    zero_state_ = in;
    
    float reed = pressure_delta * -0.2f + 0.8f;
    float out = pressure_delta * reed + breath;
    
    CONSTRAIN(out, -5.0f, 5.0f);
    delay_line_[d] = out * 0.5f;
    
    --d;
    if (d < 0) {
      d = kTubeDelaySize - 1;
    }
    pole_state_ += lpf_coefficient * (out - pole_state_);
    *input_output++ += gain * envelope * pole_state_;
  }
  delay_ptr_ = d;
}

}  // namespace elements
