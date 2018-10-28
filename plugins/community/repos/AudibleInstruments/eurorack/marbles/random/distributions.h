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
// Generates samples from various kinds of random distributions.

#ifndef MARBLES_RANDOM_DISTRIBUTIONS_H_
#define MARBLES_RANDOM_DISTRIBUTIONS_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"

#include "marbles/resources.h"

namespace marbles {
  
const size_t kNumBiasValues = 5;
const size_t kNumRangeValues = 9;
const float kIcdfTableSize = 128.0f;

// Generates samples from beta distribution, from uniformly distributed samples.
// For higher throughput, uses pre-computed tables of inverse cdfs.
inline float BetaDistributionSample(float uniform, float spread, float bias) {
  // Tables are pre-computed only for bias <= 0.5. For values above 0.5,
  // symmetry is used.
  bool flip_result = bias > 0.5f;
  if (flip_result) {
    uniform = 1.0f - uniform;
    bias = 1.0f - bias;
  }
  
  bias *= (static_cast<float>(kNumBiasValues) - 1.0f) * 2.0f;
  spread *= (static_cast<float>(kNumRangeValues) - 1.0f);
  
  MAKE_INTEGRAL_FRACTIONAL(bias);
  MAKE_INTEGRAL_FRACTIONAL(spread);
  
  size_t cell = bias_integral * (kNumRangeValues + 1) + spread_integral;
  
  // Lower 5% and 95% percentiles use a different table with higher resolution.
  size_t offset = 0;
  if (uniform <= 0.05f) {
    offset = kIcdfTableSize + 1;
    uniform *= 20.0f;
  } else if (uniform >= 0.95f) {
    offset = 2 * (kIcdfTableSize + 1);
    uniform = (uniform - 0.95f) * 20.0f;
  }
  
  float x1y1 = stmlib::Interpolate(
      distributions_table[cell] + offset,
      uniform,
      kIcdfTableSize);
  float x2y1 = stmlib::Interpolate(
      distributions_table[cell + 1] + offset,
      uniform,
      kIcdfTableSize);
  float x1y2 = stmlib::Interpolate(
      distributions_table[cell + kNumRangeValues + 1] + offset,
      uniform,
      kIcdfTableSize);
  float x2y2 = stmlib::Interpolate(
      distributions_table[cell + kNumRangeValues + 2] + offset,
      uniform,
      kIcdfTableSize);
      
  float y1 = x1y1 + (x2y1 - x1y1) * spread_fractional;
  float y2 = x1y2 + (x2y2 - x1y2) * spread_fractional;
  float y = y1 + (y2 - y1) * bias_fractional;
  
  if (flip_result) {
    y = 1.0f - y;
  }
  return y;
}

// Pre-computed beta(3, 3) with a fatter tail.
inline float FastBetaDistributionSample(float uniform) {
  return stmlib::Interpolate(dist_icdf_4_3, uniform, kIcdfTableSize);
}

// Draws samples from a discrete distribution. Used for the quantizer.
// Example:
// * 1 with probability 0.2
// * 20 with probability 0.7
// * 666 with probability 0.1
//
// DiscreteDistribution d;
// d.Init();
// d.AddToken(1, 0.2);
// d.AddToken(20, 0.7);
// d.AddToken(666, 0.1);
// d.NoMoreTokens();
// Result r = d.Sample(u);
// cout << r.token_id;
//
// Weights do not have to add to 1.0f - the class handles normalization.
//
template<size_t size>
class DiscreteDistribution {
 public:
  DiscreteDistribution() { }
  ~DiscreteDistribution() { }
  
  void Init() {
    sum_ = 0.0f;
    num_tokens_ = 1;
    
    cdf_[0] = 0.0f;
    token_ids_[0] = 0;
  }
  
  void AddToken(int token_id, float weight) {
    if (weight <= 0.0f) {
      return;
    }
    sum_ += weight;
    token_ids_[num_tokens_] = token_id;
    cdf_[num_tokens_] = sum_;
    ++num_tokens_;
  }
  
  void NoMoreTokens() {
    token_ids_[num_tokens_] = token_ids_[num_tokens_ - 1];
    cdf_[num_tokens_] = sum_ + 1.0f;
  }
  
  struct Result {
    int token_id;
    float fraction;
    float start;
    float width;
  };
  
  inline Result Sample(float u) const {
    Result r;
    u *= sum_;
    int n = std::upper_bound(&cdf_[1], &cdf_[num_tokens_ + 1], u) - &cdf_[0];
    float norm = 1.0f / sum_;
    r.token_id = token_ids_[n];
    r.width = (cdf_[n] - cdf_[n - 1]) * norm;
    r.start = (cdf_[n - 1]) * norm;
    r.fraction = (u - cdf_[n - 1]) / (cdf_[n] - cdf_[n - 1]);
    return r;
  }
  
  float sum_;
  float cdf_[size + 2];
  int token_ids_[size + 2];
  int num_tokens_;
  
  DISALLOW_COPY_AND_ASSIGN(DiscreteDistribution);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_DISTRIBUTIONS_H_
