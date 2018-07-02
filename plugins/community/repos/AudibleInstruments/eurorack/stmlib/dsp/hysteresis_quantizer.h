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
// Quantize a float in [0, 1] to an integer in [0, num_steps[. Apply hysteresis
// to prevent jumps near the decision boundary.

#ifndef STMLIB_DSP_HYSTERESIS_QUANTIZER_H_
#define STMLIB_DSP_HYSTERESIS_QUANTIZER_H_

#include "stmlib/stmlib.h"

namespace stmlib {

class HysteresisQuantizer {
 public:
  HysteresisQuantizer() { }
  ~HysteresisQuantizer() { }

  void Init() {
    quantized_value_ = 0;
  }

  inline int Process(float value, int num_steps) {
    return Process(value, num_steps, 0.25f);
  }

  inline int Process(float value, int num_steps, float hysteresis) {
    return Process(0, value, num_steps, hysteresis);
  }

  inline int Process(int base, float value, int num_steps, float hysteresis) {
    value *= static_cast<float>(num_steps - 1);
    value += static_cast<float>(base);
    float hysteresis_feedback = value > static_cast<float>(quantized_value_)
        ? -hysteresis
        : hysteresis;
    int q = static_cast<int>(value + hysteresis_feedback + 0.5f);
    CONSTRAIN(q, 0, num_steps - 1);
    quantized_value_ = q;
    return q;
  }

  template<typename T>
  const T& Lookup(const T* array, float value, int num_steps) {
    return array[Process(value, num_steps)];
  }

 private:
  int quantized_value_;
  
  DISALLOW_COPY_AND_ASSIGN(HysteresisQuantizer);
};

}  // namespace stmlib

#endif  // STMLIB_DSP_HYSTERESIS_QUANTIZER_H_
