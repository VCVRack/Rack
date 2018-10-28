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

#ifndef MARBLES_RANDOM_QUANTIZER_H_
#define MARBLES_RANDOM_QUANTIZER_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/hysteresis_quantizer.h"

#include "marbles/random/distributions.h"
#include "marbles/random/quantizer.h"

namespace marbles {

const int kMaxDegrees = 16;
const int kNumThresholds = 7;

struct Degree {
  float voltage;
  uint8_t weight;
};

struct Scale {
  float base_interval;
  int num_degrees;
  Degree degree[kMaxDegrees];

  inline float cell_voltage(int i) const {
    float transposition = static_cast<float>(i / num_degrees) * base_interval;
    return degree[i % num_degrees].voltage + transposition;
  }
  
  void Init() {
    base_interval = 1.0f;
    num_degrees = 1;
    degree[0].voltage = 0.0f;
    degree[0].weight = 0.0f;
  }
  
  void InitMajor() {
    const uint8_t major_scale_weights[] = {
      255, 16, 128, 16, 192, 64, 8, 224, 16, 96, 32, 160,
    };

    base_interval = 1.0f;
    num_degrees = 12;
    for (size_t i = 0; i < 12; ++i) {
      degree[i].voltage = static_cast<float>(i) * 0.0833333333f;
      degree[i].weight = major_scale_weights[i];
    }
  }
  
  void InitTenth() {
    const uint8_t major_scale_weights[] = {
      255, 255, 255, 255, 255, 255, 255, 255, 255, 25
    };

    base_interval = 1.0f;
    num_degrees = 10;
    for (size_t i = 0; i < 10; ++i) {
      degree[i].voltage = static_cast<float>(i) * 0.1f;
      degree[i].weight = major_scale_weights[i];
    }
  }
};

class Quantizer {
 public:
  Quantizer() { }
  ~Quantizer() { }

  void Init(const Scale& scale);

  float Process(float value, float amount, bool hysteresis);
  
 private:
  struct Level {
    uint16_t bitmask;  // bitmask of active degrees.
    uint8_t first;  // index of the first active degree.
    uint8_t last;   // index of the last active degree.
  };
  float voltage_[kMaxDegrees];

  Level level_[kNumThresholds];
  float feedback_[kNumThresholds];
  
  float base_interval_;
  float base_interval_reciprocal_;
  int num_degrees_;
  stmlib::HysteresisQuantizer level_quantizer_;
  
  DISALLOW_COPY_AND_ASSIGN(Quantizer);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_QUANTIZER_H_
