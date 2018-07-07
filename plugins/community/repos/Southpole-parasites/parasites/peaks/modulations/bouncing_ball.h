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
// Bouncing ball.

#ifndef PEAKS_MODULATIONS_BOUNCING_BALL_H_
#define PEAKS_MODULATIONS_BOUNCING_BALL_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/utils/dsp.h"
#include "peaks/resources.h"

#include "peaks/gate_processor.h"

namespace peaks {

class BouncingBall {
 public:
  BouncingBall() { }
  ~BouncingBall() { }
  
  void Init() {
    initial_amplitude_ = 65535L << 14;
    gravity_ = 40;
    bounce_loss_ = 4095;
    initial_velocity_ = 0;
  }
    
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      set_initial_amplitude(65535);
      set_initial_velocity(0);
      set_gravity(parameter[0]);
      set_bounce_loss(parameter[1]);
    } else {
      set_gravity(parameter[0]);
      set_bounce_loss(parameter[1]);
      set_initial_amplitude(parameter[2]);
      set_initial_velocity(parameter[3] - 32768);
    }
  }
  
  inline int16_t ProcessSingleSample(uint8_t control) {
    if (control & CONTROL_GATE_RISING) {
      velocity_ = initial_velocity_;
      position_ = initial_amplitude_;
    }
    velocity_ -= gravity_;
    position_ += velocity_;
    if (position_ < 0) {
      position_ = 0;
      velocity_ = -(velocity_ >> 12) * bounce_loss_;
    }
    if (position_ > (32767L << 15)) {
      position_ = 32767L << 15;
      velocity_ = -(velocity_ >> 12) * bounce_loss_;
    }
    return position_ >> 15;
  }
  
  inline void set_gravity(uint16_t gravity) {
    gravity_ = stmlib::Interpolate88(lut_gravity, gravity);
  }
  
  inline void set_bounce_loss(uint16_t bounce_loss) {
    uint32_t b = 65535 - bounce_loss;
    b = b * b >> 16;
    bounce_loss_ = 4095 - (b >> 4);
  }

  inline void set_initial_amplitude(uint16_t initial_amplitude) {
    initial_amplitude_ = static_cast<int32_t>(initial_amplitude) << 14;
  }
  
  inline void set_initial_velocity(int16_t initial_velocity) {
    initial_velocity_ = static_cast<int32_t>(initial_velocity) << 4;
  }
  
  inline bool FillBuffer() const {
    return true;
  }
  
 private:
  int32_t gravity_;
  int32_t bounce_loss_;
  int32_t initial_amplitude_;
  int32_t initial_velocity_;
   
  int32_t velocity_;
  int32_t position_; 

  DISALLOW_COPY_AND_ASSIGN(BouncingBall);
};

}  // namespace peaks

#endif  // PEAKS_MODULATIONS_BOUNCING_BALL_H_
