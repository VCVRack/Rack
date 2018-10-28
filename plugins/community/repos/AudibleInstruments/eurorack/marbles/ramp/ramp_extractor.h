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
// Recovers a ramp from a clock input by guessing at what time the next edge
// will occur. Prediction strategies:
// - Moving average of previous intervals.
// - Trigram model on quantized intervals.
// - Periodic rhythmic pattern.
// - Assume that the pulse width is constant, deduct the period from the on time
//   and the pulse width.
// 
// All prediction strategies are concurrently tested, and the output from the
// best performing one is selected (à la early Scheirer/Goto beat trackers).

#ifndef MARBLES_RAMP_RAMP_EXTRACTOR_H_
#define MARBLES_RAMP_RAMP_EXTRACTOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/gate_flags.h"

#include "marbles/ramp/ramp_divider.h"

namespace marbles {

class RampExtractor {
 public:
  RampExtractor() { }
  ~RampExtractor() { }
  
  void Init(float max_frequency);
  void Process(
      Ratio r,
      bool always_ramp_to_maximum,
      const stmlib::GateFlags* gate_flags,
      float* ramp,
      size_t size);
  void Reset();
  
 private:
  struct Pulse {
    uint32_t on_duration;
    uint32_t total_duration;
    uint32_t bucket;  // 4xlog2(total_duration).
    float pulse_width;
  };
  
  struct Prediction {
    float period;
    float accuracy;
  };
  
  enum Predictor {
    PREDICTOR_SLOW_MOVING_AVERAGE,
    PREDICTOR_FAST_MOVING_AVERAGE,
    PREDICTOR_HASH,
    PREDICTOR_PERIOD_1,
    PREDICTOR_PERIOD_2,
    PREDICTOR_PERIOD_3,
    PREDICTOR_PERIOD_4,
    PREDICTOR_PERIOD_5,
    PREDICTOR_PERIOD_6,
    PREDICTOR_PERIOD_7,
    PREDICTOR_PERIOD_8,
    PREDICTOR_PERIOD_9,
    PREDICTOR_PERIOD_10,
    PREDICTOR_LAST
  };
  
  static const size_t kHistorySize = 16;
  static const size_t kHashTableSize = 256;
  
  float ComputeAveragePulseWidth(float tolerance) const;
  
  Prediction PredictNextPeriod();

  size_t current_pulse_;
  Pulse history_[kHistorySize];
  float next_bucket_;
  
  float prediction_hash_table_[kHashTableSize];
  float predicted_period_[PREDICTOR_LAST];
  float prediction_accuracy_[PREDICTOR_LAST];
  float average_pulse_width_;
  
  float train_phase_;
  float frequency_;
  float max_output_phase_;
  float max_train_phase_;
  float reset_frequency_;
  float target_frequency_;
  float lp_coefficient_;
  
  float f_ratio_;
  float next_f_ratio_;
  float next_max_train_phase_;
  int reset_counter_;
  uint32_t reset_interval_;
  bool audio_rate_;
  
  float max_frequency_;
  float audio_rate_period_;
  float audio_rate_period_hysteresis_;
  
  DISALLOW_COPY_AND_ASSIGN(RampExtractor);
};

}  // namespace marbles

#endif  // MARBLES_RAMP_RAMP_EXTRACTOR_H_
