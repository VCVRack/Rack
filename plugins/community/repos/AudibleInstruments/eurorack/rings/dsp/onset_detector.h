// Copyright 2015 Olivier Gillet.
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
// Onset detector.

#ifndef RINGS_DSP_ONSET_DETECTOR_H_
#define RINGS_DSP_ONSET_DETECTOR_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

namespace rings {

using namespace std;
using namespace stmlib;

class ZScorer {
 public:
  ZScorer() { }
  ~ZScorer() { }
  
  void Init(float cutoff) {
    coefficient_ = cutoff;
    mean_ = 0.0f;
    variance_ = 0.00f;
  }
  
  inline float Normalize(float sample) {
    return Update(sample) / Sqrt(variance_);
  }
  
  inline bool Test(float sample, float threshold) {
    float value = Update(sample);
    return value > Sqrt(variance_) * threshold;
  }

  inline bool Test(float sample, float threshold, float absolute_threshold) {
    float value = Update(sample);
    return value > Sqrt(variance_) * threshold && value > absolute_threshold;
  }

 private:
  inline float Update(float sample) {
    float centered = sample - mean_;
    mean_ += coefficient_ * centered;
    variance_ += coefficient_ * (centered * centered - variance_);
    return centered;
  }
  
  float coefficient_;
  float mean_;
  float variance_;
  
  DISALLOW_COPY_AND_ASSIGN(ZScorer);
};

class Compressor {
 public:  
  Compressor() { }
  ~Compressor() { }
  
  void Init(float attack, float decay, float max_gain) {
    attack_ = attack;
    decay_ = decay;
    level_ = 0.0f;
    skew_ = 1.0f / max_gain;
  }
  
  void Process(const float* in, float* out, size_t size) {
    float level = level_;
    while (size--) {
      SLOPE(level, fabs(*in), attack_, decay_);
      *out++ = *in++ / (skew_ + level);
    }
    level_ = level;
  }
 
 private:
  float attack_; 
  float decay_;
  float level_;
  float skew_;
  
  DISALLOW_COPY_AND_ASSIGN(Compressor);
};

class OnsetDetector {
 public:  
  OnsetDetector() { }
  ~OnsetDetector() { }
  
  void Init(
      float low,
      float low_mid,
      float mid_high,
      float decimated_sr,
      float ioi_time) {
    float ioi_f = 1.0f / (ioi_time * decimated_sr);
    compressor_.Init(ioi_f * 10.0f, ioi_f * 0.05f, 40.0f);
    
    low_mid_filter_.Init();
    mid_high_filter_.Init();
    low_mid_filter_.set_f_q<FREQUENCY_DIRTY>(low_mid, 0.5f);
    mid_high_filter_.set_f_q<FREQUENCY_DIRTY>(mid_high, 0.5f);

    attack_[0] = low_mid;
    decay_[0] = low * 0.25f;

    attack_[1] = low_mid;
    decay_[1] = low * 0.25f;

    attack_[2] = low_mid;
    decay_[2] = low * 0.25f;

    fill(&envelope_[0], &envelope_[3], 0.0f);
    fill(&energy_[0], &energy_[3], 0.0f);
    
    z_df_.Init(ioi_f * 0.05f);
    
    inhibit_time_ = static_cast<int32_t>(ioi_time * decimated_sr);
    inhibit_decay_ = 1.0f / (ioi_time * decimated_sr);
    
    inhibit_threshold_ = 0.0f;
    inhibit_counter_ = 0;
    onset_df_ = 0.0f;
  }
  
  bool Process(const float* samples, size_t size) {
    // Automatic gain control.
    compressor_.Process(samples, bands_[0], size);
    
    // Quick and dirty filter bank - split the signal in three bands.
    mid_high_filter_.Split(bands_[0], bands_[1], bands_[2], size);
    low_mid_filter_.Split(bands_[1], bands_[0], bands_[1], size);

    // Compute low-pass energy and onset detection function
    // (derivative of energy) in each band.
    float onset_df = 0.0f;
    float total_energy = 0.0f;
    for (int32_t i = 0; i < 3; ++i) {
      float* s = bands_[i];
      float energy = 0.0f;
      float envelope = envelope_[i];
      size_t increment = 4 >> i;
      for (size_t j = 0; j < size; j += increment) {
        SLOPE(envelope, s[j] * s[j], attack_[i], decay_[i]);
        energy += envelope;
      }
      energy = Sqrt(energy) * float(increment);
      envelope_[i] = envelope;

      float derivative = energy - energy_[i];
      onset_df += derivative + fabs(derivative);
      energy_[i] = energy;
      total_energy += energy;
    }
    
    onset_df_ += 0.05f * (onset_df - onset_df_);
    bool outlier_in_df = z_df_.Test(onset_df_, 1.0f, 0.01f);
    bool exceeds_energy_threshold = total_energy >= inhibit_threshold_;
    bool not_inhibited = !inhibit_counter_;
    bool has_onset = outlier_in_df && exceeds_energy_threshold && not_inhibited;
    
    if (has_onset) {
      inhibit_threshold_ = total_energy * 1.5f;
      inhibit_counter_ = inhibit_time_;
    } else {
      inhibit_threshold_ -= inhibit_decay_ * inhibit_threshold_;
      if (inhibit_counter_) {
        --inhibit_counter_;
      }
    }
    return has_onset;
  }
  
 private:
  Compressor compressor_;
  NaiveSvf low_mid_filter_;
  NaiveSvf mid_high_filter_;
  
  float attack_[3];
  float decay_[3];
  float energy_[3];
  float envelope_[3];
  float onset_df_;
  
  float bands_[3][32];
  
  ZScorer z_df_;
  
  float inhibit_threshold_;
  float inhibit_decay_;
  int32_t inhibit_time_;
  int32_t inhibit_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(OnsetDetector);
};

}  // namespace rings

#endif  // RINGS_DSP_ONSET_DETECTOR_H_
