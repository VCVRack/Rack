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
// 808-style HH with two noise sources - one faithful to the original, the other
// more metallic.

#include "plaits/dsp/engine/hi_hat_engine.h"

namespace plaits {

using namespace stmlib;

void HiHatEngine::Init(BufferAllocator* allocator) {
  hi_hat_1_.Init();
  hi_hat_2_.Init();
  temp_buffer_[0] = allocator->Allocate<float>(kMaxBlockSize);
  temp_buffer_[1] = allocator->Allocate<float>(kMaxBlockSize);
}

void HiHatEngine::Reset() {
  
}

void HiHatEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  const float f0 = NoteToFrequency(parameters.note);
  
  hi_hat_1_.Render(
      parameters.trigger & TRIGGER_UNPATCHED,
      parameters.trigger & TRIGGER_RISING_EDGE,
      parameters.accent,
      f0,
      parameters.timbre,
      parameters.morph,
      parameters.harmonics,
      temp_buffer_[0],
      temp_buffer_[1],
      out,
      size);
  
  hi_hat_2_.Render(
      parameters.trigger & TRIGGER_UNPATCHED,
      parameters.trigger & TRIGGER_RISING_EDGE,
      parameters.accent,
      f0,
      parameters.timbre,
      parameters.morph,
      parameters.harmonics,
      temp_buffer_[0],
      temp_buffer_[1],
      aux,
      size);
}

}  // namespace plaits
