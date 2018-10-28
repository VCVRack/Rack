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

#include "plaits/dsp/speech/lpc_speech_synth.h"

#include <algorithm>

#include "stmlib/utils/random.h"

#include "plaits/dsp/oscillator/oscillator.h"
#include "plaits/resources.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void LPCSpeechSynth::Init() {
  phase_ = 0.0f;
  frequency_ = 0.0125f;
  noise_energy_ = 0.0f;
  pulse_energy_ = 0.0f;

  next_sample_ = 0.0f;
  excitation_pulse_sample_index_ = 0;

  fill(&k_[0], &k_[kLPCOrder], 0);
  fill(&s_[0], &s_[kLPCOrder + 1], 0);
}

void LPCSpeechSynth::Render(
    float prosody_amount,
    float pitch_shift,
    float* excitation,
    float* output,
    size_t size) {
  const float base_f0 = kLPCSpeechSynthDefaultF0 / 8000.0f;
  float d = frequency_ - base_f0;
  float f = (base_f0 + d * prosody_amount) * pitch_shift;
  CONSTRAIN(f, 0.0f, 0.5f);
  
  float next_sample = next_sample_;
  while (size--) {
    phase_ += f;
    
    float this_sample = next_sample;
    next_sample = 0.0f;
    
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
      float reset_time = phase_ / f;
      int reset_sample = static_cast<int>(32.0f * reset_time);
      
      float discontinuity = 0.0f;
      if (excitation_pulse_sample_index_ < LUT_LPC_EXCITATION_PULSE_SIZE) {
        excitation_pulse_sample_index_ -= reset_sample;
        int8_t s = lut_lpc_excitation_pulse[excitation_pulse_sample_index_];
        discontinuity = static_cast<float>(s) / 128.0f * pulse_energy_;
      }
      
      this_sample += -discontinuity * ThisBlepSample(reset_time);
      next_sample += -discontinuity * NextBlepSample(reset_time);
      
      excitation_pulse_sample_index_ = reset_sample;
    }
    
    float e[11];
    e[10] = Random::GetSample() > 0 ? noise_energy_ : -noise_energy_;
    if (excitation_pulse_sample_index_ < LUT_LPC_EXCITATION_PULSE_SIZE) {
      int8_t s = lut_lpc_excitation_pulse[excitation_pulse_sample_index_];
      next_sample += static_cast<float>(s) / 128.0f * pulse_energy_;
      excitation_pulse_sample_index_ += 32;
    }
    e[10] += this_sample;
    e[10] *= 1.5f;
  
    e[9] = e[10] - k_[9] * s_[9];
    e[8] = e[9] - k_[8] * s_[8];
    e[7] = e[8] - k_[7] * s_[7];
    e[6] = e[7] - k_[6] * s_[6];
    e[5] = e[6] - k_[5] * s_[5];
    e[4] = e[5] - k_[4] * s_[4];
    e[3] = e[4] - k_[3] * s_[3];
    e[2] = e[3] - k_[2] * s_[2];
    e[1] = e[2] - k_[1] * s_[1];
    e[0] = e[1] - k_[0] * s_[0];
  
    CONSTRAIN(e[0], -2.0f, 2.0f);

    s_[9] = s_[8] + k_[8] * e[8];
    s_[8] = s_[7] + k_[7] * e[7];
    s_[7] = s_[6] + k_[6] * e[6];
    s_[6] = s_[5] + k_[5] * e[5];
    s_[5] = s_[4] + k_[4] * e[4];
    s_[4] = s_[3] + k_[3] * e[3];
    s_[3] = s_[2] + k_[2] * e[2];
    s_[2] = s_[1] + k_[1] * e[1];
    s_[1] = s_[0] + k_[0] * e[0];
    s_[0] = e[0];
    
    *excitation++ = e[10];
    *output++ = e[0];
  }
  next_sample_ = next_sample;
}

void LPCSpeechSynth::PlayFrame(const Frame& f1, const Frame& f2, float blend) {
  float frequency_1 = f1.period == 0
      ? frequency_
      : 1.0f / static_cast<float>(f1.period);
  float frequency_2 = f2.period == 0
      ? frequency_
      : 1.0f / static_cast<float>(f2.period);
  frequency_ = frequency_1 + (frequency_2 - frequency_1) * blend;
  
  float energy_1 = static_cast<float>(f1.energy) / 256.0f;
  float energy_2 = static_cast<float>(f2.energy) / 256.0f;
  float noise_energy_1 = f1.period == 0 ? energy_1 : 0.0f;
  float noise_energy_2 = f2.period == 0 ? energy_2 : 0.0f;
  noise_energy_ = noise_energy_1 + (noise_energy_2 - noise_energy_1) * blend;

  float pulse_energy_1 = f1.period != 0 ? energy_1 : 0;
  float pulse_energy_2 = f2.period != 0 ? energy_2 : 0;
  pulse_energy_ = pulse_energy_1 + (pulse_energy_2 - pulse_energy_1) * blend;
  
  k_[0] = BlendCoefficient<32768>(f1.k0, f2.k0, blend);
  k_[1] = BlendCoefficient<32768>(f1.k1, f2.k1, blend);
  k_[2] = BlendCoefficient<128>(f1.k2, f2.k2, blend);
  k_[3] = BlendCoefficient<128>(f1.k3, f2.k3, blend);
  k_[4] = BlendCoefficient<128>(f1.k4, f2.k4, blend);
  k_[5] = BlendCoefficient<128>(f1.k5, f2.k5, blend);
  k_[6] = BlendCoefficient<128>(f1.k6, f2.k6, blend);
  k_[7] = BlendCoefficient<128>(f1.k7, f2.k7, blend);
  k_[8] = BlendCoefficient<128>(f1.k8, f2.k8, blend);
  k_[9] = BlendCoefficient<128>(f1.k9, f2.k9, blend);
}

}  // namespace plaits
