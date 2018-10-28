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

#ifndef PLAITS_DSP_PHYSICAL_MODELLING_MODAL_VOICE_H_
#define PLAITS_DSP_PHYSICAL_MODELLING_MODAL_VOICE_H_

#include "plaits/dsp/physical_modelling/resonator.h"

namespace plaits {

class ModalVoice {
 public:
  ModalVoice() { }
  ~ModalVoice() { }
  
  void Init();
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
  ResonatorSvf<1> excitation_filter_;
  Resonator resonator_;
  
  DISALLOW_COPY_AND_ASSIGN(ModalVoice);
};

}  // namespace plaits

#endif  // PLAITS_DSP_PHYSICAL_MODELLING_MODAL_VOICE_H_
