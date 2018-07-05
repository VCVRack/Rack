// Copyright 2014 Olivier Gillet.
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
// Sample rate converter

#ifndef CLOUDS_DSP_SAMPLE_RATE_CONVERTER_H_
#define CLOUDS_DSP_SAMPLE_RATE_CONVERTER_H_

#include "stmlib/stmlib.h"

#include "clouds/dsp/frame.h"

namespace clouds {

template<int32_t ratio, int32_t filter_size, const float* coefficients>
class SampleRateConverter {
 public:
  SampleRateConverter() { }
  ~SampleRateConverter() { }
 
  void Init() {
    for (int32_t i = 0; i < filter_size * 2; ++i) {
      history_[i].l = history_[i].r = 0.0f;
    }
    std::copy(&coefficients[0], &coefficients[filter_size], &coefficients_[0]);
    history_ptr_ = filter_size - 1;
  };

  void Process(const FloatFrame* in, FloatFrame* out, size_t input_size) {
    int32_t history_ptr = history_ptr_;
    FloatFrame* history = history_;
    const float scale = ratio < 0 ? 1.0f : float(ratio);
    while (input_size) {
      int32_t consumed = ratio < 0 ? -ratio : 1;
      for (int32_t i = 0; i < consumed; ++i) {
        history[history_ptr + filter_size] = history[history_ptr] = *in++;
        --input_size;
        --history_ptr;
        if (history_ptr < 0) {
          history_ptr += filter_size;
        }
      }
    
      int32_t produced = ratio > 0 ? ratio : 1;
      for (int32_t i = 0; i < produced; ++i) {
        float y_l = 0.0f;
        float y_r = 0.0f;
        const FloatFrame* x = &history[history_ptr + 1];
        for (int32_t j = i; j < filter_size; j += produced) {
          const float h = coefficients_[j];
          y_l += x->l * h;
          y_r += x->r * h;
          ++x;
        }
        out->l = y_l * scale;
        out->r = y_r * scale;
        ++out;
      }
    }
    history_ptr_ = history_ptr;
  }
 
 private:
  float coefficients_[filter_size];
  FloatFrame history_[filter_size * 2];
  int32_t history_ptr_;

  DISALLOW_COPY_AND_ASSIGN(SampleRateConverter);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_SAMPLE_RATE_CONVERTER_H_
