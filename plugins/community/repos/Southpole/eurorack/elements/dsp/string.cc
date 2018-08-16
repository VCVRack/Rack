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
// Resonator.

#include "elements/dsp/string.h"

#include <cmath>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "elements/dsp/dsp.h"
#include "elements/resources.h"

namespace elements {
  
using namespace std;
using namespace stmlib;

void String::Init(bool enable_dispersion) {
  enable_dispersion_ = enable_dispersion;
  
  string_.Init();
  stretch_.Init();
  fir_damping_filter_.Init();
  iir_damping_filter_.Init();
  
  set_frequency(220.0f / kSampleRate);
  set_dispersion(0.25f);
  set_brightness(0.5f);
  set_damping(0.3f);
  set_position(0.8f);
  
  delay_ = 1.0f / frequency_;
  clamped_position_ = 0.0f;
  previous_dispersion_ = 0.0f;
  dispersion_noise_ = 0.0f;
  curved_bridge_ = 0.0f;
  previous_damping_compensation_ = 0.0f;
  
  out_sample_[0] = out_sample_[1] = 0.0f;
  aux_sample_[0] = aux_sample_[1] = 0.0f;
  
  dc_blocker_.Init(1.0f - 20.0f / kSampleRate);
}

template<bool enable_dispersion>
void String::ProcessInternal(
    const float* in,
    float* out,
    float* aux,
    size_t size) {
  float delay = 1.0f / frequency_;
  CONSTRAIN(delay, 4.0f, kDelayLineSize - 4.0f);
  
  // If there is not enough delay time in the delay line, we play at the
  // lowest possible note and we upsample on the fly with a shitty linear
  // interpolator. We don't care because it's a corner case (f0 < 11.7Hz)
  float src_ratio = delay * frequency_;
  if (src_ratio >= 0.9999f) {
    // When we are above 11.7 Hz, we make sure that the linear interpolator
    // does not get in the way.
    src_phase_ = 1.0f;
    src_ratio = 1.0f;
  }

  float clamped_position = 0.5f - 0.98f * fabs(position_ - 0.5f);
  
  // Linearly interpolate all comb-related CV parameters for each sample.
  ParameterInterpolator delay_modulation(
      &delay_, delay, size);
  ParameterInterpolator position_modulation(
      &clamped_position_, clamped_position, size);
  ParameterInterpolator dispersion_modulation(
      &previous_dispersion_, dispersion_, size);
  
  // For damping/absorption, the interpolation is done in the filter code.
  float lf_damping = damping_ * (2.0f - damping_);
  float rt60 = 0.07f * SemitonesToRatio(lf_damping * 96.0f) * kSampleRate;
  float rt60_base_2_12 = max(-120.0f * delay / src_ratio / rt60, -127.0f);
  float damping_coefficient = SemitonesToRatio(rt60_base_2_12);
  float brightness = brightness_ * brightness_;
  float noise_filter = SemitonesToRatio((brightness_ - 1.0f) * 48.0f);
  float damping_cutoff = min(
      24.0f + damping_ * damping_ * 48.0f + brightness_ * brightness_ * 24.0f,
      84.0f);
  float damping_f = min(frequency_ * SemitonesToRatio(damping_cutoff), 0.499f);
  
  // Crossfade to infinite decay.
  if (damping_ >= 0.95f) {
    float to_infinite = 20.0f * (damping_ - 0.95f);
    damping_coefficient += to_infinite * (1.0f - damping_coefficient);
    brightness += to_infinite * (1.0f - brightness);
    damping_f += to_infinite * (0.4999f - damping_f);
    damping_cutoff += to_infinite * (128.0f - damping_cutoff);
  }
  
  fir_damping_filter_.Configure(damping_coefficient, brightness, size);
  iir_damping_filter_.set_f_q<FREQUENCY_ACCURATE>(damping_f, 0.5f);
  ParameterInterpolator damping_compensation_modulation(
      &previous_damping_compensation_,
      1.0f - Interpolate(lut_svf_shift, damping_cutoff, 1.0f),
      size);
  
  while (size--) {
    src_phase_ += src_ratio;
    if (src_phase_ > 1.0f) {
      src_phase_ -= 1.0f;
      
      float delay = delay_modulation.Next();
      float comb_delay = delay * position_modulation.Next();
    
#ifndef MIC_W
      delay *= damping_compensation_modulation.Next();  // IIR delay.
#endif  // MIC_W
      delay -= 1.0f; // FIR delay.
    
      float s = 0.0f;

      if (enable_dispersion) {
        float noise = 2.0f * Random::GetFloat() - 1.0f;
        noise *= 1.0f / (0.2f + noise_filter);
        dispersion_noise_ += noise_filter * (noise - dispersion_noise_);

        float dispersion = dispersion_modulation.Next();
        float stretch_point = dispersion <= 0.0f
            ? 0.0f
            : dispersion * (2.0f - dispersion) * 0.475f;
        float noise_amount = dispersion > 0.75f
            ? 4.0f * (dispersion - 0.75f)
            : 0.0f;
        float bridge_curving = dispersion < 0.0f
            ? -dispersion
            : 0.0f;
        
        noise_amount = noise_amount * noise_amount * 0.025f;
        float ac_blocking_amount = bridge_curving;

        bridge_curving = bridge_curving * bridge_curving * 0.01f;
        float ap_gain = -0.618f * dispersion / (0.15f + fabs(dispersion));
        
        float delay_fm = 1.0f;
        delay_fm += dispersion_noise_ * noise_amount;
        delay_fm -= curved_bridge_ * bridge_curving;
        delay *= delay_fm;
        
        float ap_delay = delay * stretch_point;
        float main_delay = delay - ap_delay;
        if (ap_delay >= 4.0f && main_delay >= 4.0f) {
          s = string_.ReadHermite(main_delay);
          s = stretch_.Allpass(s, ap_delay, ap_gain);
        } else {
          s = string_.ReadHermite(delay);
        }
        float s_ac = s;
        dc_blocker_.Process(&s_ac, 1);
        s += ac_blocking_amount * (s_ac - s);
        
        float value = fabs(s) - 0.025f;
        float sign = s > 0.0f ? 1.0f : -1.5f;
        curved_bridge_ = (fabs(value) + value) * sign;
      } else {
        s = string_.ReadHermite(delay);
      }
    
      s += *in;  // When f0 < 11.7 Hz, causes ugly bitcrushing on the input!
      s = fir_damping_filter_.Process(s);
#ifndef MIC_W
      s = iir_damping_filter_.Process<FILTER_MODE_LOW_PASS>(s);
#endif  // MIC_W
      string_.Write(s);

      out_sample_[1] = out_sample_[0];
      aux_sample_[1] = aux_sample_[0];

      out_sample_[0] = s;
      aux_sample_[0] = string_.Read(comb_delay);
    }
    *out++ += Crossfade(out_sample_[1], out_sample_[0], src_phase_);
    *aux++ += Crossfade(aux_sample_[1], aux_sample_[0], src_phase_);
    in++;
  }
}

void String::Process(const float* in, float* out, float* aux, size_t size) {
  if (enable_dispersion_) {
    ProcessInternal<true>(in, out, aux, size);
  } else {
    ProcessInternal<false>(in, out, aux, size);
  }
}

}  // namespace elements
