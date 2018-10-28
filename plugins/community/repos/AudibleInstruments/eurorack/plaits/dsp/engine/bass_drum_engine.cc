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
// 808 and synthetic bass drum generators.

#include "plaits/dsp/engine/bass_drum_engine.h"

#include <algorithm>

namespace plaits {

using namespace std;
using namespace stmlib;

void BassDrumEngine::Init(BufferAllocator* allocator) {
  analog_bass_drum_.Init();
  synthetic_bass_drum_.Init();
  overdrive_.Init();
}

void BassDrumEngine::Reset() {
  
}

void BassDrumEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  const float f0 = NoteToFrequency(parameters.note);
  
  const float attack_fm_amount = min(parameters.harmonics * 4.0f, 1.0f);
  const float self_fm_amount = max(min(parameters.harmonics * 4.0f - 1.0f, 1.0f), 0.0f);
  const float drive = max(parameters.harmonics * 2.0f - 1.0f, 0.0f) * \
      max(1.0f - 16.0f * f0, 0.0f);
  
  const bool sustain = parameters.trigger & TRIGGER_UNPATCHED;
  
  analog_bass_drum_.Render(
      sustain,
      parameters.trigger & TRIGGER_RISING_EDGE,
      parameters.accent,
      f0,
      parameters.timbre,
      parameters.morph,
      attack_fm_amount,
      self_fm_amount,
      out,
      size);

  overdrive_.Process(
      0.5f + 0.5f * drive,
      out,
      size);

  synthetic_bass_drum_.Render(
      sustain,
      parameters.trigger & TRIGGER_RISING_EDGE,
      parameters.accent,
      f0,
      parameters.timbre,
      parameters.morph,
      sustain
          ? parameters.harmonics
          : 0.4f - 0.25f * parameters.morph * parameters.morph,
      min(parameters.harmonics * 2.0f, 1.0f),
      max(parameters.harmonics * 2.0f - 1.0f, 0.0f),
      aux,
      size);
}

}  // namespace plaits
