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

#include "elements/dsp/exciter.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"

#include "elements/dsp/dsp.h"
#include "elements/resources.h"

namespace elements {

using namespace std;
using namespace stmlib;

void Exciter::Init() {
  set_model(EXCITER_MODEL_MALLET);
  set_parameter(0.0f);
  set_timbre(0.99f);

  lp_.Init();
  damp_state_ = 0.0f;
  delay_ = 0;
  plectrum_delay_ = 0;
  particle_state_ = 0.5f;
  damping_ = 0.0f;
  signature_ = 0.0f;
}

float Exciter::GetPulseAmplitude(float cutoff) {
  uint32_t cutoff_index = static_cast<uint32_t>(cutoff * 256.0f);
  return lut_approx_svf_gain[cutoff_index];
}

void Exciter::Process(const uint8_t flags, float* out, size_t size) {
  damping_ = 0.0f;
  (this->*fn_table_[model_])(flags, out, size);
  // Apply filters.
  if (model_ != EXCITER_MODEL_GRANULAR_SAMPLE_PLAYER &&
      model_ != EXCITER_MODEL_SAMPLE_PLAYER) {
    uint32_t cutoff_index = static_cast<uint32_t>(timbre_ * 256.0f);
    if (model_ == EXCITER_MODEL_NOISE) {
      uint32_t resonance_index = static_cast<uint32_t>(parameter_ * 256.0f);
      lp_.set_g_r(
          lut_approx_svf_g[cutoff_index],
          lut_approx_svf_r[resonance_index]);
    } else {
      lp_.set_g_r_h(
          lut_approx_svf_g[cutoff_index],
          2.0f,
          lut_approx_svf_h[cutoff_index]);
    }
    lp_.Process<FILTER_MODE_LOW_PASS>(out, out, size);
  }
}

void Exciter::ProcessGranularSamplePlayer(
    const uint8_t flags, float* out, size_t size) {
  const uint32_t restart_prob = uint32_t(0.01f * 4294967296.0f);
  const uint32_t restart_point = uint32_t(parameter_ * 32767.0f) << 17;
  const uint32_t phase_increment = static_cast<uint32_t>(
      131072.0f * SemitonesToRatio(72.0f * timbre_ - 60.0f));
  const int16_t* base = &smp_noise_sample[static_cast<size_t>(
      signature_ * 8192.0f)];
  
  uint32_t phase = phase_;
  while (size--) {
    uint32_t phase_integral = phase >> 17;
    float phase_fractional = static_cast<float>(phase & 0x1ffff) / 131072.0f;
    float a = static_cast<float>(base[phase_integral]);
    float b = static_cast<float>(base[phase_integral + 1]);
    *out++ = (a + (b - a) * phase_fractional) / 32768.0f;
    phase += phase_increment;
    if (Random::GetWord() < restart_prob) {
      phase = restart_point;
    }
  }
  phase_ = phase;
  damping_ = 0.0f;
}

void Exciter::ProcessSamplePlayer(
    const uint8_t flags, float* out, size_t size) {
  float index = (1.0f - parameter_) * 8.0f;
  MAKE_INTEGRAL_FRACTIONAL(index);
  if (index_integral == 8) {
    index_integral = 7;
    index_fractional = 1.0f;
  }
  
  const uint32_t offset_1 = smp_boundaries[index_integral];
  const uint32_t offset_2 = smp_boundaries[index_integral + 1];
  const uint32_t length_1 = offset_2 - offset_1 - 1;
  const uint32_t length_2 = smp_boundaries[index_integral + 2] - offset_2 - 1;
  const uint32_t phase_increment = static_cast<uint32_t>(
      65536.0f * SemitonesToRatio(72.0f * timbre_ - 36.0f + 7.0f));
  
  float damp = damp_state_;
  uint32_t phase = phase_;

  if (flags & EXCITER_FLAG_RISING_EDGE) {
    damp = 0.0f;
    phase = 0;
  }
  if (!(flags & EXCITER_FLAG_GATE)) {
    damp = 1.0f - 0.95f * (1.0f - damp);
  }
  
  while (size--) {
    uint32_t phase_integral = phase >> 16;
    float phase_fractional = static_cast<float>(phase & 0xffff) / 65536.0f;
    float sample_1 = 0.0f;
    float sample_2 = 0.0f;
    bool step = false;
    if (phase_integral < length_1) {
      const int16_t* base = &smp_sample_data[offset_1 + phase_integral];
      float a = static_cast<float>(base[0]);
      float b = static_cast<float>(base[1]);
      sample_1 = a + (b - a) * phase_fractional;
      step = true;
    }
    if (phase_integral < length_2) {
      const int16_t* base = &smp_sample_data[offset_2 + phase_integral];
      float a = static_cast<float>(base[0]);
      float b = static_cast<float>(base[1]);
      sample_2 = a + (b - a) * phase_fractional;
      step = true;
    }
    if (step) {
      phase += phase_increment;
    }
    
    *out++ = (sample_1 + (sample_2 - sample_1) * index_fractional) / 65536.0f;
  }
  phase_ = phase;
  damping_ = damp * (parameter_ >= 0.8f ? parameter_ * 5.0f - 4.0f : 0.0f);
  damp_state_ = damp;
}

void Exciter::ProcessMallet(const uint8_t flags, float* out, size_t size) {
  fill(&out[0], &out[size], 0.0f);
  if (flags & EXCITER_FLAG_RISING_EDGE) {
    damp_state_ = 0.0f;
    out[0] = GetPulseAmplitude(timbre_);
  }
  if (!(flags & EXCITER_FLAG_GATE)) {
    damp_state_ = 1.0f - 0.95f * (1.0f - damp_state_);
  }
  damping_ = damp_state_ * (1.0f - parameter_);
}

void Exciter::ProcessPlectrum(
    const uint8_t flags,
    float* out,
    size_t size) {
  float amplitude = GetPulseAmplitude(timbre_);
  float damp = damp_state_;
  float impulse = 0.0f;
  if (flags & EXCITER_FLAG_RISING_EDGE) {
    impulse = -amplitude * (0.05f + signature_ * 0.2f);
    plectrum_delay_ = static_cast<uint32_t>(
        4096.0f * parameter_ * parameter_) + 64;
  }
  while (size--) {
    if (plectrum_delay_) {
      --plectrum_delay_;
      if (plectrum_delay_ == 0) {
        impulse = amplitude;
      }
      damp = 1.0f - 0.997f * (1.0f - damp);
    } else {
      damp = 0.9f * damp;
    }
    *out++ = impulse;
    impulse = 0.0f;
  }        
  damping_ = damp * 0.5f;
  damp_state_ = damp;
}

void Exciter::ProcessParticles(
    const uint8_t flags,
    float* out,
    size_t size) {
  if (flags & EXCITER_FLAG_RISING_EDGE) {
    particle_state_ = RandomSample();
    particle_state_ = 1.0f - 0.6f * particle_state_ * particle_state_;
    delay_ = 0;
    particle_range_ = 1.0f;
  }
  fill(&out[0], &out[size], 0.0f);
  if (flags & EXCITER_FLAG_GATE) {
    const uint32_t up_probability = uint32_t(0.7f * 4294967296.0f);
    const uint32_t down_probability = uint32_t(0.3f * 4294967296.0f);
    const float amplitude = GetPulseAmplitude(timbre_);
    while (size--) {
      if (delay_ == 0) {
        float amount = RandomSample();
        amount = 1.05f + 0.5f * amount * amount;
        if (Random::GetWord() > up_probability) {
          particle_state_ *= amount;
          if (particle_state_ >= (particle_range_ + 0.25f)) {
            particle_state_ = particle_range_ + 0.25f;
          }
        } else if (Random::GetWord() < down_probability) {
          particle_state_ /= amount;
          if (particle_state_ <= 0.02f) {
            particle_state_ = 0.02f;
          }
        }
        delay_ = static_cast<uint32_t>(particle_state_ * 0.15f * kSampleRate);
        float gain = 1.0f - particle_range_;
        gain *= gain;
        *out = particle_state_ * amplitude * (1.0f - gain);
        
        float decay_factor = 1.0f - parameter_;
        particle_range_ *= 1.0f - decay_factor * decay_factor * 0.5f;
      } else {
        --delay_;
      }
      ++out;
    }
  }
}

void Exciter::ProcessFlow(
    const uint8_t flags,
    float* out,
    size_t size) {
  float scale = parameter_ * parameter_ * parameter_ * parameter_;
  float threshold = 0.0001f + scale * 0.125f;
  if (flags & EXCITER_FLAG_RISING_EDGE) {
    particle_state_ = 0.5f;
  }
  while (size--) {
    float sample = RandomSample();
    if (sample < threshold) {
      particle_state_ = -particle_state_;
    }
    *out++ = particle_state_ + (sample - 0.5f - particle_state_) * scale;
  }
}

void Exciter::ProcessNoise(const uint8_t flags, float* out, size_t size) {
  while (size--) {
    *out++ = RandomSample() - 0.5f;
  }
}

/* static */
Exciter::ProcessFn Exciter::fn_table_[] = {
  &Exciter::ProcessGranularSamplePlayer,
  &Exciter::ProcessSamplePlayer,
  &Exciter::ProcessMallet,
  &Exciter::ProcessPlectrum,
  &Exciter::ProcessParticles,
  &Exciter::ProcessFlow,
  &Exciter::ProcessNoise
};

}  // namespace elements
