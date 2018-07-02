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
// Follower.

#ifndef STREAMS_FOLLOWER_H_
#define STREAMS_FOLLOWER_H_

#include "stmlib/stmlib.h"

#include "streams/meta_parameters.h"
#include "streams/resources.h"
#include "streams/svf.h"

namespace streams {

const uint16_t kNumBands = 3;

class Follower {
 public:
  Follower() { }
  ~Follower() { }
  
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
      // Attack: 1ms to 100ms
      attack_time = globals[0] >> 8;
      
      // Decay: 10ms to 1000ms
      decay_time = 128 + (globals[2] >> 8);
      
      ComputeAmountOffset(
          parameters[1],
          &target_frequency_amount_,
          &target_frequency_offset_);
    } else {
      uint16_t shape = parameters[0];
      if (shape < 32768) {
        // attack: 1ms to 2ms.
        attack_time = (shape * 39 >> 15);
        // decay: 10ms to 100ms.
        decay_time = 128 + (shape * 128 >> 15);
      } else {
        shape -= 32768;
        // attack: 2ms to 20ms.
        attack_time = 39 + (shape * 128 >> 15);
        // decay: 100ms to 200ms.
        decay_time = 128 + 128 + (shape * 39 >> 15);
      }
      
      ComputeAmountOffset(
          parameters[1],
          &target_frequency_amount_,
          &target_frequency_offset_);
    }
    
    // Slow down the attack detection on low frequencies.
    attack_coefficient_[0] = lut_lp_coefficients[attack_time + 39];
    attack_coefficient_[1] = lut_lp_coefficients[attack_time + 19];
    attack_coefficient_[2] = lut_lp_coefficients[attack_time + 0];
    
    // Slow down the decay detection on high frequencies as there is more noise.
    decay_coefficient_[0] = lut_lp_coefficients[decay_time + 39];
    decay_coefficient_[1] = lut_lp_coefficients[decay_time + 19];
    decay_coefficient_[2] = lut_lp_coefficients[decay_time + 99];
    
    only_filter_ = alternate;
  }

 private:
  Svf analysis_low_;
  Svf analysis_medium_;
  int32_t energy_[kNumBands][2];
  int64_t follower_[kNumBands];

  int64_t attack_coefficient_[kNumBands];
  int64_t decay_coefficient_[kNumBands];
  int64_t follower_lp_[kNumBands];

  int32_t spectrum_[kNumBands];

  int32_t centroid_;
  
  int32_t frequency_offset_;
  int32_t frequency_amount_;
  int32_t target_frequency_offset_;
  int32_t target_frequency_amount_;
  
  int32_t naive_;
  
  bool only_filter_;
  
  DISALLOW_COPY_AND_ASSIGN(Follower);
};

}  // namespace streams

#endif  // STREAMS_FOLLOWER_H_
