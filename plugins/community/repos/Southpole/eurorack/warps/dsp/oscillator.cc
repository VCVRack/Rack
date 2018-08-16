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
// Oscillator.

#include "warps/dsp/oscillator.h"

#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/utils/random.h"

namespace warps {

using namespace stmlib;

const float kToFloat = 1.0f / 4294967296.0f;
const float kToUint32 = 4294967296.0f;


void Oscillator::Init(float sample_rate) {
  one_hertz_ = 1.0f / sample_rate;

  next_sample_ = 0.0f;
  phase_ = 0.0f;
  phase_increment_ = 100.0f * one_hertz_;
  hp_state_ = 0.0f;
  lp_state_ = 0.0f;
  
  high_ = false;
  
  external_input_level_ = 0.0f;

  filter_.Init();
}

float Oscillator::Render(
    OscillatorShape shape,
    float note,
    float* modulation,
    float* out,
    size_t size) {
  return (this->*fn_table_[shape])(note, modulation, out, size);
}

float Oscillator::RenderSine(
    float note,
    float* modulation,
    float* out,
    size_t size) {
  float phase = phase_;
  ParameterInterpolator phase_increment(
      &phase_increment_,
      midi_to_increment(note),
      size);
  while (size--) {
    phase += phase_increment.Next();
    if (phase >= 1.0f) {
      phase -= 1.0f;
    }
    uint32_t modulated_phase = static_cast<uint32_t>(phase * kToUint32);
    modulated_phase += static_cast<int32_t>(*modulation++ * 0.5f * kToUint32);
    uint32_t integral = modulated_phase >> 22;
    float fractional = static_cast<float>(modulated_phase << 10) * kToFloat;
    float a = lut_sin[integral];
    float b = lut_sin[integral + 1];
    *out++ = a + (b - a) * fractional;
  }
  phase_ = phase;
  return 1.0f;
}

template<OscillatorShape shape>
float Oscillator::RenderPolyblep(
    float note,
    float* modulation,
    float* out,
    size_t size) {
  float phase = phase_;
  ParameterInterpolator phase_increment(
      &phase_increment_,
      midi_to_increment(note),
      size);
  
  float next_sample = next_sample_;
  bool high = high_;
  float lp_state = lp_state_;
  float hp_state = hp_state_;
  
  while (size--) {
    float this_sample = next_sample;
    next_sample = 0.0f;

    float modulated_increment = phase_increment.Next() * (1.0f + *modulation++);
    
    if (modulated_increment <= 0.0f) {
      modulated_increment = 1.0e-7;
    }
    phase += modulated_increment;
    
    if (shape == OSCILLATOR_SHAPE_TRIANGLE) {
      if (!high && phase >= 0.5f) {
        float t = (phase - 0.5f) / modulated_increment;
        this_sample += ThisBlepSample(t);
        next_sample += NextBlepSample(t);
        high = true;
      }
      if (phase >= 1.0f) {
        phase -= 1.0f;
        float t = phase / modulated_increment;
        this_sample -= ThisBlepSample(t);
        next_sample -= NextBlepSample(t);
        high = false;
      }
      const float integrator_coefficient = modulated_increment * 0.0625f;
      next_sample += phase < 0.5f ? 0.0f : 1.0f;
      this_sample = 128.0f * (this_sample - 0.5f);
      lp_state += integrator_coefficient * (this_sample - lp_state);
      *out++ = lp_state;
    } else {
      if (phase >= 1.0f) {
        phase -= 1.0f;
        float t = phase / modulated_increment;
        this_sample -= ThisBlepSample(t);
        next_sample -= NextBlepSample(t);
      }
      next_sample += phase;
      
      if (shape == OSCILLATOR_SHAPE_SAW) {
        this_sample = this_sample * 2.0f - 1.0f;
        // Slight roll-off of high frequencies - prevent high components near
        // 48kHz that are not eliminated by the upsampling filter.
        lp_state += 0.3f * (this_sample - lp_state);
        *out++ = lp_state;
      } else {
        lp_state += 0.25f * ((hp_state - this_sample) - lp_state);
        *out++ = 4.0f * lp_state;
        hp_state = this_sample;
      }
    }
  }
  
  high_ = high;
  phase_ = phase;
  next_sample_ = next_sample;
  lp_state_ = lp_state;
  hp_state_ = hp_state;
  
  return shape == OSCILLATOR_SHAPE_PULSE
      ?  0.025f / (0.0002f + phase_increment_)
      : 1.0f;
}

float Oscillator::Duck(
    const float* internal,
    const float* external,
    float* destination, size_t size) {
  float level = external_input_level_;
  for (size_t i = 0; i < size; ++i) {
    float error = external[i] * external[i] - level;
    level += ((error > 0.0f) ? 0.01f : 0.0001f) * error;
    float internal_gain = 1.0f - 32.0f * level;
    if (internal_gain <= 0.0f) {
      internal_gain = 0.0f;
    }
    destination[i] = external[i] + internal_gain * (internal[i] - external[i]);
  }
  external_input_level_ = level;
  return level;
}

float Oscillator::RenderNoise(
    float note,
    float* modulation,
    float* out,
    size_t size) {
  for (size_t i = 0; i < size; ++i) {
    float noise = static_cast<float>(stmlib::Random::GetWord()) * kToFloat;
    out[i] = 2.0f * noise - 1.0f;
  }
  Duck(out, modulation, out, size);
  filter_.set_f_q<FREQUENCY_ACCURATE>(midi_to_increment(note) * 4.0f, 1.0f);
  filter_.Process<FILTER_MODE_LOW_PASS>(out, out, size);
  return 1.0f;
}

/* static */
Oscillator::RenderFn Oscillator::fn_table_[] = {
  &Oscillator::RenderSine,
  &Oscillator::RenderPolyblep<OSCILLATOR_SHAPE_TRIANGLE>,
  &Oscillator::RenderPolyblep<OSCILLATOR_SHAPE_SAW>,
  &Oscillator::RenderPolyblep<OSCILLATOR_SHAPE_PULSE>,
  &Oscillator::RenderNoise,
};

}  // namespace warps
