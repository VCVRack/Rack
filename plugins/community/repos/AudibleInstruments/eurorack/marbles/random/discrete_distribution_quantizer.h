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
// Quantize voltages by sampling from a discrete distribution.

#ifndef MARBLES_RANDOM_DISCRETE_DISTRIBUTION_QUANTIZER_H_
#define MARBLES_RANDOM_DISCRETE_DISTRIBUTION_QUANTIZER_H_

#include "stmlib/stmlib.h"

#include "marbles/random/distributions.h"
#include "marbles/random/quantizer.h"

namespace marbles {

class DiscreteDistributionQuantizer {
 public:
  typedef DiscreteDistribution<kMaxDegrees> Distribution;
  
  struct Cell {
    float center;
    float width;
    float weight;
  
    inline float scaled_width(float amount) {
      float w = 8.0f * (weight - amount) + 0.5f;
      CONSTRAIN(w, 0.0f, 1.0f);
      return w * width;
    }
  };
  
  DiscreteDistributionQuantizer() { }
  ~DiscreteDistributionQuantizer() { }

  void Init(const Scale& scale);
  
  float Process(float value, float amount);

 private:
  float base_interval_;
  float base_interval_reciprocal_;
  
  int num_cells_;
  Cell cells_[kMaxDegrees + 1];
  Distribution distribution_;
  
  DISALLOW_COPY_AND_ASSIGN(DiscreteDistributionQuantizer);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_DISCRETE_DISTRIBUTION_QUANTIZER_H_
