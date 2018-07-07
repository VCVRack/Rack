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
// Float version of Peaks' multistage envelope.

#ifndef ELEMENTS_DSP_MULTISTAGE_ENVELOPE_H_
#define ELEMENTS_DSP_MULTISTAGE_ENVELOPE_H_

#include "stmlib/stmlib.h"

#include "elements/resources.h"

namespace elements {

enum EnvelopeShape {
  ENV_SHAPE_LINEAR,
  ENV_SHAPE_EXPONENTIAL,
  ENV_SHAPE_QUARTIC
};

enum EnvelopeFlags {
  ENVELOPE_FLAG_RISING_EDGE = 1,
  ENVELOPE_FLAG_FALLING_EDGE = 2,
  ENVELOPE_FLAG_GATE = 4
};

const uint16_t kMaxNumSegments = 6;

class MultistageEnvelope {
 public:
  MultistageEnvelope() { }
  ~MultistageEnvelope() { }
  
  void Init();
  inline float Process(uint8_t flags) {
    if (flags & ENVELOPE_FLAG_RISING_EDGE) {
      start_value_ = (segment_ == num_segments_ || hard_reset_)
          ? level_[0]
          : value_;
      segment_ = 0;
      phase_ = 0.0f;
    } else if (flags & ENVELOPE_FLAG_FALLING_EDGE && sustain_point_) {
      start_value_ = value_;
      segment_ = sustain_point_;
      phase_ = 0.0f;
    } else if (phase_ >= 1.0f) {
      start_value_ = level_[segment_ + 1];
      ++segment_;
      phase_ = 0.0f;
      if (segment_ == loop_end_) {
        segment_ = loop_start_;
      }
    }
  
    bool done = segment_ == num_segments_;
    bool sustained = sustain_point_ && segment_ == sustain_point_ &&
        flags & ENVELOPE_FLAG_GATE;
  
    float phase_increment = 0.0f;
    if (!sustained && !done) {
      phase_increment = Interpolate8(lut_env_increments, time_[segment_]);
    }
    float t = Interpolate8(
        lookup_table_table[LUT_ENV_LINEAR + shape_[segment_]],
        phase_);
    phase_ += phase_increment;
    value_ = start_value_ + (level_[segment_ + 1] - start_value_) * t;
    return value_;
  }

  inline void Process(const uint8_t* flags_in, float* out, size_t size) {
    while (size--) {
      *out++ = Process(*flags_in++);
    }
  }

  inline void set_time(uint16_t segment, float time) {
    time_[segment] = time;
  }
  
  inline void set_level(uint16_t segment, float level) {
    level_[segment] = level;
  }
  
  inline void set_num_segments(uint16_t num_segments) {
    num_segments_ = num_segments;
  }
  
  inline void set_sustain_point(float sustain_point) {
    sustain_point_ = sustain_point;
  }
  
  inline void set_adsr(
      float attack,
      float decay,
      float sustain,
      float release) {
    num_segments_ = 3;
    sustain_point_ = 2;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = sustain;
    level_[3] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = release;
    
    shape_[0] = ENV_SHAPE_QUARTIC;
    shape_[1] = ENV_SHAPE_EXPONENTIAL;
    shape_[2] = ENV_SHAPE_EXPONENTIAL;
    
    loop_start_ = loop_end_ = 0;
  }
  
  inline void set_ad(float attack, float decay) {
    num_segments_ = 2;
    sustain_point_ = 0;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_EXPONENTIAL;

    loop_start_ = loop_end_ = 0;
  }
  
  inline void set_adr(
      float attack,
      float decay,
      float sustain,
      float release) {
    num_segments_ = 3;
    sustain_point_ = 0;

    level_[0] = 0.0f;
    level_[1] = 1.0F;
    level_[2] = sustain;
    level_[3] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = sustain;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;

    loop_start_ = loop_end_ = 0;
  }
  
  inline void set_ar(float attack, float decay) {
    num_segments_ = 2;
    sustain_point_ = 1;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;

    loop_start_ = loop_end_ = 0;
  }
  
  inline void set_adsar(
      float attack,
      float decay,
      float sustain,
      float release) {
    num_segments_ = 4;
    sustain_point_ = 2;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = sustain;
    level_[3] = 1.0f;
    level_[4] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = attack;
    time_[3] = release;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
    shape_[3] = ENV_SHAPE_LINEAR;

    loop_start_ = loop_end_ = 0;
  }
  
  inline void set_adar(
      float attack,
      float decay,
      float sustain,
      float release) {
    num_segments_ = 4;
    sustain_point_ = 0;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = sustain;
    level_[3] = 1.0f;
    level_[4] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = attack;
    time_[3] = release;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
    shape_[3] = ENV_SHAPE_LINEAR;

    loop_start_ = loop_end_ = 0;
  }
  
  inline void set_ad_loop(float attack, float decay) {
    num_segments_ = 2;
    sustain_point_ = 0;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    
    loop_start_ = 0;
    loop_end_ = 2;
  }
  
  inline void set_adr_loop(
      float attack,
      float decay,
      float sustain,
      float release) {
    num_segments_ = 3;
    sustain_point_ = 0;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = sustain;
    level_[3] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = release;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
    
    loop_start_ = 0;
    loop_end_ = 3;
  }
  
  inline void set_adar_loop(
      float attack,
      float decay,
      float sustain,
      float release) {
    num_segments_ = 4;
    sustain_point_ = 0;

    level_[0] = 0.0f;
    level_[1] = 1.0f;
    level_[2] = sustain;
    level_[3] = 1.0f;
    level_[4] = 0.0f;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = attack;
    time_[3] = release;

    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
    shape_[3] = ENV_SHAPE_LINEAR;

    loop_start_ = 0;
    loop_end_ = 4;
  }

  inline void set_hard_reset(bool hard_reset) {
    hard_reset_ = hard_reset;
  }
  
 private:
  inline float Interpolate8(const float* table, float index) const {
    index *= 256.0f;
    size_t integral = static_cast<size_t>(index);
    float fractional = index - static_cast<float>(integral);
    float a = table[integral];
    float b = table[integral + 1];
    return a + (b - a) * fractional;
  }

  float level_[kMaxNumSegments];
  float time_[kMaxNumSegments];
  EnvelopeShape shape_[kMaxNumSegments];
  
  int16_t segment_;
  float start_value_;
  float value_;

  float phase_;
  
  uint16_t num_segments_;
  uint16_t sustain_point_;
  uint16_t loop_start_;
  uint16_t loop_end_;

  bool hard_reset_;
  
  DISALLOW_COPY_AND_ASSIGN(MultistageEnvelope);
};

}  // namespace elements

#endif  // ELEMENTS_DSP_MULTISTAGE_ENVELOPE_H_
