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
// Three voices of string synthesis.

#include "plaits/dsp/engine/string_engine.h"

#include <algorithm>

namespace plaits {

using namespace std;
using namespace stmlib;

void StringEngine::Init(BufferAllocator* allocator) {
  temp_buffer_ = allocator->Allocate<float>(kMaxBlockSize);
  for (int i = 0; i < kNumStrings; ++i) {
    voice_[i].Init(allocator);
    f0_[i] = 0.01f;
  }
  active_string_ = kNumStrings - 1;
  f0_delay_.Init(allocator->Allocate<float>(16));
}

void StringEngine::Reset() {
  f0_delay_.Reset();
  for (int i = 0; i < kNumStrings; ++i) {
    voice_[i].Reset();
  }
}

void StringEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  if (parameters.trigger & TRIGGER_RISING_EDGE) {
    // 8 in original firmware version.
    // 05.01.18: mic.w: problem with microbrute.
    f0_[active_string_] = f0_delay_.Read(14);
    active_string_ = (active_string_ + 1) % kNumStrings;
  }
  
  const float f0 = NoteToFrequency(parameters.note);
  f0_[active_string_] = f0;
  f0_delay_.Write(f0);
  
  fill(&out[0], &out[size], 0.0f);
  fill(&aux[0], &aux[size], 0.0f);
  
  for (int i = 0; i < kNumStrings; ++i) {
    voice_[i].Render(
        parameters.trigger & TRIGGER_UNPATCHED && i == active_string_,
        parameters.trigger & TRIGGER_RISING_EDGE && i == active_string_,
        parameters.accent,
        f0_[i],
        parameters.harmonics,
        parameters.timbre * parameters.timbre,
        parameters.morph,
        temp_buffer_,
        out,
        aux,
        size);
  }
}

}  // namespace plaits
