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
// Variable resolution quantizer.

#include "marbles/random/quantizer.h"

#include "stmlib/dsp/dsp.h"

#include <cmath>
#include <algorithm>

namespace marbles {

using namespace std;

void Quantizer::Init(const Scale& scale) {
  int n = scale.num_degrees;

  // We don't want garbage scale data here...
  if (!n || n > kMaxDegrees || scale.base_interval == 0.0f) {
    return;
  }

  num_degrees_ = n;
  base_interval_ = scale.base_interval;
  base_interval_reciprocal_ = 1.0f / scale.base_interval;
  
  uint8_t second_largest_threshold = 0;
  for (int i = 0; i < n; ++i) {
    voltage_[i] = scale.degree[i].voltage;
    if (scale.degree[i].weight != 255 && \
        scale.degree[i].weight >= second_largest_threshold) {
      second_largest_threshold = scale.degree[i].weight;
    }
  }
  
  uint8_t thresholds_[kNumThresholds] = {
    0, 16, 32, 64, 128, 192, 255
  };
  
  if (second_largest_threshold > 192) {
    // Be more selective to only include the notes at rank 1 and 2 at
    // the last but one position.
    thresholds_[kNumThresholds - 2] = second_largest_threshold;
  }
  
  for (int t = 0; t < kNumThresholds; ++t) {
    uint16_t bitmask = 0;
    uint8_t first = 0xff;
    uint8_t last = 0;
    for (int i = 0; i < n; ++i) {
      if (scale.degree[i].weight >= thresholds_[t]) {
        bitmask |= 1 << i;
        if (first == 0xff) first = i;
        last = i;
      }
    }
    level_[t].bitmask = bitmask;
    level_[t].first = first;
    level_[t].last = last;
  }
  
  level_quantizer_.Init();
  fill(&feedback_[0], &feedback_[kNumThresholds], 0.0f);
}

float Quantizer::Process(float value, float amount, bool hysteresis) {
  int level = level_quantizer_.Process(amount, kNumThresholds + 1);
  float quantized_voltage = value;

  if (level > 0) {
    level -= 1;
    float raw_value = value;
    if (hysteresis) {
      value += feedback_[level];
    }

    const float note = value * base_interval_reciprocal_;
    MAKE_INTEGRAL_FRACTIONAL(note);
    if (value < 0.0f) {
      note_integral -= 1;
      note_fractional += 1.0f;
    }
    note_fractional *= base_interval_;
    
    // Search for the tightest upper/lower bound in the set of available
    // voltages. stl::upper_bound / stl::lower_bound wouldn't work here
    // because some entries are masked.
    Level l = level_[level];
    float a = voltage_[l.last] - base_interval_;
    float b = voltage_[l.first] + base_interval_;

    uint16_t bitmask = l.bitmask;
    for (int i = 0; i < num_degrees_; ++i) {
      if (bitmask & 1) {
        float v = voltage_[i];
        if (note_fractional > v) {
          a = v;
        } else {
          b = v;
          break;
        }
      }
      bitmask >>= 1;
    }
    
    quantized_voltage = note_fractional < (a + b) * 0.5f ? a : b;
    quantized_voltage += static_cast<float>(note_integral) * base_interval_;
    feedback_[level] = (quantized_voltage - raw_value) * 0.25f;
  }
  return quantized_voltage;
}

}  // namespace marbles
