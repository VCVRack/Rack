// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (olivier@mutable-instruments.net)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Square oscillator generated from a timer.

#ifndef EDGES_TIMER_OSCILLATOR_H_
#define EDGES_TIMER_OSCILLATOR_H_

#include "avrlibx/avrlibx.h"
#include "avrlibx/system/timer.h"

namespace edges {
  
using namespace avrlibx;

enum PulseWidth {
  PULSE_WIDTH_50,
  PULSE_WIDTH_66,
  PULSE_WIDTH_75,
  PULSE_WIDTH_87,
  PULSE_WIDTH_95,
  PULSE_WIDTH_CV_CONTROLLED
};

class TimerOscillator {
 public:
  TimerOscillator() { }
  ~TimerOscillator() { }

  template<typename Timer>
  void UpdatePitch(int16_t pitch, PulseWidth pulse_width) {
    if (pitch < (24 << 7) && prescaler_ != TIMER_PRESCALER_CLK_64) {
      prescaler_ = TIMER_PRESCALER_CLK_64;
    }
    if (pitch > (104 << 7) && prescaler_ != TIMER_PRESCALER_CLK_8) {
      prescaler_ = TIMER_PRESCALER_CLK_8;
    }
    UpdateTimerParameters(pitch, pulse_width);
    Timer::set_period(period_);
    Timer::set_prescaler(prescaler_);
  }
  
  // Create a x16 version (/2 after triangle conversion) for the NES triangle
  // expander.
  template<typename Timer>
  void SubFollow(const TimerOscillator& t) {
    uint16_t period = t.period();
    period >>= 1;
    value_ = period >> 1;
    Timer::set_period(period);
    Timer::set_prescaler(
        t.prescaler() == TIMER_PRESCALER_CLK_64
        ? TIMER_PRESCALER_CLK_8
        : TIMER_PRESCALER_CLK);
  }
  
  template<typename PWM>
  void Gate(bool gate) {
    PWM::set_value(gate ? value_ : 0);
  }
  
  template<typename PWM, typename Timer>
  void Init() {
    Timer::set_mode(TIMER_MODE_DUAL_PWM_T);
    prescaler_ = TIMER_PRESCALER_CLK_8;
    PWM::Start();
  }
    // Value of the pulse width when the mode is set to "cv controlled".
  inline void set_cv_pw(uint8_t pw) {
    if (pw > 250) {
      pw = 250;
    }
    if (pw < 6) {
      pw = 6;
    }
    cv_pw_ = pw;
  }
  
  inline uint16_t period() const { return period_; }
  inline uint16_t value() const { return value_; }
  inline TimerPrescaler prescaler() const { return prescaler_; }
  
 private:
  void UpdateTimerParameters(int16_t pitch, PulseWidth pulse_width);
  
  uint16_t value_;
  uint16_t period_;
  uint8_t pitch_range_;
  uint8_t cv_pw_;
  TimerPrescaler prescaler_;

  DISALLOW_COPY_AND_ASSIGN(TimerOscillator);
};

}  // namespace edges

#endif  // EDGES_TIMER_OSCILLATOR_H_
