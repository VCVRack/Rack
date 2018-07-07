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
// Compressor.

#ifndef STREAMS_COMPRESSOR_H_
#define STREAMS_COMPRESSOR_H_

#include "stmlib/stmlib.h"

#include <cstdio>

#include "streams/gain.h"
#include "streams/resources.h"

namespace streams {

class Compressor {
 public:
  Compressor() { }
  ~Compressor() { }
  
  void Init();
  
  void Process(
      int16_t audio,
      int16_t excite,
      uint16_t* gain,
      uint16_t* frequency);
  
  void Configure(bool alternate, int32_t* parameters, int32_t* globals) {
    uint16_t attack_time;
    uint16_t decay_time;
    uint16_t amount;
    uint16_t threshold;
    
    if (globals) {
      attack_time = globals[0] * (128 + 128 + 99) >> 16;  // 1ms to 500ms
      decay_time = 128 + 99 + (globals[2] >> 8);  // 50ms to 5000ms
      threshold = globals[1];
      amount = globals[3];
    } else {
      attack_time = !alternate ? 1 : 40;  // 0.2ms or 2ms;
      decay_time = !alternate ? 279 : 236;  // 150ms or 70ms;
      threshold = parameters[0];
      amount = parameters[1];
    }
    
    attack_coefficient_ = lut_lp_coefficients[attack_time];
    decay_coefficient_ = lut_lp_coefficients[decay_time];
    soft_knee_ = alternate;
    threshold_ = (-1280 + 5 * (threshold >> 8)) << 8;
    
    if (amount < 32768) {
      // Compression with no makeup gain.
      ratio_ = lut_compressor_ratio[(32767 - amount) >> 7];
      makeup_gain_ = 0;
    } else {
      // Adaptive compression with makeup gain.
      amount -= 32768;

      int32_t max_gain, knee_gain;
      max_gain = kMaxExponentialGain;
      makeup_gain_ = amount * (max_gain >> 8) >> 7;
      knee_gain = threshold_+ makeup_gain_;
      if (knee_gain >= 0) {
        makeup_gain_ = -threshold_;
        knee_gain = 0;
      }
      
      if (knee_gain > -4096) {
        // So intense! Brickwall limiter mode. In this case, we use an
        // instant attack to tame transients as soon as they appear.
        ratio_ = 0;  
        attack_coefficient_ = -1;
      } else {
        ratio_ = knee_gain / (threshold_ >> 8);
      }
    }
  }
  
  inline int32_t gain_reduction() const { return gain_reduction_; }
  
 private:
  static int32_t Log2(int32_t value);
  static int32_t Exp2(int32_t value);
  static int32_t Compress(
      int32_t squared_level,
      int32_t threshold,
      int32_t ratio,
      bool soft_knee);

  int32_t ratio_; // Reciprocal of the ratio, 8:8
  int32_t threshold_;
  int32_t makeup_gain_;

  bool soft_knee_;

  int64_t attack_coefficient_;
  int64_t decay_coefficient_;
  int64_t detector_;
  int64_t sidechain_signal_detector_;
  int32_t gain_reduction_;

  DISALLOW_COPY_AND_ASSIGN(Compressor);
};

}  // namespace streams

#endif  // STREAMS_COMPRESSOR_H_
