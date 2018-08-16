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
// Vocoder.

#ifndef WARPS_DSP_VOCODER_H_
#define WARPS_DSP_VOCODER_H_

#include "stmlib/stmlib.h"

#include "warps/dsp/filter_bank.h"
#include "warps/dsp/limiter.h"

namespace warps {

const float kFollowerGain = sqrtf(kNumBands);

class EnvelopeFollower {
 public:
  EnvelopeFollower() { }
  ~EnvelopeFollower() { }
  
  void Init() {
    envelope_ = 0.0f;
    freeze_ = false;
    attack_ = decay_ = 0.1f;
    peak_ = 0.0f;
  };
  
  void set_attack(float attack) {
    attack_ = attack;
  }
  
  void set_decay(float decay) {
    decay_ = decay;
  }
  
  void set_freeze(bool freeze) {
    freeze_ = freeze;
  }
  
  void Process(const float* in, float* out, size_t size) {
    float envelope = envelope_;
    float attack = freeze_ ? 0.0f : attack_;
    float decay = freeze_ ? 0.0f : decay_;
    float peak = 0.0f;
    while (size--) {
      float error = fabs(*in++ * kFollowerGain) - envelope;
      envelope += (error > 0.0f ? attack : decay) * error;
      if (envelope > peak) {
        peak = envelope;
      }
      *out++ = envelope;
    }
    envelope_ = envelope;
    float error = peak - peak_;
    peak_ += (error > 0.0f ? 0.5f : 0.1f) * error;
  }
  
  inline float peak() const { return peak_; }
  
 private:
  float attack_;
  float decay_;
  float envelope_;
  float peak_;
  float freeze_;
  
  DISALLOW_COPY_AND_ASSIGN(EnvelopeFollower);
};

struct BandGain {
  float carrier;
  float vocoder;
};

class Vocoder {
 public:
  Vocoder() { }
  ~Vocoder() { }
  
  void Init(float sample_rate);
  void Process(
      const float* modulator,
      const float* carrier,
      float* out,
      size_t size);
  
  void set_release_time(float release_time) {
    release_time_ = release_time;
  }

  void set_formant_shift(float formant_shift) {
    formant_shift_ = formant_shift;
  }

 private:
  float release_time_;
  float formant_shift_;
  
  BandGain previous_gain_[kNumBands];
  BandGain gain_[kNumBands];

  float tmp_[kMaxFilterBankBlockSize];
   
  FilterBank modulator_filter_bank_;
  FilterBank carrier_filter_bank_;
  Limiter limiter_;
  EnvelopeFollower follower_[kNumBands];
  
  DISALLOW_COPY_AND_ASSIGN(Vocoder);
};

}  // namespace warps

#endif  // WARPS_DSP_VOCODER_H_
