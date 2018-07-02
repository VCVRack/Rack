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
// FM Voice.

#ifndef RINGS_DSP_FM_VOICE_H_
#define RINGS_DSP_FM_VOICE_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/dsp/filter.h"

#include "rings/dsp/dsp.h"
#include "rings/dsp/follower.h"

#include "rings/resources.h"

namespace rings {

using namespace stmlib;

class FMVoice {
 public:
  FMVoice() { }
  ~FMVoice() { }
  
  void Init();
  void Process(
      const float* in,
      float* out,
      float* aux,
      size_t size);
  
  inline void set_frequency(float frequency) {
    carrier_frequency_ = frequency;
  }
  
  inline void set_ratio(float ratio) {
    ratio_ = ratio;
  }

  inline void set_brightness(float brightness) {
    brightness_ = brightness;
  }
  
  inline void set_damping(float damping) {
    damping_ = damping;
  }
  
  inline void set_position(float position) {
    position_ = position;
  }
  
  inline void set_feedback_amount(float feedback_amount) {
    feedback_amount_ = feedback_amount;
  }
  
  inline void TriggerInternalEnvelope() {
    amplitude_envelope_ = 1.0f;
    brightness_envelope_ = 1.0f;
  }
  
  inline float SineFm(uint32_t phase, float fm) const {
    phase += (static_cast<uint32_t>((fm + 4.0f) * 536870912.0f)) << 3;
    uint32_t integral = phase >> 20;
    float fractional = static_cast<float>(phase << 12) / 4294967296.0f;
    float a = lut_sine[integral];
    float b = lut_sine[integral + 1];
    return a + (b - a) * fractional;
  }
  
 private:
  float carrier_frequency_;
  float ratio_;
  float brightness_;
  float damping_;
  float position_;
  float feedback_amount_;
  
  float previous_carrier_frequency_;
  float previous_modulator_frequency_;
  float previous_brightness_;
  float previous_damping_;
  float previous_feedback_amount_;
  
  float amplitude_envelope_;
  float brightness_envelope_;
  float gain_;
  float fm_amount_;
  uint32_t carrier_phase_;
  uint32_t modulator_phase_;
  float previous_sample_;
  
  Follower follower_;
  
  DISALLOW_COPY_AND_ASSIGN(FMVoice);
};

}  // namespace rings

#endif  // RINGS_DSP_FM_VOICE_H_
