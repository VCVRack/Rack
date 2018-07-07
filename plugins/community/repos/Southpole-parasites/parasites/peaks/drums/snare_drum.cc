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
// 808-style snare drum.

#include "peaks/drums/snare_drum.h"

#include <cstdio>

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

#include "peaks/resources.h"

namespace peaks {

using namespace stmlib;

void SnareDrum::Init() {
  excitation_1_up_.Init();
  excitation_1_up_.set_delay(0);
  excitation_1_up_.set_decay(1536);
  
  excitation_1_down_.Init();
  excitation_1_down_.set_delay(1e-3 * 48000);
  excitation_1_down_.set_decay(3072);

  excitation_2_.Init();
  excitation_2_.set_delay(1e-3 * 48000);
  excitation_2_.set_decay(1200);
  
  excitation_noise_.Init();
  excitation_noise_.set_delay(0);
  
  body_1_.Init();
  body_2_.Init();

  noise_.Init();
  noise_.set_resonance(2000);
  noise_.set_mode(SVF_MODE_BP);
  
  set_tone(0);
  set_snappy(32768);
  set_decay(32768);
  set_frequency(0);
}

int16_t SnareDrum::ProcessSingleSample(uint8_t control) {
  if (control & CONTROL_GATE_RISING) {
    excitation_1_up_.Trigger(15 * 32768);
    excitation_1_down_.Trigger(-1 * 32768);
    excitation_2_.Trigger(13107);
    excitation_noise_.Trigger(snappy_);
  }
  
  int32_t excitation_1 = 0;
  excitation_1 += excitation_1_up_.Process();
  excitation_1 += excitation_1_down_.Process();
  excitation_1 += !excitation_1_down_.done() ? 2621 : 0;
  
  int32_t body_1 = body_1_.Process(excitation_1) + (excitation_1 >> 4);
  
  int32_t excitation_2 = 0;
  excitation_2 += excitation_2_.Process();
  excitation_2 += !excitation_2_.done() ? 13107 : 0;

  int32_t body_2 = body_2_.Process(excitation_2) + (excitation_2 >> 4);
  int32_t noise_sample = Random::GetSample();
  int32_t noise = noise_.Process(noise_sample);
  int32_t noise_envelope = excitation_noise_.Process();
  int32_t sd = 0;
  sd += body_1 * gain_1_ >> 15;
  sd += body_2 * gain_2_ >> 15;
  sd += noise_envelope * noise >> 15;
  CLIP(sd);
  return sd;
}

}  // namespace peaks
