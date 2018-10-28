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
// Windowed sine segments.

#include <algorithm>

#include "plaits/dsp/engine/grain_engine.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void GrainEngine::Init(BufferAllocator* allocator) {
  grainlet_[0].Init();
  grainlet_[1].Init();
  // vosim_oscillator_.Init();
  z_oscillator_.Init();
  dc_blocker_[0].Init();
  dc_blocker_[1].Init();
}

void GrainEngine::Reset() {
  
}

void GrainEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  const float root = parameters.note;
  const float f0 = NoteToFrequency(root);
  
  const float f1 = NoteToFrequency(24.0f + 84.0f * parameters.timbre);
  const float ratio = SemitonesToRatio(-24.0f + 48.0f * parameters.harmonics);
  const float carrier_bleed = parameters.harmonics < 0.5f
      ? 1.0f - 2.0f * parameters.harmonics
      : 0.0f;
  const float carrier_bleed_fixed = carrier_bleed * (2.0f - carrier_bleed);
  const float carrier_shape = 0.33f + (parameters.morph - 0.33f) * \
      max(1.0f - f0 * 24.0f, 0.0f);
  
  grainlet_[0].Render(f0, f1, carrier_shape, carrier_bleed_fixed, out, size);
  grainlet_[1].Render(f0, f1 * ratio, carrier_shape, carrier_bleed_fixed, aux, size);
  dc_blocker_[0].set_f<FREQUENCY_DIRTY>(0.3f * f0);
  for (size_t i = 0; i < size; ++i) {
    out[i] = dc_blocker_[0].Process<FILTER_MODE_HIGH_PASS>(out[i] + aux[i]);
  }

  const float cutoff = NoteToFrequency(root + 96.0f * parameters.timbre);
  z_oscillator_.Render(
      f0,
      cutoff,
      parameters.morph,
      parameters.harmonics,
      aux,
      size);
  
  dc_blocker_[1].set_f<FREQUENCY_DIRTY>(0.3f * f0);
  dc_blocker_[1].Process<FILTER_MODE_HIGH_PASS>(aux, size);
}

}  // namespace plaits
