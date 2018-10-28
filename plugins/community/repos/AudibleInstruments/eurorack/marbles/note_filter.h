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
// Low pass filter for getting stable pitch data.

#ifndef MARBLES_NOTE_FILTER_H_
#define MARBLES_NOTE_FILTER_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/delay_line.h"

namespace marbles {

// Note: this is a "slow" filter, since it takes about 10 samples to catch
// up with an edge in the input signal.
class NoteFilter {
 public:
  enum {
    N = 7  // Median filter order
  };
  NoteFilter() { }
  ~NoteFilter() { }

  void Init() {
    lp_1_ = 0.0f;
    lp_2_ = 0.0f;
    std::fill(&previous_values_[0], &previous_values_[N], 0.0f);
  }

  inline float Process(float value) {
    float sorted_values[N];

    std::rotate(
        &previous_values_[0],
        &previous_values_[1],
        &previous_values_[N]);
    previous_values_[N - 1] = value;
    
    std::copy(&previous_values_[0], &previous_values_[N], &sorted_values[0]);
    std::sort(&sorted_values[0], &sorted_values[N]);
    value = sorted_values[(N - 1) / 2];
    
    const float kLPCoefficient = 0.65f;
    ONE_POLE(lp_1_, value, kLPCoefficient);
    ONE_POLE(lp_2_, lp_1_, kLPCoefficient);
    return lp_2_;
  }

 private:
  float previous_values_[N];
  float lp_1_;
  float lp_2_;
  
  DISALLOW_COPY_AND_ASSIGN(NoteFilter);
};

}  // namespace marbles

#endif  // MARBLES_NOTE_FILTER_H_
