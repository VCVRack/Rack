// Copyright 2013 Olivier Gillet.
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
// 808-style HH.

#include "peaks/drums/high_hat.h"

#include <cstdio>

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

#include "peaks/resources.h"

namespace peaks {

using namespace stmlib;

void HighHat::Init() {
  noise_.Init();
  noise_.set_frequency(105 << 7);  // 8kHz
  noise_.set_resonance(24000);
  noise_.set_mode(SVF_MODE_BP);
  
  vca_coloration_.Init();
  vca_coloration_.set_frequency(110 << 7);  // 13kHz
  vca_coloration_.set_resonance(0);
  vca_coloration_.set_mode(SVF_MODE_HP);
  
  vca_envelope_.Init();
  vca_envelope_.set_delay(0);
  vca_envelope_.set_decay(4093);
}

int16_t HighHat::ProcessSingleSample(uint8_t control) {
  if (control & CONTROL_GATE_RISING) {
    vca_envelope_.Trigger(32768 * 15);
  }
  
  phase_[0] += 48318382;
  phase_[1] += 71582788;
  phase_[2] += 37044092;
  phase_[3] += 54313440;
  phase_[4] += 66214079;
  phase_[5] += 93952409;

  int16_t noise = 0;
  noise += phase_[0] >> 31;
  noise += phase_[1] >> 31;
  noise += phase_[2] >> 31;
  noise += phase_[3] >> 31;
  noise += phase_[4] >> 31;
  noise += phase_[5] >> 31;
  noise <<= 12;
  
  // Run the SVF at the double of the original sample rate for stability.
  int32_t filtered_noise = 0;
  filtered_noise += noise_.Process(noise);
  filtered_noise += noise_.Process(noise);

  // The 808-style VCA amplifies only the positive section of the signal.
  if (filtered_noise < 0) {
    filtered_noise = 0;
  } else if (filtered_noise > 32767) {
    filtered_noise = 32767;
  }
  
  int32_t envelope = vca_envelope_.Process() >> 4;
  int32_t vca_noise = envelope * filtered_noise >> 14;
  CLIP(vca_noise);
  int32_t hh = 0;
  hh += vca_coloration_.Process(vca_noise);
  hh += vca_coloration_.Process(vca_noise);
  hh <<= 1;
  CLIP(hh);
  return hh;
}

}  // namespace peaks
