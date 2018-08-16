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
// Exciter.

#ifndef ELEMENTS_DSP_EXCITER_H_
#define ELEMENTS_DSP_EXCITER_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"
#include "stmlib/utils/random.h"

namespace elements {

enum ExciterModel {
  EXCITER_MODEL_GRANULAR_SAMPLE_PLAYER,
  EXCITER_MODEL_SAMPLE_PLAYER,
  EXCITER_MODEL_MALLET,
  EXCITER_MODEL_PLECTRUM,
  EXCITER_MODEL_PARTICLES,
  EXCITER_MODEL_FLOW,
  EXCITER_MODEL_NOISE
};

enum ExciterFlags {
  EXCITER_FLAG_RISING_EDGE = 1,
  EXCITER_FLAG_FALLING_EDGE = 2,
  EXCITER_FLAG_GATE = 4
};

class Exciter {
 public:
  typedef void (Exciter::*ProcessFn)(const uint8_t, float*, size_t);
   
  Exciter() { }
  ~Exciter() { }
  
  void Init();
  
  inline void set_signature(float signature) {
    signature_ = signature;
  }
  
  inline void set_model(ExciterModel model) {
    model_ = model;
  }
  
  inline void set_parameter(float parameter) {
    parameter_ = parameter;
  }
  
  inline void set_timbre(float timbre) {
    timbre_ = timbre;
  }
  
  inline void set_meta(float meta, ExciterModel first, ExciterModel last) {
    meta *= static_cast<float>(last - first + 1);
    MAKE_INTEGRAL_FRACTIONAL(meta);
    model_ = static_cast<ExciterModel>(first + meta_integral);
    parameter_ = meta_fractional;
    if (model_ > EXCITER_MODEL_NOISE) {
      model_ = EXCITER_MODEL_NOISE;
    }
  }
  
  inline float damping() const {
    return damping_;
  }
  
  inline const stmlib::Svf& filter() const { return lp_; }
  
  void Process(const uint8_t flags, float* out, size_t n);
  void ProcessGranularSamplePlayer(const uint8_t, float*, size_t);
  void ProcessSamplePlayer(const uint8_t, float*, size_t);
  void ProcessMallet(const uint8_t, float*, size_t);
  void ProcessPlectrum(const uint8_t, float*, size_t);
  void ProcessParticles(const uint8_t, float*, size_t);
  void ProcessFlow(const uint8_t, float*, size_t);
  void ProcessNoise(const uint8_t, float*, size_t);
  
 private:
  float GetPulseAmplitude(float cutoff);

  inline float RandomSample() const {
    return static_cast<float>(stmlib::Random::GetWord()) / 4294967296.0f;
  }

  ExciterModel model_;
  float parameter_;
  float timbre_;
  
  stmlib::Svf lp_;
  float damp_state_;
  float particle_state_;
  float particle_range_;
  float damping_;
  float signature_;
  uint32_t phase_;
  uint32_t delay_;
  uint32_t plectrum_delay_;
  
  static ProcessFn fn_table_[];
  
  DISALLOW_COPY_AND_ASSIGN(Exciter);
};

}  // namespace elements

#endif  // ELEMENTS_DSP_EXCITER_H_
