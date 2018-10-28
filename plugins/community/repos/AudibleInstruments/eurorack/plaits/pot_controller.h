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
// Handles the "hold button to tweak a hidden parameter" behaviour of pots.
// A pot can be in 4 states:
// - POT_STATE_TRACKING: the main parameter tracks the position of the pot.
// - POT_STATE_LOCKING: the main parameter no longer tracks the position of
//   the pot. We wait for the pot to move further from its original position
//   to start modifying the hidden parameter.
// - POT_STATE_HIDDEN_PARAMETER: the hidden parameter tracks the position of
//   the pot.
// - POT_STATE_CATCHING_UP: the pot adjusts the main parameter in a relative
//   way, until the position of the pot and the value of the parameter match
//   again.

#ifndef PLAITS_POT_CONTROLLER_H_
#define PLAITS_POT_CONTROLLER_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"

namespace plaits {

enum PotState {
  POT_STATE_TRACKING,
  POT_STATE_LOCKING,
  POT_STATE_HIDDEN_PARAMETER,
  POT_STATE_CATCHING_UP
};
  
class PotController {
 public:
  PotController() { }
  ~PotController() { }
  
  inline void Init(
      float* main_parameter,
      float* hidden_parameter,
      float scale,
      float offset) {
    state_ = POT_STATE_TRACKING;
    was_catching_up_ = false;

    main_parameter_ = main_parameter;
    hidden_parameter_ = hidden_parameter;

    value_ = 0.0f;
    stored_value_ = 0.0f;
    previous_value_ = 0.0f;
    scale_ = scale;
    offset_ = offset;
  }
  
  inline void Lock() {
    if (state_ == POT_STATE_LOCKING || state_ == POT_STATE_HIDDEN_PARAMETER) {
      return;
    }
    if (hidden_parameter_) {
      was_catching_up_ = state_ == POT_STATE_CATCHING_UP;
      state_ = POT_STATE_LOCKING;
    }
  }
  
  inline bool editing_hidden_parameter() const {
    return state_ == POT_STATE_HIDDEN_PARAMETER;
  }
  
  inline void Unlock() {
    if (state_ == POT_STATE_HIDDEN_PARAMETER || was_catching_up_) {
      state_ = POT_STATE_CATCHING_UP;
    } else {
      state_ = POT_STATE_TRACKING;
    }
  }
  
  inline void Realign() {
    state_ = POT_STATE_TRACKING;
  }
  
  inline void ProcessControlRate(float adc_value) {
    ONE_POLE(value_, adc_value, 0.01f);
    if (state_ == POT_STATE_TRACKING) {
      *main_parameter_ = value_ * scale_ + offset_;
    }
  }
  
  inline void ProcessUIRate() {
    switch (state_) {
      case POT_STATE_TRACKING:
        previous_value_ = value_;
        break;
        
      case POT_STATE_LOCKING:
        if (fabsf(value_ - previous_value_) > 0.03f) {
          stored_value_ = previous_value_;
          *hidden_parameter_ = value_;
          state_ = POT_STATE_HIDDEN_PARAMETER;
          previous_value_ = value_;
        }
        break;
      
      case POT_STATE_HIDDEN_PARAMETER:
        *hidden_parameter_ = value_;
        previous_value_ = value_;
        break;
        
      case POT_STATE_CATCHING_UP:
        {
          if (fabsf(value_ - previous_value_) > 0.005f) {
            float delta = value_ - previous_value_;

            float skew_ratio = delta > 0.0f
              ? (1.001f - stored_value_) / (1.001f - previous_value_)
              : (0.001f + stored_value_) / (0.001f + previous_value_);
            CONSTRAIN(skew_ratio, 0.1f, 10.0f);
        
            stored_value_ += skew_ratio * delta;
            CONSTRAIN(stored_value_, 0.0f, 1.0f);
        
            if (fabsf(stored_value_ - value_) < 0.005f) {
              state_ = POT_STATE_TRACKING;
            }
            previous_value_ = value_;
            *main_parameter_ = stored_value_ * scale_ + offset_;
          }
        }
        break;
    }
  }
  
 private:
  PotState state_;
  bool was_catching_up_;
  
  float* main_parameter_;
  float* hidden_parameter_;
  float value_;
  float stored_value_;
  float previous_value_;
  float scale_;
  float offset_;
  
  DISALLOW_COPY_AND_ASSIGN(PotController);
};

}  // namespace plaits

#endif  // PLAITS_POT_CONTROLLER_H_
