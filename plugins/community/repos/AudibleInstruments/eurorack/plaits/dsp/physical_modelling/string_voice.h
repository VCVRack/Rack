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
// Extended Karplus-Strong, with all the niceties from Rings.

#ifndef PLAITS_DSP_PHYSICAL_STRING_VOICE_H_
#define PLAITS_DSP_PHYSICAL_STRING_VOICE_H_

#include "stmlib/dsp/filter.h"
#include "stmlib/utils/buffer_allocator.h"

#include "plaits/dsp/physical_modelling/string.h"

namespace plaits {

class StringVoice {
 public:
  StringVoice() { }
  ~StringVoice() { }
  
  void Init(stmlib::BufferAllocator* allocator);
  void Reset();
  void Render(
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
      size_t size);
  
 private:
  stmlib::Svf excitation_filter_;
  String string_;
  size_t remaining_noise_samples_;
  
  DISALLOW_COPY_AND_ASSIGN(StringVoice);
};

}  // namespace plaits

#endif  // PLAITS_DSP_PHYSICAL_STRING_VOICE_H_
