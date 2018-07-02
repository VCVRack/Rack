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
// Resonator.

#include "elements/dsp/resonator.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/cosine_oscillator.h"

#include "elements/drivers/debug_pin.h"

#include "elements/dsp/dsp.h"
#include "elements/resources.h"

namespace elements {

using namespace std;
using namespace stmlib;

void Resonator::Init() {
  for (size_t i = 0; i < kMaxModes; ++i) {
    f_[i].Init();
  }

  for (size_t i = 0; i < kMaxBowedModes; ++i) {
    f_bow_[i].Init();
    d_bow_[i].Init();
  }
  
  set_frequency(220.0f / kSampleRate);
  set_geometry(0.25f);
  set_brightness(0.5f);
  set_damping(0.3f);
  set_position(0.999f);
  set_resolution(kMaxModes);
  
  bow_signal_ = 0.0f;
}

size_t Resonator::ComputeFilters() {
  ++clock_divider_;
  float stiffness = Interpolate(lut_stiffness, geometry_, 256.0f);
  float harmonic = frequency_;
  float stretch_factor = 1.0f; 
  float q = 500.0f * Interpolate(
      lut_4_decades,
      damping_ * 0.8f,
      256.0f);
  float brightness_attenuation = 1.0f - geometry_;
  // Reduces the range of brightness when geometry is very low, to prevent
  // clipping.
  brightness_attenuation *= brightness_attenuation;
  brightness_attenuation *= brightness_attenuation;
  brightness_attenuation *= brightness_attenuation;
  float brightness = brightness_ * (1.0f - 0.2f * brightness_attenuation);
  float q_loss = brightness * (2.0f - brightness) * 0.85f + 0.15f;
  float q_loss_damping_rate = geometry_ * (2.0f - geometry_) * 0.1f;
  size_t num_modes = 0;
  for (size_t i = 0; i < min(kMaxModes, resolution_); ++i) {
    // Update the first 24 modes every time (2kHz). The higher modes are
    // refreshed as a slowest rate.
    bool update = i <= 24 || ((i & 1) == (clock_divider_ & 1));
    float partial_frequency = harmonic * stretch_factor;
    if (partial_frequency >= 0.49f) {
      partial_frequency = 0.49f;
    } else {
      num_modes = i + 1;
    }
    if (update) {
      f_[i].set_f_q<FREQUENCY_FAST>(
          partial_frequency,
          1.0f + partial_frequency * q);
      if (i < kMaxBowedModes) {
        size_t period = 1.0f / partial_frequency;
        while (period >= kMaxDelayLineSize) period >>= 1;
        d_bow_[i].set_delay(period);
        f_bow_[i].set_g_q(f_[i].g(), 1.0f + partial_frequency * 1500.0f);
      }
    }
    stretch_factor += stiffness;
    if (stiffness < 0.0f) {
      // Make sure that the partials do not fold back into negative frequencies.
      stiffness *= 0.93f;
    } else {
      // This helps adding a few extra partials in the highest frequencies.
      stiffness *= 0.98f;
    }
    // This prevents the highest partials from decaying too fast.
    q_loss += q_loss_damping_rate * (1.0f - q_loss);
    harmonic += frequency_;
    q *= q_loss;
  }
  
  return num_modes;
}

void Resonator::Process(
    const float* bow_strength,
    const float* in,
    float* center,
    float* sides,
    size_t size) {
  size_t num_modes = ComputeFilters();
  size_t num_banded_wg = min(kMaxBowedModes, num_modes);
  // Linearly interpolate position. This parameter is extremely sensitive to
  // zipper noise.
  float position_increment = (position_ - previous_position_) / size;
  while (size--) {
    float s;

    // 0.5 Hz LFO used to modulate the position of the stereo side channel.
    lfo_phase_ += modulation_frequency_;
    if (lfo_phase_ >= 1.0f) {
      lfo_phase_ -= 1.0f;
    }
    previous_position_ += position_increment;
    float lfo = lfo_phase_ > 0.5f ? 1.0f - lfo_phase_ : lfo_phase_;
    CosineOscillator amplitudes;
    CosineOscillator aux_amplitudes;
    amplitudes.Init<COSINE_OSCILLATOR_APPROXIMATE>(previous_position_);
    aux_amplitudes.Init<COSINE_OSCILLATOR_APPROXIMATE>(
        modulation_offset_ + lfo);
  
    // Render normal modes.
    float input = *in++ * 0.125f;
    float sum_center = 0.0f;
    float sum_side = 0.0f;

    // Note: For a steady sound, the correct way of simulating the effect of
    // a pickup is to use a comb filter. But it sounds very flange-y when
    // modulated, even mildly, and incur a slight delay/smearing of the
    // attacks.
    // Thus, we directly apply the comb filter in the frequency domain by
    // adjusting the amplitude of each mode in the sum. Because the
    // partials may not be in an integer ratios, what we are doing here is
    // approximative when the stretch factor is non null.
    // It sounds interesting nevertheless.
    amplitudes.Start();
    aux_amplitudes.Start();
    for (size_t i = 0; i < num_modes; i++) {
      s = f_[i].Process<FILTER_MODE_BAND_PASS>(input);
      sum_center += s * amplitudes.Next();
      sum_side += s * aux_amplitudes.Next();
    }
    *sides++ = sum_side - sum_center;
    
    // Render bowed modes.
    float bow_signal = 0.0f;
    input += bow_signal_;
    amplitudes.Start();
    for (size_t i = 0; i < num_banded_wg; ++i) {
      s = 0.99f * d_bow_[i].Read();
      bow_signal += s;
      s = f_bow_[i].Process<FILTER_MODE_BAND_PASS_NORMALIZED>(input + s);
      d_bow_[i].Write(s);
      sum_center += s * amplitudes.Next() * 8.0f;
    }
    bow_signal_ = BowTable(bow_signal, *bow_strength++);
    *center++ = sum_center;
  }
}

}  // namespace elements
