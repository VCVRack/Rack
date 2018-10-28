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
// 2 variable shape oscillators with sync, FM and crossfading.

#include "plaits/dsp/engine/virtual_analog_engine.h"

#include <algorithm>

#include "stmlib/dsp/parameter_interpolator.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void VirtualAnalogEngine::Init(BufferAllocator* allocator) {
  primary_.Init();
  auxiliary_.Init();
  sync_.Init();
  variable_saw_.Init();
  
  auxiliary_amount_ = 0.0f;
  xmod_amount_ = 0.0f;
  
  temp_buffer_ = allocator->Allocate<float>(kMaxBlockSize);
}

void VirtualAnalogEngine::Reset() {
  
}

const float intervals[5] = {
  0.0f, 7.01f, 12.01f, 19.01f, 24.01f
};

inline float Squash(float x) {
  return x * x * (3.0f - 2.0f * x);
}

float VirtualAnalogEngine::ComputeDetuning(float detune) const {
  detune = 2.05f * detune - 1.025f;
  CONSTRAIN(detune, -1.0f, 1.0f);
  
  float sign = detune < 0.0f ? -1.0f : 1.0f;
  detune = detune * sign * 3.9999f;
  MAKE_INTEGRAL_FRACTIONAL(detune);
  
  float a = intervals[detune_integral];
  float b = intervals[detune_integral + 1];
  return (a + (b - a) * Squash(Squash(detune_fractional))) * sign;
}

void VirtualAnalogEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {

#if VA_VARIANT == 0
  
  // 1 = variable waveshape controlled by TIMBRE.
  // 2 = variable waveshape controlled by MORPH, detuned by HARMONICS.
  // OUT = 1 + 2.
  // AUX = 1 + sync 2.
  const float auxiliary_detune = ComputeDetuning(parameters.harmonics);
  const float primary_f = NoteToFrequency(parameters.note);
  const float auxiliary_f = NoteToFrequency(parameters.note + auxiliary_detune);
  const float sync_f = NoteToFrequency(
      parameters.note + parameters.harmonics * 48.0f);

  float shape_1 = parameters.timbre * 1.5f;
  CONSTRAIN(shape_1, 0.0f, 1.0f);

  float pw_1 = 0.5f + (parameters.timbre - 0.66f) * 1.4f;
  CONSTRAIN(pw_1, 0.5f, 0.99f);

  float shape_2 = parameters.morph * 1.5f;
  CONSTRAIN(shape_2, 0.0f, 1.0f);

  float pw_2 = 0.5f + (parameters.morph - 0.66f) * 1.4f;
  CONSTRAIN(pw_2, 0.5f, 0.99f);
  
  primary_.Render<false>(
      primary_f, primary_f, pw_1, shape_1, temp_buffer_, size);
  auxiliary_.Render<false>(auxiliary_f, auxiliary_f, pw_2, shape_2, aux, size);
  for (size_t i = 0; i < size; ++i) {
    out[i] = (aux[i] + temp_buffer_[i]) * 0.5f;
  }
  
  sync_.Render<true>(primary_f, sync_f, pw_2, shape_2, aux, size);
  for (size_t i = 0; i < size; ++i) {
    aux[i] = (aux[i] + temp_buffer_[i]) * 0.5f;
  }

#elif VA_VARIANT == 1
  
  // 1 = variable waveshape controlled by MORPH.
  // 2 = variable waveshape controlled by MORPH.
  // OUT = crossfade between 1 + 2, 1, 1 sync 2 controlled by TIMBRE.
  // AUX = 2.
  
  float auxiliary_amount = max(0.5f - parameters.timbre, 0.0f) * 2.0f;
  auxiliary_amount *= auxiliary_amount * 0.5f;

  const float xmod_amount = max(parameters.timbre - 0.5f, 0.0f) * 2.0f;
  const float squashed_xmod_amount = xmod_amount * (2.0f - xmod_amount);

  const float auxiliary_detune = ComputeDetuning(parameters.harmonics);
  const float primary_f = NoteToFrequency(parameters.note);
  const float auxiliary_f = NoteToFrequency(parameters.note + auxiliary_detune);
  const float sync_f = primary_f * SemitonesToRatio(
      xmod_amount * (auxiliary_detune + 36.0f));

  float shape = parameters.morph * 1.5f;
  CONSTRAIN(shape, 0.0f, 1.0f);

  float pw = 0.5f + (parameters.morph - 0.66f) * 1.4f;
  CONSTRAIN(pw, 0.5f, 0.99f);

  primary_.Render<false>(primary_f, primary_f, pw, shape, out, size);
  sync_.Render<true>(primary_f, sync_f, pw, shape, aux, size);

  ParameterInterpolator xmod_amount_modulation(
      &xmod_amount_,
      squashed_xmod_amount * (2.0f - squashed_xmod_amount),
      size);
  for (size_t i = 0; i < size; ++i) {
    out[i] += (aux[i] - out[i]) * xmod_amount_modulation.Next();
  }

  auxiliary_.Render<false>(auxiliary_f, auxiliary_f, pw, shape, aux, size);

  ParameterInterpolator auxiliary_amount_modulation(
      &auxiliary_amount_,
      auxiliary_amount,
      size);
  for (size_t i = 0; i < size; ++i) {
    out[i] += (aux[i] - out[i]) * auxiliary_amount_modulation.Next();
  }
  
#elif VA_VARIANT == 2

  // 1 = variable square controlled by TIMBRE.
  // 2 = variable saw controlled by MORPH.
  // OUT = 1 + 2.
  // AUX = dual variable waveshape controlled by MORPH, self sync by TIMBRE.
  
  const float sync_amount = parameters.timbre * parameters.timbre;
  const float auxiliary_detune = ComputeDetuning(parameters.harmonics);
  const float primary_f = NoteToFrequency(parameters.note);
  const float auxiliary_f = NoteToFrequency(parameters.note + auxiliary_detune);
  const float primary_sync_f = NoteToFrequency(
      parameters.note + sync_amount * 48.0f);
  const float auxiliary_sync_f = NoteToFrequency(
      parameters.note + auxiliary_detune + sync_amount * 48.0f);

  float shape = parameters.morph * 1.5f;
  CONSTRAIN(shape, 0.0f, 1.0f);

  float pw = 0.5f + (parameters.morph - 0.66f) * 1.46f;
  CONSTRAIN(pw, 0.5f, 0.995f);
  
  // Render monster sync to AUX.
  primary_.Render<true>(primary_f, primary_sync_f, pw, shape, out, size);
  auxiliary_.Render<true>(auxiliary_f, auxiliary_sync_f, pw, shape, aux, size);
  for (size_t i = 0; i < size; ++i) {
    aux[i] = (aux[i] - out[i]) * 0.5f;
  }
  
  // Render double varishape to OUT.
  float square_pw = 1.3f * parameters.timbre - 0.15f;
  CONSTRAIN(square_pw, 0.005f, 0.5f);
  
  const float square_sync_ratio = parameters.timbre < 0.5f
      ? 0.0f
      : (parameters.timbre - 0.5f) * (parameters.timbre - 0.5f) * 4.0f * 48.0f;
  
  const float square_gain = min(parameters.timbre * 8.0f, 1.0f);
  
  float saw_pw = parameters.morph < 0.5f
      ? parameters.morph + 0.5f
      : 1.0f - (parameters.morph - 0.5f) * 2.0f;
  saw_pw *= 1.1f;
  CONSTRAIN(saw_pw, 0.005f, 1.0f);
    
  float saw_shape = 10.0f - 21.0f * parameters.morph;
  CONSTRAIN(saw_shape, 0.0f, 1.0f);
  
  float saw_gain = 8.0f * (1.0f - parameters.morph);
  CONSTRAIN(saw_gain, 0.02f, 1.0f);
  
  const float square_sync_f = NoteToFrequency(
      parameters.note + square_sync_ratio);
  
  sync_.Render<true>(
      primary_f, square_sync_f, square_pw, 1.0f, temp_buffer_, size);
  variable_saw_.Render(auxiliary_f, saw_pw, saw_shape, out, size);
  
  float norm = 1.0f / (std::max(square_gain, saw_gain));
  
  ParameterInterpolator square_gain_modulation(
      &auxiliary_amount_,
      square_gain * 0.3f * norm,
      size);

  ParameterInterpolator saw_gain_modulation(
      &xmod_amount_,
      saw_gain * 0.5f * norm,
      size);
  
  for (size_t i = 0; i < size; ++i) {
    out[i] = out[i] * saw_gain_modulation.Next() + \
        square_gain_modulation.Next() * temp_buffer_[i];
  }

#endif  // VA_VARIANT values

}

}  // namespace plaits
