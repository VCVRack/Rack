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
  
template<size_t history_size = 16, uint8_t max_candidate_period = 8>
class PatternPredictor {
 public: 
  PatternPredictor() { }

  void Init() {
    history_pointer_ = 0;
    std::fill(&history_[0], &history_[history_size], 0);
    std::fill(
        &prediction_error_[0],
        &prediction_error_[max_candidate_period + 1],
        0);
    std::fill(
        &predicted_period_[0],
        &predicted_period_[max_candidate_period + 1],
        0);
  }
  
  uint32_t Predict(int32_t value) {
    history_[history_pointer_] = value;
    int best_period = 0;
    
    for (int i = 0; i <= max_candidate_period; ++i) {
      int32_t error = abs(predicted_period_[i] - value);
      int32_t delta = error - prediction_error_[i];
      
      // Compute LP-ed prediction error.
      if (delta > 0) {
        prediction_error_[i] += delta >> 1;
      } else {
        prediction_error_[i] += delta >> 3;
      }
      
      if (i == 0) {
        predicted_period_[i] = (value + predicted_period_[i]) >> 1;
      } else {
        uint32_t t = history_pointer_ + 1 + history_size - i;
        predicted_period_[i] = history_[t % history_size];
      }
      
      if (prediction_error_[i] < prediction_error_[best_period]) {
        best_period = i;
      }
    }

    history_pointer_ = (history_pointer_ + 1) % history_size;
    return static_cast<uint32_t>(predicted_period_[best_period]);
  }


 private:
  uint32_t history_[history_size];
  int32_t prediction_error_[max_candidate_period + 1];
  int32_t predicted_period_[max_candidate_period + 1];
  uint32_t history_pointer_;

  DISALLOW_COPY_AND_ASSIGN(PatternPredictor);
};

}  // namespace stmlib

#endif  // STMLIB_ALGORITHMS_PATTERN_PREDICTOR_H_
