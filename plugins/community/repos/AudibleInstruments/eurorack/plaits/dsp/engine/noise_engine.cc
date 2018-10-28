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
// Clocked noise processed by a multimode filter.

#include "plaits/dsp/engine/noise_engine.h"

#include "stmlib/dsp/parameter_interpolator.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void NoiseEngine::Init(BufferAllocator* allocator) {
  clocked_noise_[0].Init();
  clocked_noise_[1].Init();
  lp_hp_filter_.Init();
  bp_filter_[0].Init();
  bp_filter_[1].Init();

  previous_f0_ = 0.0f;
  previous_f1_ = 0.0f;
  previous_q_ = 0.0f;
  previous_mode_ = 0.0f;

  temp_buffer_ = allocator->Allocate<float>(kMaxBlockSize);
}

void NoiseEngine::Reset() {
  
}

void NoiseEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  const float f0 = NoteToFrequency(parameters.note);
  const float f1 = NoteToFrequency(
      parameters.note + parameters.harmonics * 48.0f - 24.0f);
  const float clock_lowest_note = parameters.trigger & TRIGGER_UNPATCHED
      ? 0.0f
      : -24.0f;
  const float clock_f = NoteToFrequency(
      parameters.timbre * (128.0f - clock_lowest_note) + clock_lowest_note);
  const float q = 0.5f * SemitonesToRatio(parameters.morph * 120.0f);
  const bool sync = parameters.trigger & TRIGGER_RISING_EDGE;
  clocked_noise_[0].Render(sync, clock_f, aux, size);
  clocked_noise_[1].Render(sync, clock_f * f1 / f0, temp_buffer_, size);
  
  ParameterInterpolator f0_modulation(&previous_f0_, f0, size);
  ParameterInterpolator f1_modulation(&previous_f1_, f1, size);
  ParameterInterpolator q_modulation(&previous_q_, q, size);
  ParameterInterpolator mode_modulation(
      &previous_mode_, parameters.harmonics, size);
  
  const float* in_1 = aux;
  const float* in_2 = temp_buffer_;
  while (size--) {
    const float f0 = f0_modulation.Next();
    const float f1 = f1_modulation.Next();
    const float q = q_modulation.Next();
    const float gain = 1.0f / Sqrt((0.5f + q) * 40.0f * f0);
    lp_hp_filter_.set_f_q<FREQUENCY_ACCURATE>(f0, q);
    bp_filter_[0].set_f_q<FREQUENCY_ACCURATE>(f0, q);
    bp_filter_[1].set_f_q<FREQUENCY_ACCURATE>(f1, q);
    
    float input_1 = *in_1++ * gain;
    float input_2 = *in_2++ * gain;
    lp_hp_filter_.ProcessMultimodeLPtoHP(
        &input_1, out++, 1, mode_modulation.Next());
    *aux++ = bp_filter_[0].Process<FILTER_MODE_BAND_PASS>(input_1) + \
        bp_filter_[1].Process<FILTER_MODE_BAND_PASS>(input_2);
  }
}

}  // namespace plaits
