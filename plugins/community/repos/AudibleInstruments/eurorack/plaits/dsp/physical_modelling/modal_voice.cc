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
// Simple modal synthesis voice with a mallet exciter:
// click -> LPF -> resonator.
// 
// The click is replaced by continuous white noise when the trigger input
// of the module is not patched.

#include "plaits/dsp/physical_modelling/modal_voice.h"

#include <algorithm>

#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "plaits/dsp/noise/dust.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void ModalVoice::Init() {
  excitation_filter_.Init();
  resonator_.Init(0.015f, kMaxNumModes);
}

void ModalVoice::Render(
    bool sustain,
    bool trigger,
    float accent,
    float f0,
    float structure,
    float brightness,
    float damping,
    float* temp,
    float* out,
    float* aux,
    size_t size) {
  const float density = brightness * brightness;
  
  brightness += 0.25f * accent * (1.0f - brightness);
  damping += 0.25f * accent * (1.0f - damping);
  
  const float range = sustain ? 36.0f : 60.0f;
  const float f = sustain ? 4.0f * f0 : 2.0f * f0;
  const float cutoff = min(
      f * SemitonesToRatio((brightness * (2.0f - brightness) - 0.5f) * range),
      0.499f);
  const float q = sustain ? 0.7f : 1.5f;
  
  // Synthesize excitation signal.
  if (sustain) {
    const float dust_f = 0.00005f + 0.99995f * density * density;
    for (size_t i = 0; i < size; ++i) {
      temp[i] = Dust(dust_f) * (4.0f - dust_f * 3.0f) * accent;
    }
  } else {
    fill(&temp[0], &temp[size], 0.0f);
    if (trigger) {
      const float attenuation = 1.0f - damping * 0.5f;
      const float amplitude = (0.12f + 0.08f * accent) * attenuation;
      temp[0] = amplitude * SemitonesToRatio(cutoff * cutoff * 24.0f) / cutoff;
    }
  }
  const float one = 1.0f;
  excitation_filter_.Process<FILTER_MODE_LOW_PASS, false>(
      &cutoff, &q, &one, temp, temp, size);
  for (size_t i = 0; i < size; ++i) {
    aux[i] += temp[i];
  }
  
  resonator_.Process(f0, structure, brightness, damping, temp, out, size);
}

}  // namespace plaits
