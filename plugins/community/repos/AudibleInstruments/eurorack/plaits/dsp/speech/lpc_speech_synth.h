// Copyright 2016 Olivier Gillet.
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
// LPC10 speech synth.

#ifndef PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_H_
#define PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_H_

#include "stmlib/dsp/dsp.h"

#include "plaits/dsp/dsp.h"

namespace plaits {

const int kLPCOrder = 10;

const float kLPCSpeechSynthDefaultF0 = 100.0f;

class LPCSpeechSynth {
 public:
  LPCSpeechSynth() { }
  ~LPCSpeechSynth() { }

  struct Frame {
    // 14 bytes.
    uint8_t energy;
    uint8_t period;
    int16_t k0;
    int16_t k1;
    int8_t k2;
    int8_t k3;
    int8_t k4;
    int8_t k5;
    int8_t k6;
    int8_t k7;
    int8_t k8;
    int8_t k9;
  };

  void Init();
  
  void Render(
      float prosody_amount,
      float pitch_shift,
      float* excitation,
      float* output,
      size_t size);
  
  void PlayFrame(const Frame* frames, float frame, bool interpolate) {
    MAKE_INTEGRAL_FRACTIONAL(frame);
    
    if (!interpolate) {
      frame_fractional = 0.0f;
    }
    PlayFrame(
        frames[frame_integral],
        frames[frame_integral + 1],
        frame_fractional);
  }

 private:
  void PlayFrame(const Frame& f1, const Frame& f2, float blend);
  
  template <int scale, typename X>
  float BlendCoefficient(X a, X b, float blend) {
    float a_f = static_cast<float>(a) / float(scale);
    float b_f = static_cast<float>(b) / float(scale);
    return a_f + (b_f - a_f) * blend;
  }
  
  float phase_;
  float frequency_;
  float noise_energy_;
  float pulse_energy_;
  
  float next_sample_;
  int excitation_pulse_sample_index_;

  float k_[kLPCOrder];
  float s_[kLPCOrder + 1];

  DISALLOW_COPY_AND_ASSIGN(LPCSpeechSynth);
};

};  // namespace plaits

#endif  // PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_H_
