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
// Sine FM drum - similar to the BD/SD in Anushri.

#ifndef PEAKS_DRUMS_FM_DRUM_H_
#define PEAKS_DRUMS_FM_DRUM_H_

#include "stmlib/stmlib.h"

#include "peaks/gate_processor.h"

namespace peaks {

class FmDrum {
 public:
  FmDrum() { }
  ~FmDrum() { }
  
  void Init();
  void FillBuffer(InputBuffer* input_buffer, OutputBuffer* output_buffer);
  void Morph(uint16_t x, uint16_t y);
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      Morph(parameter[0], parameter[1]);
    } else {
      set_frequency(parameter[0]);
      set_fm_amount((parameter[1] >> 2) * 3);
      set_decay(parameter[2]);
      set_noise(parameter[3]);
    }
  }
  
  inline void set_sd_range(bool sd_range) {
    sd_range_ = sd_range;
  }

  inline void set_frequency(uint16_t frequency) {
    if (frequency <= 16384) {
      aux_envelope_strength_ = 1024;
    } else if (frequency <= 32768) {
      aux_envelope_strength_ = 2048 - (frequency >> 4);
    } else {
      aux_envelope_strength_ = 0;
    }
    frequency_ = (24 << 7) + ((72 << 7) * frequency >> 16);
  }
  
  inline void set_fm_amount(uint16_t fm_amount) {
    fm_amount_ = fm_amount >> 2;
  }

  inline void set_decay(uint16_t decay) {
    am_decay_ = 16384 + (decay >> 1);
    fm_decay_ = 8192 + (decay >> 2);
  }
  
  inline void set_noise(uint16_t noise) {
    uint32_t n = noise;
    noise_ = noise >= 32768 ? ((n - 32768) * (n - 32768) >> 15) : 0;
    noise_ = (noise_ >> 2) * 5;
    overdrive_ = noise <= 32767 ? ((32767 - n) * (32767 - n) >> 14) : 0;
  }
  
 private:
  bool sd_range_;

  uint32_t ComputePhaseIncrement(int16_t midi_pitch) IN_RAM;
  uint32_t ComputeEnvelopeIncrement(uint16_t time) IN_RAM;

  uint16_t aux_envelope_strength_;
  uint16_t frequency_;
  uint16_t fm_amount_;
  uint16_t am_decay_;
  uint16_t fm_decay_;
  uint16_t noise_;
  uint16_t overdrive_;
  int16_t previous_sample_;

  uint32_t phase_;
  uint32_t fm_envelope_phase_;
  uint32_t am_envelope_phase_;
  uint32_t aux_envelope_phase_;
  uint32_t phase_increment_;

  DISALLOW_COPY_AND_ASSIGN(FmDrum);
};

}  // namespace peaks

#endif  // PEAKS_DRUMS_FM_DRUM_H_
