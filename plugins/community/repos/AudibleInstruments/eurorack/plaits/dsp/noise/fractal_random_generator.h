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
// Stack octaves of a noise source to generate a fractal noise.

#ifndef PLAITS_DSP_NOISE_FRACTAL_RANDOM_GENERATOR_H_
#define PLAITS_DSP_NOISE_FRACTAL_RANDOM_GENERATOR_H_

#include "stmlib/stmlib.h"

namespace plaits {

template<typename T, int order>
class FractalRandomGenerator {
 public:
  FractalRandomGenerator() { }
  ~FractalRandomGenerator() { }
  
  void Init() {
    for (int i = 0; i < order; ++i) {
      generator_[i].Init();
    }
  }
  
  float Render(float frequency) {
    return Render(frequency, 0.5f);
  }
  
  float Render(float frequency, float decay) {
    float gain = 0.5f;
    float sum = 0.0f;

    for (int i = 0; i < order; ++i) {
      sum += generator_[i].Render(frequency) * gain;
      gain *= decay;
      frequency *= 2.0f;
    }

    return sum;
  }
  
 private:
  T generator_[order];
  
  DISALLOW_COPY_AND_ASSIGN(FractalRandomGenerator);
};

}  // namespace plaits

#endif  // PLAITS_DSP_NOISE_FRACTAL_RANDOM_GENERATOR_H_
