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
// Strumming logic.

#ifndef RINGS_DSP_STRUMMER_H_
#define RINGS_DSP_STRUMMER_H_

#include "stmlib/stmlib.h"

#include "rings/dsp/onset_detector.h"
#include "rings/dsp/part.h"

namespace rings {

class Strummer {
 public:
  Strummer() { }
  ~Strummer() { }
  
  void Init(float ioi, float sr) {
    onset_detector_.Init(
        8.0f / kSampleRate,
        160.0f / kSampleRate,
        1600.0f / kSampleRate,
        sr,
        ioi);
    inhibit_timer_ = static_cast<int32_t>(ioi * sr);
    inhibit_counter_ = 0;
    previous_note_ = 69.0f;
  }
  
  void Process(
      const float* in,
      size_t size,
      PerformanceState* performance_state) {
    
    bool has_onset = in && onset_detector_.Process(in, size);
    bool note_changed = fabs(performance_state->note - previous_note_) > 0.4f;

    int32_t inhibit_timer = inhibit_timer_;
    if (performance_state->internal_strum) {
      bool has_external_note_cv = !performance_state->internal_note;
      bool has_external_exciter = !performance_state->internal_exciter;
      if (has_external_note_cv) {
        performance_state->strum = note_changed;
      } else if (has_external_exciter) {
        performance_state->strum = has_onset;
        // Use longer inhibit time for onset detector.
        inhibit_timer *= 4;
      } else {
        // Nothing is connected. Should the module play itself in this case?
        performance_state->strum = false;
      }
    }

    if (inhibit_counter_) {
      --inhibit_counter_;
      performance_state->strum = false;
    } else {
      if (performance_state->strum) {
        inhibit_counter_ = inhibit_timer;
      }
    }
    previous_note_ = performance_state->note;
  }

 private:
  float previous_note_;
  int32_t inhibit_counter_;
  int32_t inhibit_timer_;
  
  OnsetDetector onset_detector_;
  
  DISALLOW_COPY_AND_ASSIGN(Strummer);
};

}  // namespace rings

#endif  // RINGS_DSP_STRUMMER_H_
