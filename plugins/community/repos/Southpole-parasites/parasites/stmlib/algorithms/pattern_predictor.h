// Copyright 2013 Olivier Gillet.
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
// Pattern predictor for synchronization to drum patterns or clocks with swing.

#ifndef STMLIB_ALGORITHMS_PATTERN_PREDICTOR_H_
#define STMLIB_ALGORITHMS_PATTERN_PREDICTOR_H_

#include "stmlib/stmlib.h"

#include <algorithm>
#include <cstdlib>

namespace stmlib {
  
template<size_t history_size = 32, uint8_t max_candidate_period = 8>
class PatternPredictor {
 public: 
  PatternPredictor() { }

  void Init() {
    last_prediction_ = 0;
    history_pointer_ = 0;
    std::fill(&history_[0], &history_[history_size], 0);
  }
  
  uint32_t Predict(uint32_t value) {
    // Record the incoming value.
    history_[history_pointer_] = value;
    
    // Try various candidate periods
    uint8_t best_score = 0;
    uint8_t period = 0;
    for (uint8_t t = 1; t < max_candidate_period; ++t) {
      uint8_t score = 0;
      for (uint8_t k = 0; k < history_size; ++k) {
        uint32_t i = history_pointer_ + 2 * history_size - k;
        uint32_t j = i - t;
        i = i % history_size;
        j = j % history_size;
        uint32_t error = abs(static_cast<int32_t>(history_[i] - history_[j]));
        if (error < (history_[i] >> 4)) {
          ++score;
        }
      }
      if (score >= best_score) {
        best_score = score;
        period = t;
      }
    }
    history_pointer_ = (history_pointer_ + 1) % history_size;
    uint32_t new_prediction = \
        history_[(history_pointer_ - period + history_size) % history_size];
    
    uint32_t error = abs(static_cast<int32_t>(value - last_prediction_));
    bool prediction_was_good = error < (value >> 4);
    last_prediction_ = new_prediction;
    return prediction_was_good ? new_prediction : value;
  }

 private:
  uint32_t history_[history_size];
  uint32_t history_pointer_;
  uint32_t last_prediction_;

  DISALLOW_COPY_AND_ASSIGN(PatternPredictor);
};

}  // namespace stmlib

#endif  // STMLIB_ALGORITHMS_PATTERN_PREDICTOR_H_
