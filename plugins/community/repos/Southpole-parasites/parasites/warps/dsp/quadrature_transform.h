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
// Extracts, from an audio signal, two phase-shifted signals that are the
// Hilbert transform of each other.

#ifndef WARPS_DSP_QUADRATURE_TRANSFORM_H_
#define WARPS_DSP_QUADRATURE_TRANSFORM_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

namespace warps {

const int32_t kMaxNumFilters = 24;

class QuadratureTransform {
 public:
  QuadratureTransform() { }
  ~QuadratureTransform() { }
  
  void Init(const float* poles, int32_t num_filters) {
    num_filters_ = num_filters;
    for (int32_t i = 0; i < num_filters; ++i) {
      filter_[i].Init(-poles[i]);
    }
  }
  
  inline void Process(float in, float* i_out, float* q_out) {
    for (int32_t i = 0; i < num_filters_; ++i) {
      float* destination = i & 1 ? q_out : i_out;
      float source = i <= 1 ? in : *destination;
      *destination = filter_[i].Process(source);
    }
  }
  
  void Process(const float* in, float* i_out, float* q_out, size_t size) {
    for (int32_t i = 0; i < num_filters_; ++i) {
      float* destination = i & 1 ? q_out : i_out;
      const float* source = i <= 1 ? in : destination;
      filter_[i].Process(source, destination, size);
    }
  }
  
 private:
  class AllPassFilter {
   public:
    AllPassFilter() { }
    ~AllPassFilter() { }

    void Init(float coefficient) {
      x_ = 0.0f;
      y_ = 0.0f;
      coefficient_ = coefficient;
    }

    inline float Process(float in) {
      float y = coefficient_ * (in - y_) + x_;
      x_ = in;
      y_ = y;
      return y;
    }

    inline void Process(const float* in, float* out, size_t size) {
      const float coefficient = coefficient_;
      float xp = x_;
      float yp = y_;
      while (size--) {
        float x = *in++;
        float y = coefficient * (x - yp) + xp;
        *out++ = y;
        xp = x;
        yp = y;
      }
      x_ = xp;
      y_ = yp;
    }

   private:
    float x_;
    float y_;
    float coefficient_;

    DISALLOW_COPY_AND_ASSIGN(AllPassFilter);
  };

  AllPassFilter filter_[kMaxNumFilters];
  int32_t num_filters_;

  DISALLOW_COPY_AND_ASSIGN(QuadratureTransform);
};

}  // namespace warps

#endif  // WARPS_DSP_QUADRATURE_TRANSFORM_H_
