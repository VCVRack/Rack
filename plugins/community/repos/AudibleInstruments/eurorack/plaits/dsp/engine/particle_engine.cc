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
// Clocked noise processed by a filter.

#include "plaits/dsp/engine/particle_engine.h"

#include <algorithm>

namespace plaits {

using namespace std;
using namespace stmlib;

void ParticleEngine::Init(BufferAllocator* allocator) {
  for (int i = 0; i < kNumParticles; ++i) {
    particle_[i].Init();
  }
  diffuser_.Init(allocator->Allocate<uint16_t>(8192));
  post_filter_.Init();
}

void ParticleEngine::Reset() {
  diffuser_.Clear();
}

void ParticleEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  const float f0 = NoteToFrequency(parameters.note);
  const float density_sqrt = NoteToFrequency(
      60.0f + parameters.timbre * parameters.timbre * 72.0f);
  const float density = density_sqrt * density_sqrt * (1.0f / kNumParticles);
  const float gain = 1.0f / density;
  const float q_sqrt = SemitonesToRatio(parameters.morph >= 0.5f
      ? (parameters.morph - 0.5f) * 120.0f
      : 0.0f);
  const float q = 0.5f + q_sqrt * q_sqrt;
  const float spread = 48.0f * parameters.harmonics * parameters.harmonics;
  const float raw_diffusion_sqrt = 2.0f * fabsf(parameters.morph - 0.5f);
  const float raw_diffusion = raw_diffusion_sqrt * raw_diffusion_sqrt;
  const float diffusion = parameters.morph < 0.5f
      ? raw_diffusion
      : 0.0f;
  const bool sync = parameters.trigger & TRIGGER_RISING_EDGE;
  
  fill(&out[0], &out[size], 0.0f);
  fill(&aux[0], &aux[size], 0.0f);
  
  for (int i = 0; i < kNumParticles; ++i) {
    particle_[i].Render(
        sync,
        density,
        gain,
        f0,
        spread,
        q,
        out,
        aux,
        size);
  }
  
  post_filter_.set_f_q<FREQUENCY_DIRTY>(min(f0, 0.49f), 0.5f);
  post_filter_.Process<FILTER_MODE_LOW_PASS>(out, out, size);
  
  diffuser_.Process(
      0.8f * diffusion * diffusion,
      0.5f * diffusion + 0.25f,
      out,
      size);
}

}  // namespace plaits
