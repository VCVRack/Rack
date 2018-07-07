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
 
#ifndef PEAKS_DRUMS_SNARE_DRUM_H_
#define PEAKS_DRUMS_SNARE_DRUM_H_

#include "stmlib/stmlib.h"

#include "peaks/drums/svf.h"
#include "peaks/drums/excitation.h"

#include <cstdio>

#include "peaks/gate_processor.h"

namespace peaks {

class SnareDrum {
 public:
  SnareDrum() { }
  ~SnareDrum() { }

  void Init();
  int16_t ProcessSingleSample(uint8_t control) IN_RAM;
  
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      set_frequency(0);
      set_decay(32768);
      set_tone(parameter[0]);
      set_snappy(parameter[1]);
    } else {
      set_frequency(parameter[0] - 32768);
      set_tone(parameter[1]);
      set_snappy(parameter[2]);
      set_decay(parameter[3]);
    }
  }

  void set_tone(uint16_t tone) {
    gain_1_ = 22000 - (tone >> 2);
    gain_2_ = 22000 + (tone >> 2);
  }

  void set_snappy(uint16_t snappy) {
    snappy >>= 1;
    if (snappy >= 28672) {
      snappy = 28672;
    }
    snappy_ = 512 + snappy;
  }

  void set_decay(uint16_t decay) {
    body_1_.set_resonance(29000 + (decay >> 5));
    body_2_.set_resonance(26500 + (decay >> 5));
    excitation_noise_.set_decay(4092 + (decay >> 14));
  }
  
  void set_frequency(int16_t frequency) {
    int16_t base_note = 52 << 7;
    int32_t transposition = frequency;
    base_note += transposition * 896 >> 15;
    body_1_.set_frequency(base_note);
    body_2_.set_frequency(base_note + (12 << 7));
    noise_.set_frequency(base_note + (48 << 7));
  }

 private:
  Excitation excitation_1_up_;
  Excitation excitation_1_down_;
  Excitation excitation_2_;
  Excitation excitation_noise_;
  Svf body_1_;
  Svf body_2_;
  Svf noise_;
  
  int32_t gain_1_;
  int32_t gain_2_;
  
  uint16_t snappy_;

  DISALLOW_COPY_AND_ASSIGN(SnareDrum);
};

}  // namespace peaks

#endif  // PEAKS_DRUMS_SNARE_DRUM_H_
