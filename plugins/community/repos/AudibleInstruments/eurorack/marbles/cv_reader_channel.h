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
// CV reader channel.

#ifndef MARBLES_CV_READER_CHANNEL_H_
#define MARBLES_CV_READER_CHANNEL_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"

namespace marbles {

enum PotState {
  POT_STATE_TRACKING,
  POT_STATE_LOCKED,
  POT_STATE_CATCHING_UP
};

class HysteresisFilter {
 public:
  HysteresisFilter() { }
  ~HysteresisFilter() { }
  
  void Init(float threshold) {
    value_ = 0.0f;
    threshold_ = threshold;
  }
  
  inline float Process(float value) {
    float error = value - value_;
    if (error > threshold_) {
      value_ = value - threshold_;
    } else if (error < -threshold_) {
      value_ = value + threshold_;
    }
    return value_;
  }
 private:
  float value_;
  float threshold_;
  
  DISALLOW_COPY_AND_ASSIGN(HysteresisFilter);
};

class CvReaderChannel {
 public:
  CvReaderChannel() { }
  ~CvReaderChannel() { }

  // Because of the large number of initialization parameters, they are
  // passed in one single struct.
  struct Settings {
    float cv_lp;
    float pot_scale;
    float pot_offset;
    float pot_lp;
    float min;
    float max;
    float hysteresis;
  };

  void Init(float* cv_scale, float* cv_offset, const Settings& settings) {
    cv_scale_ = cv_scale;
    cv_offset_ = cv_offset;
    cv_lp_ = settings.cv_lp;
    pot_scale_ = settings.pot_scale + 2.0f * settings.hysteresis;
    pot_offset_ = settings.pot_offset - settings.hysteresis;
    pot_lp_ = settings.pot_lp;
    min_ = settings.min;
    max_ = settings.max;
  
    raw_cv_value_ = 0.0f;
    cv_value_ = 0.0f;
    pot_value_ = 0.0f;
    stored_pot_value_ = 0.0f;
    attenuverter_value_ = 0.0f;
    previous_pot_value_ = 0.0f;

    pot_state_ = POT_STATE_TRACKING;
    
    hystereis_filter_.Init(settings.hysteresis);
  }
  
  inline float Process(float pot, float cv) {
    return Process(pot, cv, 1.0f);
  }
  
  inline float Process(float pot, float cv, float attenuverter) {
    cv *= *cv_scale_;
    cv += *cv_offset_;

    attenuverter -= 0.5f;
    attenuverter = attenuverter * attenuverter * attenuverter * 8.0f;
    ONE_POLE(attenuverter_value_, attenuverter, pot_lp_);

    raw_cv_value_ = cv;
    ONE_POLE(cv_value_, cv, cv_lp_);
    ONE_POLE(pot_value_, pot, pot_lp_);
  
    switch (pot_state_) {
      case POT_STATE_TRACKING:
        stored_pot_value_ = pot_value_;
        previous_pot_value_ = pot_value_;
        break;
    
      case POT_STATE_LOCKED:
        break;
      
      case POT_STATE_CATCHING_UP:
        {
          if (fabs(pot_value_ - previous_pot_value_) > 0.01f) {
            float delta = pot_value_ - previous_pot_value_;

            float skew_ratio = delta > 0.0f
              ? (1.001f - stored_pot_value_) / (1.001f - previous_pot_value_)
              : (0.001f + stored_pot_value_) / (0.001f + previous_pot_value_);
            CONSTRAIN(skew_ratio, 0.1f, 10.0f);
          
            stored_pot_value_ += skew_ratio * delta;
            CONSTRAIN(stored_pot_value_, 0.0f, 1.0f);
          
            if (fabs(stored_pot_value_ - pot_value_) < 0.01f) {
              pot_state_ = POT_STATE_TRACKING;
            }

            previous_pot_value_ = pot_value_;
          }
        }
        break;
    };

    float value = hystereis_filter_.Process(
        cv_value_ * attenuverter_value_ + this->pot());
    CONSTRAIN(value, min_, max_);
  
    return value;
  }

  inline float cv() const { return cv_value_; }
  inline float scaled_raw_cv() const { return raw_cv_value_; }
  inline float unscaled_cv_lp() const {
    return (cv_value_ - *cv_offset_) / (*cv_scale_);
  }
  
  inline float pot() const {
    return stored_pot_value_ * pot_scale_ + pot_offset_;
  }
  inline float unscaled_pot() const { return pot_value_; }

  inline void LockPot() {
    pot_state_ = POT_STATE_LOCKED;
  }

  inline void UnlockPot() {
    if (pot_state_ == POT_STATE_LOCKED) {
      previous_pot_value_ = pot_value_;
      pot_state_ = POT_STATE_CATCHING_UP;
    }
  }

 private:
  float* cv_scale_;
  float* cv_offset_;
  float raw_cv_value_;
  float cv_lp_;
  float pot_scale_;
  float pot_offset_;
  float pot_lp_;
  float attenuverter_lp_;
  float min_;
  float max_;

  PotState pot_state_;

  float cv_value_;
  float pot_value_;  // Value after low-pass filtering.
  float previous_pot_value_;
  float stored_pot_value_;  // The actual parameter value.
  float attenuverter_value_;
  
  HysteresisFilter hystereis_filter_;

  DISALLOW_COPY_AND_ASSIGN(CvReaderChannel);
};


}  // namespace marbles

#endif  // MARBLES_CV_READER_CHANNEL_H_
