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
// Number station.

#ifndef PEAKS_NUMBER_STATION_NUMBER_STATION_H_
#define PEAKS_NUMBER_STATION_NUMBER_STATION_H_

#include "stmlib/stmlib.h"

#include "peaks/drums/svf.h"
#include "peaks/gate_processor.h"

namespace peaks {

class NumberStation {
 public:
  NumberStation() { }
  ~NumberStation() { }
  
  void Init();
  void FillBuffer(InputBuffer* input_buffer, OutputBuffer* output_buffer);
  
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      set_tone(parameter[0]);
      set_transition_probability(parameter[1]);
      set_noise(32768);
      set_distortion(32768);
    } else {
      set_tone(parameter[0]);
      set_transition_probability(parameter[1]);
      set_noise(parameter[2]);
      set_distortion(parameter[3]);
    }
  }
  
  inline void set_tone(uint16_t tone) {
    tone_ = (tone >> 2) + 32768 + 8192;
    pitch_shift_ = (tone < 32768) ? (24576 + (tone >> 2)) : 16384 + (tone >> 1);
  }
  
  inline void set_transition_probability(uint16_t transition_probability) {
    transition_probability_ = transition_probability;
  }

  inline void set_distortion(uint16_t distortion) {
    distortion_ = 8192;
    distortion_ += ((32767 - 8192) * static_cast<uint32_t>(distortion) >> 16);
  }
  
  inline void set_noise(uint16_t noise) {
    noise_ = 256 + (noise >> 3);
  }
  
  inline void set_voice(bool voice) {
    voice_ = voice;
  }
  
  inline uint8_t digit() const { return digit_; }
  inline bool gate() const { return gate_; }
  
 private:
  uint16_t tone_;
  uint16_t pitch_shift_;
  uint16_t transition_probability_;
  int32_t distortion_;
  int32_t noise_; 
  uint8_t digit_;

  int32_t drift_;
   
  uint32_t phase_;
  uint32_t noise_phase_;
  uint32_t ringmod_phase_;
  uint32_t interference_phase_;

  int16_t tone_amplitude_;
  int32_t lp_noise_;

  int32_t previous_inner_sample_;
  int32_t previous_outer_sample_;
  
  bool voice_;
  bool gate_;

  Svf lp_;
  Svf hp_;
  
  DISALLOW_COPY_AND_ASSIGN(NumberStation);
};

}  // namespace peaks

#endif  // PEAKS_NUMBER_STATION_NUMBER_STATION_H_
