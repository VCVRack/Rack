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
// Vactrol.

#ifndef STREAMS_VACTROL_H_
#define STREAMS_VACTROL_H_

#include "stmlib/stmlib.h"

#include "streams/meta_parameters.h"
#include "streams/resources.h"

namespace streams {

class Vactrol {
 public:
  Vactrol() { }
  ~Vactrol() { }
  
  void Init();
  void Process(
      int16_t audio,
      int16_t excite,
      uint16_t* gain,
      uint16_t* frequency);

  void Configure(bool alternate, int32_t* parameters, int32_t* globals) {
    uint16_t attack_time;
    uint16_t decay_time;
    
    if (globals) {
      // Attack: 10ms to 1000ms
      attack_time = 128 + (globals[0] >> 8);
      // Decay: 10ms to 5000ms
      decay_time = 128 + (globals[2] * 355 >> 16);
      ComputeAmountOffset(
          parameters[1],
          &target_frequency_amount_,
          &target_frequency_offset_);
    } else {
      uint16_t shape = parameters[0];
      ComputeAmountOffset(
          parameters[1],
          &target_frequency_amount_,
          &target_frequency_offset_);
    
      if (shape < 32768) {
        // attack: 10ms
        attack_time = 128;
        // decay: 50ms to 2000ms
        decay_time = 227 + (shape * 196 >> 15);
      } else if (shape < 49512) {
        shape -= 32768;
        // attack: 10ms to 500ms.
        attack_time = 128 + (shape * 227 >> 15);
        // decay: 2000ms to 1000ms.
        decay_time = 423 - (89 * shape >> 15);
      } else {
        shape -= 49512;
        // attack: 500ms to 50ms.
        attack_time = 355 - (shape >> 7);
        // decay: 1000ms to 100ms.
        decay_time = 384 - (128 * shape >> 15);
      }
    }
    
    attack_coefficient_ = lut_lp_coefficients[attack_time];
    fast_attack_coefficient_ = lut_lp_coefficients[attack_time - 128];
    decay_coefficient_ = lut_lp_coefficients[decay_time];
    fast_decay_coefficient_ = lut_lp_coefficients[decay_time - 128];
    
    plucked_ = alternate;
    if (alternate) {
      fast_attack_coefficient_ <<= 4;
    } else {
      decay_coefficient_ >>= 1;
    }
    
    int32_t ringing_tail = 8192;
    int32_t headroom = 65535 - target_frequency_offset_;
    if (ringing_tail > headroom) {
      ringing_tail = headroom;
    }
    if (ringing_tail > target_frequency_amount_) {
      ringing_tail = target_frequency_amount_;
    }
    
    target_frequency_offset_ += ringing_tail;
    target_frequency_amount_ -= ringing_tail;
  }


 private:
  int32_t target_frequency_amount_;
  int32_t target_frequency_offset_;
  int32_t frequency_amount_;
  int32_t frequency_offset_;
  
  int32_t attack_coefficient_;
  int32_t decay_coefficient_;
  int32_t fast_attack_coefficient_;
  int32_t fast_decay_coefficient_;
  
  int32_t state_[4];
  int32_t excite_;
  
  bool gate_;
  bool plucked_;
  
  DISALLOW_COPY_AND_ASSIGN(Vactrol);
};

}  // namespace streams

#endif  // STREAMS_VACTROL_H_
