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
// Modulator.

#ifndef WARPS_DSP_MODULATOR_H_
#define WARPS_DSP_MODULATOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"

#include "warps/dsp/oscillator.h"
#include "warps/dsp/parameters.h"
#include "warps/dsp/quadrature_oscillator.h"
#include "warps/dsp/quadrature_transform.h"
#include "warps/dsp/sample_rate_converter.h"
#include "warps/dsp/vocoder.h"
#include "warps/resources.h"

namespace warps {

const size_t kMaxBlockSize = 96;
const size_t kOversampling = 6;
const size_t kNumOscillators = 1;

typedef struct { short l; short r; } ShortFrame;
typedef struct { float l; float r; } FloatFrame;

class SaturatingAmplifier {
 public:
  SaturatingAmplifier() { }
  ~SaturatingAmplifier() { }
  void Init() {
    drive_ = 0.0f;
  }
  
  void Process(
      float drive,
      float limit,
      short* in,
      float* out,
      float* out_raw,
      size_t in_stride,
      size_t size) {
    // Process noise gate and compute raw output
    stmlib::ParameterInterpolator drive_modulation(&drive_, drive, size);
    float level = level_;
    for (size_t i = 0; i < size; ++i) {
      float s = static_cast<float>(*in) / 32768.0f;
      float error = s * s - level;
      level += error * (error > 0.0f ? 0.1f: 0.0001f);
      s *= level <= 0.0001f ? (1.0f / 0.0001f) * level : 1.0f;
      out[i] = s;
      out_raw[i] += s * drive_modulation.Next();
      in += in_stride;
    }
    level_ = level;
    
    // Process overdrive / gain
    float drive_2 = drive * drive;
    float pre_gain_a = drive * 0.5f;
    float pre_gain_b = drive_2 * drive_2 * drive * 24.0f;
    float pre_gain = pre_gain_a + (pre_gain_b - pre_gain_a) * drive_2;
    float drive_squished = drive * (2.0f - drive);
    float post_gain = 1.0f / stmlib::SoftClip(
          0.33f + drive_squished * (pre_gain - 0.33f));
    stmlib::ParameterInterpolator pre_gain_modulation(
        &pre_gain_,
        pre_gain,
        size);
    stmlib::ParameterInterpolator post_gain_modulation(
        &post_gain_,
        post_gain,
        size);
    
    for (size_t i = 0; i < size; ++i) {
      float pre = pre_gain_modulation.Next() * out[i];
      float post = stmlib::SoftClip(pre) * post_gain_modulation.Next();
      out[i] = pre + (post - pre) * limit;
    }
  }

 private:
  float level_;
  float drive_;
  float post_gain_;
  float pre_gain_;
  float unclipped_gain_;

  DISALLOW_COPY_AND_ASSIGN(SaturatingAmplifier);
};

enum XmodAlgorithm {
  ALGORITHM_XFADE,
  ALGORITHM_FOLD,
  ALGORITHM_ANALOG_RING_MODULATION,
  ALGORITHM_DIGITAL_RING_MODULATION,
  ALGORITHM_XOR,
  ALGORITHM_COMPARATOR,
  ALGORITHM_NOP,
  ALGORITHM_LAST
};

class Modulator {
 public:
  typedef void (Modulator::*XmodFn)(
      float balance,
      float balance_end,
      float parameter,
      float parameter_end,
      const float* in_1,
      const float* in_2,
      float* out,
      size_t size);

  Modulator() { }
  ~Modulator() { }

  void Init(float sample_rate);
  void Process(ShortFrame* input, ShortFrame* output, size_t size);
  void ProcessEasterEgg(ShortFrame* input, ShortFrame* output, size_t size);
  inline Parameters* mutable_parameters() { return &parameters_; }
  inline const Parameters& parameters() { return parameters_; }
  
  inline bool bypass() const { return bypass_; }
  inline void set_bypass(bool bypass) { bypass_ = bypass; }

  inline bool easter_egg() const { return easter_egg_; }
  inline void set_easter_egg(bool easter_egg) { easter_egg_ = easter_egg; }
  
 private:
  template<XmodAlgorithm algorithm_1, XmodAlgorithm algorithm_2>
  void ProcessXmod(
      float balance,
      float balance_end,
      float parameter,
      float parameter_end,
      const float* in_1,
      const float* in_2,
      float* out,
      size_t size) {
    float step = 1.0f / static_cast<float>(size);
    float parameter_increment = (parameter_end - parameter) * step;
    float balance_increment = (balance_end - balance) * step; 
    while (size) {
      {
        const float x_1 = *in_1++;
        const float x_2 = *in_2++;
        float a = Xmod<algorithm_1>(x_1, x_2, parameter);
        float b = Xmod<algorithm_2>(x_1, x_2, parameter);
        *out++ = a + (b - a) * balance;
        parameter += parameter_increment;
        balance += balance_increment;
        size--;
      }
      {
        const float x_1 = *in_1++;
        const float x_2 = *in_2++;
        float a = Xmod<algorithm_1>(x_1, x_2, parameter);
        float b = Xmod<algorithm_2>(x_1, x_2, parameter);
        *out++ = a + (b - a) * balance;
        parameter += parameter_increment;
        balance += balance_increment;
        size--;
      }
      {
        const float x_1 = *in_1++;
        const float x_2 = *in_2++;
        float a = Xmod<algorithm_1>(x_1, x_2, parameter);
        float b = Xmod<algorithm_2>(x_1, x_2, parameter);
        *out++ = a + (b - a) * balance;
        parameter += parameter_increment;
        balance += balance_increment;
        size--;
      }
    }
  }
  
  template<XmodAlgorithm algorithm>
  static float Xmod(float x_1, float x_2, float parameter);
  
  static float Diode(float x);
  
  bool bypass_;
  bool easter_egg_;
  
  Parameters parameters_;
  Parameters previous_parameters_;
  
  SaturatingAmplifier amplifier_[2];
  Oscillator xmod_oscillator_;
  Oscillator vocoder_oscillator_;
  QuadratureOscillator quadrature_oscillator_;
  
  SampleRateConverter<SRC_UP, kOversampling, 48> src_up_[2];
  SampleRateConverter<SRC_DOWN, kOversampling, 48> src_down_;

  Vocoder vocoder_;
  QuadratureTransform quadrature_transform_[2];
  
  float internal_modulation_[kMaxBlockSize];
  float buffer_[3][kMaxBlockSize];
  float src_buffer_[2][kMaxBlockSize * kOversampling];

  float feedback_sample_;
  
  static XmodFn xmod_table_[];
  
  DISALLOW_COPY_AND_ASSIGN(Modulator);
};

}  // namespace warps

#endif  // WARPS_DSP_MODULATOR_H_
