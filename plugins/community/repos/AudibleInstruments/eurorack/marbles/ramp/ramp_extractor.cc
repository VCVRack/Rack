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

#include "marbles/ramp/ramp_extractor.h"

#include <algorithm>

#include "marbles/ramp/ramp.h"

#include "stmlib/dsp/dsp.h"

namespace marbles {

using namespace std;
using namespace stmlib;

const float kLogOneFourth = 1.189207115f;
const float kPulseWidthTolerance = 0.05f;

inline bool IsWithinTolerance(float x, float y, float error) {
  return x >= y * (1.0f - error) && x <= y * (1.0f + error);
}

void RampExtractor::Init(float max_frequency) {
  max_frequency_ = max_frequency;
  audio_rate_period_ = 1.0f / (100.0f / 32000.0f);
  audio_rate_period_hysteresis_ = audio_rate_period_;
  Reset();
}

void RampExtractor::Reset() {
  audio_rate_ = false;
  train_phase_ = 0.0f;
  target_frequency_ = frequency_ = 0.0001f;
  lp_coefficient_ = 0.5f;
  next_max_train_phase_ = max_train_phase_ = 0.999f;
  next_f_ratio_ = f_ratio_ = 1.0f;
  reset_counter_ = 1;
  reset_frequency_ = 0.0f;
  reset_interval_ = 32000 * 3;
  
  Pulse p;
  p.bucket = 1;
  p.on_duration = 2000;
  p.total_duration = 4000;
  p.pulse_width = 0.5f;
  fill(&history_[0], &history_[kHistorySize], p);

  current_pulse_ = 0;
  next_bucket_ = 48.0f;
  
  average_pulse_width_ = 0.0f;
  fill(&predicted_period_[0], &predicted_period_[PREDICTOR_LAST], 4000.0f);
  fill(&prediction_accuracy_[0], &prediction_accuracy_[PREDICTOR_LAST], 0.0f);
  fill(
      &prediction_hash_table_[0],
      &prediction_hash_table_[kHashTableSize],
      0.0f);
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

RampExtractor::Prediction RampExtractor::PredictNextPeriod() {
  float last_period = static_cast<float>(history_[current_pulse_].total_duration);
  
  Predictor best_predictor = PREDICTOR_FAST_MOVING_AVERAGE;

  for (int i = PREDICTOR_FAST_MOVING_AVERAGE; i < PREDICTOR_LAST; ++i) {
    float error = (predicted_period_[i] - last_period) / (last_period + 0.01f);
    // Scoring function: 10% error is half as good as 0% error.
    float accuracy = 1.0f / (1.0f + 100.0f * error * error);
    // Slowly trust good predictors, quickly demote predictors who make errors.
    SLOPE(prediction_accuracy_[i], accuracy, 0.1f, 0.5f);

    // (Ugly code but I don't want virtuals for these.)
    switch (i) {
      case PREDICTOR_SLOW_MOVING_AVERAGE:
        ONE_POLE(predicted_period_[i], last_period, 0.1f);
        break;

      case PREDICTOR_FAST_MOVING_AVERAGE:
        ONE_POLE(predicted_period_[i], last_period, 0.5f);
        break;

      case PREDICTOR_HASH:
        {
          size_t t_2 = (current_pulse_ - 2 + kHistorySize) % kHistorySize;
          size_t t_1 = (current_pulse_ - 1 + kHistorySize) % kHistorySize;
          size_t t_0 = current_pulse_;
          
          size_t hash = history_[t_1].bucket + 17 * history_[t_2].bucket;
          ONE_POLE(
              prediction_hash_table_[hash % kHashTableSize],
              last_period,
              0.5f);
              
          hash = history_[t_0].bucket + 17 * history_[t_1].bucket;
          predicted_period_[i] = prediction_hash_table_[hash % kHashTableSize];
          if (predicted_period_[i] == 0.0f) {
            predicted_period_[i] = last_period;
          }
        }
        break;
        
      default:
        {
          // Periodicity detector.
          size_t candidate_period = i - PREDICTOR_PERIOD_1 + 1;
          size_t t = current_pulse_ + 1 + kHistorySize - candidate_period;
          predicted_period_[i] = history_[t % kHistorySize].total_duration;
        }
        break;
    }
    
    if (prediction_accuracy_[i] >= prediction_accuracy_[best_predictor]) {
      best_predictor = Predictor(i);
    }
  }
  
  Prediction p;
  p.period = predicted_period_[best_predictor];
  p.accuracy = prediction_accuracy_[best_predictor];
  
  return p;
}

void RampExtractor::Process(
    Ratio ratio,
    bool always_ramp_to_maximum,
    const GateFlags* gate_flags,
    float* ramp, 
    size_t size) {
  while (size--) {
    GateFlags flags = *gate_flags++;
    // We are done with the previous pulse.
    if (flags & GATE_FLAG_RISING) {
      Pulse& p = history_[current_pulse_];
      const bool record_pulse = p.total_duration < reset_interval_;
      
      if (!record_pulse) {
        // Quite a long pause - the clock has probably been stopped
        // and restarted.
        reset_frequency_ = 0.0f;
        train_phase_ = 0.0f;
        reset_counter_ = ratio.q;
        reset_interval_ = 4 * p.total_duration;
      } else {
        float period = float(p.total_duration);
        if (period <= audio_rate_period_hysteresis_) {
          audio_rate_ = true;
          audio_rate_period_hysteresis_ = audio_rate_period_ * 1.1f;

          average_pulse_width_ = 0.0f;
          
          bool no_glide = f_ratio_ != ratio.to_float();
          f_ratio_ = ratio.to_float();
        
          float frequency = 1.0f / period;
          target_frequency_ = std::min(f_ratio_ * frequency, max_frequency_);
          float up_tolerance = (1.02f + 2.0f * frequency) * frequency_;
          float down_tolerance = (0.98f - 2.0f * frequency) * frequency_;
          no_glide |= target_frequency_ > up_tolerance ||
              target_frequency_ < down_tolerance;
          lp_coefficient_ = no_glide ? 1.0f : period * 0.00001f;
        } else {
          audio_rate_ = false;
          audio_rate_period_hysteresis_ = audio_rate_period_;

          // Compute the pulse width of the previous pulse, and check if the
          // PW has been consistent over the past pulses.
          p.pulse_width = static_cast<float>(p.on_duration) / period;
          average_pulse_width_ = ComputeAveragePulseWidth(kPulseWidthTolerance);
        
          if (p.on_duration < 32) {
            average_pulse_width_ = 0.0f;
          }

          // Try to predict the next interval between pulses. If the prediction
          // has been reliable over the past pulses, or if the PW is steady,
          // we'll be able to make reliable prediction about the time at which
          // the next pulse will occur
          Prediction prediction = PredictNextPeriod();
          frequency_ = 1.0f / prediction.period;
        
          --reset_counter_;
          if (!reset_counter_) {
            next_f_ratio_ = ratio.to_float() * kMaxRampValue;
            next_max_train_phase_ = static_cast<float>(ratio.q);
            if (always_ramp_to_maximum && train_phase_ < max_train_phase_) {
              reset_frequency_ = \
                  (0.01f + max_train_phase_ - train_phase_) * 0.0625f;
            } else {
              reset_frequency_ = 0.0f;
              train_phase_ = 0.0f;
              f_ratio_ = next_f_ratio_;
              max_train_phase_ = next_max_train_phase_;
            }
            reset_counter_ = ratio.q;
          } else {
            float expected = max_train_phase_ - static_cast<float>(reset_counter_);
            float warp =  expected - train_phase_ + 1.0f;
            frequency_ *= max(warp, 0.01f);
          }
        }
        reset_interval_ = static_cast<uint32_t>(
            std::max(4.0f / target_frequency_, 32000 * 3.0f));
        current_pulse_ = (current_pulse_ + 1) % kHistorySize;
      }
      history_[current_pulse_].on_duration = 0;
      history_[current_pulse_].total_duration = 0;
      history_[current_pulse_].bucket = 0;
      next_bucket_ = 48.0f;
    }
    
    // Update history buffer with total duration and on duration.
    ++history_[current_pulse_].total_duration;
    if (flags & GATE_FLAG_HIGH) {
      ++history_[current_pulse_].on_duration;
    }
    if (float(history_[current_pulse_].total_duration) >= next_bucket_) {
      ++history_[current_pulse_].bucket;
      next_bucket_ *= kLogOneFourth;
    }
    
    // If the pulse width is constant, and if a clock falling edge is
    // detected, estimate the period using the on time and the pulse width,
    // and correct the phase increment accordingly.
    if ((flags & GATE_FLAG_FALLING) &&
        average_pulse_width_ > 0.0f) {
      float t_on = static_cast<float>(history_[current_pulse_].on_duration);
      float next = max_train_phase_ - static_cast<float>(reset_counter_) + 1.0f;
      float pw = average_pulse_width_;
      frequency_ = max((next - train_phase_), 0.0f) * pw / ((1.0f - pw) * t_on);
    }
    
    if (audio_rate_) {
      ONE_POLE(frequency_, target_frequency_, lp_coefficient_);
      train_phase_ += frequency_;
      if (train_phase_ >= 1.0f) {
        train_phase_ -= 1.0f;
      }
      *ramp++ = train_phase_;
    } else {
      if (reset_frequency_) {
        train_phase_ += reset_frequency_;
        if (train_phase_ >= max_train_phase_) {
          train_phase_ = 0.0f;
          reset_frequency_ = 0.0f;
          f_ratio_ = next_f_ratio_;
          max_train_phase_ = next_max_train_phase_;
        }
      } else {
        train_phase_ += frequency_;
        if (train_phase_ >= max_train_phase_) {
          if (frequency_ == max_frequency_) {
            train_phase_ -= max_train_phase_;
          } else {
            train_phase_ = max_train_phase_;
          }
        }
      }
    
      float output_phase = train_phase_ * f_ratio_;
      output_phase -= static_cast<float>(static_cast<int>(output_phase));
      *ramp++ = output_phase;
    }
  }
}

}  // namespace marbles