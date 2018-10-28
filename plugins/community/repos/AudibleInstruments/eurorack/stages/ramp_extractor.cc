// Copyright 2017 Olivier Gillet.
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
// - Periodic rhythmic pattern.
// - Assume that the pulse width is constant, deduct the period from the on time
//   and the pulse width.
// 
// All prediction strategies are concurrently tested, and the output from the
// best performing one is selected (à la early Scheirer/Goto beat trackers).

#include "stages/ramp_extractor.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"

namespace stages {

using namespace std;
using namespace stmlib;

const float kPulseWidthTolerance = 0.05f;

inline bool IsWithinTolerance(float x, float y, float error) {
  return x >= y * (1.0f - error) && x <= y * (1.0f + error);
}

void RampExtractor::Init(float sample_rate, float max_frequency) {
  max_frequency_ = max_frequency;
  min_period_ = 1.0f / max_frequency_;
  min_period_hysteresis_ = min_period_;
  reset_interval_ = 5.0f * sample_rate;
  
  train_phase_ = 0.0f;
  frequency_ = 0.0f;
  max_ramp_value_ = 1.0f;
  f_ratio_ = 1.0f;
  reset_counter_ = 1;

  Pulse p;
  p.on_duration = uint32_t(sample_rate * 0.25f);
  p.total_duration = uint32_t(sample_rate * 0.5f);
  p.pulse_width = 0.5f;

  fill(&history_[0], &history_[kHistorySize], p);
  current_pulse_ = 0;
  history_[current_pulse_].on_duration = 0;
  history_[current_pulse_].total_duration = 0;
  
  average_pulse_width_ = 0.0f;
  fill(&prediction_error_[0], &prediction_error_[kMaxPatternPeriod + 1], 50.0f);
  fill(&predicted_period_[0], &predicted_period_[kMaxPatternPeriod + 1],
       sample_rate * 0.5f);
  prediction_error_[0] = 0.0f;
}

float RampExtractor::ComputeAveragePulseWidth(float tolerance) const {
  float sum = 0.0f;
  for (size_t i = 0; i < kHistorySize; ++i) {
    if (!IsWithinTolerance(history_[i].pulse_width,
                           history_[current_pulse_].pulse_width,
                           tolerance)) {
      return 0.0f;
    }
    sum += history_[i].pulse_width;
  }
  return sum / static_cast<float>(kHistorySize);
}

float RampExtractor::PredictNextPeriod() {
  float last_period = static_cast<float>(history_[current_pulse_].total_duration);
  
  int best_pattern_period = 0;
  for (int i = 0; i <= kMaxPatternPeriod; ++i) {
    float error = predicted_period_[i] - last_period;
    float error_sq = error * error;
    SLOPE(prediction_error_[i], error_sq, 0.7f, 0.2f);

    if (i == 0) {
      ONE_POLE(predicted_period_[0], last_period, 0.5f);
    } else {
      size_t t = current_pulse_ + 1 + kHistorySize - i;
      predicted_period_[i] = history_[t % kHistorySize].total_duration;
    }
    
    if (prediction_error_[i] < prediction_error_[best_pattern_period]) {
      best_pattern_period = i;
    }
  }
  return predicted_period_[best_pattern_period];
}

void RampExtractor::Process(
    Ratio ratio, 
    const GateFlags* gate_flags,
    float* ramp, 
    size_t size) {
  float frequency = frequency_;
  float train_phase = train_phase_;
  float max_train_phase = max_train_phase_;
  
  while (size--) {
    GateFlags flags = *gate_flags++;

    // We are done with the previous pulse.
    if (flags & GATE_FLAG_RISING) {
      Pulse& p = history_[current_pulse_];
      const bool record_pulse = p.total_duration < reset_interval_;
      
      if (!record_pulse) {
        train_phase = 0.0f;
        reset_counter_ = ratio.q;
        f_ratio_ = ratio.ratio;
        max_train_phase = static_cast<float>(ratio.q);
        frequency = 1.0f / PredictNextPeriod();
      } else {
        if (float(p.total_duration) <= min_period_hysteresis_) {
          min_period_hysteresis_ = min_period_ * 1.05f;
          frequency = 1.0f / (p.total_duration);
          average_pulse_width_ = 0.0f;
        } else {
          // Compute the pulse width of the previous pulse, and check if the
          // PW has been consistent over the past pulses.
          min_period_hysteresis_ = min_period_;
          p.pulse_width = static_cast<float>(p.on_duration) / \
              static_cast<float>(p.total_duration);
          average_pulse_width_ = ComputeAveragePulseWidth(kPulseWidthTolerance);
          if (p.on_duration < 32) {
            average_pulse_width_ = 0.0f;
          }
          frequency = 1.0f / PredictNextPeriod();
        }

        // Reset the phase if necessary, according to the divider ratio.
        --reset_counter_;
        if (!reset_counter_) {
          train_phase = 0.0f;
          reset_counter_ = ratio.q;
          f_ratio_ = ratio.ratio;
          max_train_phase = static_cast<float>(ratio.q);
        } else {
          float expected = max_train_phase - static_cast<float>(reset_counter_);
          float warp =  expected - train_phase + 1.0f;
          frequency *= max(warp, 0.01f);
        }
        current_pulse_ = (current_pulse_ + 1) % kHistorySize;
      }
      history_[current_pulse_].on_duration = 0;
      history_[current_pulse_].total_duration = 0;
    }
    
    // Update history buffer with total duration and on duration.
    ++history_[current_pulse_].total_duration;
    if (flags & GATE_FLAG_HIGH) {
      ++history_[current_pulse_].on_duration;
    }
    
    if ((flags & GATE_FLAG_FALLING) &&
        average_pulse_width_ > 0.0f) {
      float t_on = static_cast<float>(history_[current_pulse_].on_duration);
      float next = max_train_phase - static_cast<float>(reset_counter_) + 1.0f;
      float pw = average_pulse_width_;
      frequency = max((next - train_phase), 0.0f) * pw / ((1.0f - pw) * t_on);
    }

    train_phase += frequency;
    if (train_phase >= max_train_phase) {
      train_phase = max_train_phase;
    }
    
    float phase = train_phase * f_ratio_;
    phase -= static_cast<float>(static_cast<int32_t>(phase));
    *ramp++ = phase;
  }
  
  frequency_ = frequency;
  train_phase_ = train_phase;
  max_train_phase_ = max_train_phase;
}

}  // namespace stages