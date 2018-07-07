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
// Low pass filter for getting stable pitch data.

#ifndef RINGS_DSP_NOTE_FILTER_H_
#define RINGS_DSP_NOTE_FILTER_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/delay_line.h"

namespace rings {

class NoteFilter {
 public:
  enum {
    N = 4  // Median filter order
  };
  NoteFilter() { }
  ~NoteFilter() { }

  void Init(
      float sample_rate,
      float time_constant_fast_edge,
      float time_constant_steady_part,
      float edge_recovery_time,
      float edge_avoidance_delay) {
    fast_coefficient_ = 1.0f / (time_constant_fast_edge * sample_rate);
    slow_coefficient_ = 1.0f / (time_constant_steady_part * sample_rate);
    lag_coefficient_ = 1.0f / (edge_recovery_time * sample_rate);
  
    delayed_stable_note_.Init();
    delayed_stable_note_.set_delay(
        std::min(size_t(15), size_t(edge_avoidance_delay * sample_rate)));
  
    stable_note_ = note_ = 69.0f;
    coefficient_ = fast_coefficient_;
    stable_coefficient_ = slow_coefficient_;
    std::fill(&previous_values_[0], &previous_values_[N], note_);
  }

  inline float Process(float note, bool strum) {
    // If there is a sharp change, follow it instantly.
    if (fabs(note - note_) > 0.4f || strum) {
      stable_note_ = note_ = note;
      coefficient_ = fast_coefficient_;
      stable_coefficient_ = slow_coefficient_;
      std::fill(&previous_values_[0], &previous_values_[N], note);
    } else {
      // Median filtering of the raw ADC value.
      float sorted_values[N];
      std::rotate(
          &previous_values_[0],
          &previous_values_[1],
          &previous_values_[N]);
      previous_values_[N - 1] = note;
      std::copy(&previous_values_[0], &previous_values_[N], &sorted_values[0]);
      std::sort(&sorted_values[0], &sorted_values[N]);
      float median = 0.5f * (sorted_values[(N - 1) / 2] + sorted_values[N / 2]);
    
      // Adaptive lag processor.
      note_ += coefficient_ * (median - note_);
      stable_note_ += stable_coefficient_ * (note_ - stable_note_);

      coefficient_ += lag_coefficient_ * (slow_coefficient_ - coefficient_);
      stable_coefficient_ += lag_coefficient_ * \
          (lag_coefficient_ - stable_coefficient_);
    
      delayed_stable_note_.Write(stable_note_);
    }
    return note_;
  }

  inline float note() const {
    return note_;
  }

  inline float stable_note() const {
    return delayed_stable_note_.Read();
  }

 private:
  float previous_values_[N];
  float note_;
  float stable_note_;
  stmlib::DelayLine<float, 16> delayed_stable_note_;

  float coefficient_;
  float stable_coefficient_;

  float fast_coefficient_;
  float slow_coefficient_;
  float lag_coefficient_;
};

}  // namespace rings

#endif  // RINGS_DSP_NOTE_FILTER_H_
