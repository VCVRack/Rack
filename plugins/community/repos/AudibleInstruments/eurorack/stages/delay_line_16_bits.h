// Copyright 2017 Olivier Gillet.
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
// Delay line, like the one in stmlib/dsp, but for int16_t, with
// built-in quantization and linear interpolation guard.
// Too specialized to be in a library...

#ifndef STAGES_DELAY_LINE_16_BITS_H_
#define STAGES_DELAY_LINE_16_BITS_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"

#include <algorithm>

namespace stages {

template<size_t max_delay>
class DelayLine16Bits {
 public:
  DelayLine16Bits() { }
  ~DelayLine16Bits() { }
  
  void Init() {
    Reset();
  }

  void Reset() {
    std::fill(&line_[0], &line_[max_delay], 0);
    write_ptr_ = 0;
  }
  
  inline void Write(const float sample) {
    int32_t word = static_cast<int32_t>(sample * 32768.0f);
    CONSTRAIN(word, -32768, 32767);
    line_[write_ptr_] = word;
    if (write_ptr_ == 0) {
      line_[max_delay] = word;
      write_ptr_ = max_delay - 1;
    } else {
      --write_ptr_;
    }
  }
  
  inline const float Read(float delay) const {
    MAKE_INTEGRAL_FRACTIONAL(delay)
    size_t read_ptr = (write_ptr_ + delay_integral) % max_delay;
    float a = static_cast<float>(line_[read_ptr]) / 32768.0f;
    float b = static_cast<float>(line_[read_ptr + 1]) / 32768.0f;
    return a + (b - a) * delay_fractional;
  }

 private:
  size_t write_ptr_;
  int16_t line_[max_delay + 1];
  
  DISALLOW_COPY_AND_ASSIGN(DelayLine16Bits);
};

}  // namespace stages

#endif  // STAGES_DELAY_LINE_16_BITS_H_
