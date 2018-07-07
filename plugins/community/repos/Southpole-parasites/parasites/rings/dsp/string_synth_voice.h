// Copyright 2015 Olivier Gillet.
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
// Voice for the string synth easter egg.

#ifndef RINGS_DSP_STRING_SYNTH_VOICE_H_
#define RINGS_DSP_STRING_SYNTH_VOICE_H_

#include "stmlib/stmlib.h"

#include "rings/dsp/string_synth_oscillator.h"

namespace rings {

template<size_t num_harmonics>
class StringSynthVoice {
 public:
  StringSynthVoice() { }
  ~StringSynthVoice() { }
  
  void Init() {
    for (size_t i = 0; i < num_harmonics; ++i) {
      oscillator_[i].Init();
    }
  }
  
  void Render(
      float frequency,
      const float* amplitudes,
      size_t summed_harmonics,
      float* out,
      size_t size) {
    oscillator_[0].template Render<OSCILLATOR_SHAPE_DARK_SQUARE, true>(
        frequency, amplitudes[0], amplitudes[1], out, size);
    amplitudes += 2;
    
    for (size_t i = 1; i < summed_harmonics; ++i) {
      frequency *= 2.0f;
      oscillator_[i].template Render<OSCILLATOR_SHAPE_BRIGHT_SQUARE, false>(
          frequency, amplitudes[0], amplitudes[1], out, size);
      amplitudes += 2;
    }
  }

 private:
  StringSynthOscillator oscillator_[num_harmonics];
  DISALLOW_COPY_AND_ASSIGN(StringSynthVoice);
};

}  // namespace rings

#endif  // RINGS_DSP_STRING_SYNTH_VOICE_H_
