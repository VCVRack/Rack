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

#ifndef PLAITS_DSP_ENGINE_HI_HAT_ENGINE_H_
#define PLAITS_DSP_ENGINE_HI_HAT_ENGINE_H_

#include "plaits/dsp/drums/hi_hat.h"
#include "plaits/dsp/engine/engine.h"

namespace plaits {
  
class HiHatEngine : public Engine {
 public:
  HiHatEngine() { }
  ~HiHatEngine() { }
  
  virtual void Init(stmlib::BufferAllocator* allocator);
  virtual void Reset();
  virtual void Render(const EngineParameters& parameters,
      float* out,
      float* aux,
      size_t size,
      bool* already_enveloped);

 private:
  HiHat<SquareNoise, SwingVCA, true> hi_hat_1_;
  HiHat<RingModNoise, LinearVCA, false> hi_hat_2_;
  
  float* temp_buffer_[2];
  
  DISALLOW_COPY_AND_ASSIGN(HiHatEngine);
};

}  // namespace plaits

#endif  // PLAITS_DSP_ENGINE_HI_HAT_ENGINE_H_