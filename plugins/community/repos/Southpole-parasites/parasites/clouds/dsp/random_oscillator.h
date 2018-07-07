// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
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
// Smoothed random oscillator

#include "../resources.h"
#include "stmlib/utils/random.h"
#include "stmlib/dsp/dsp.h"

#ifndef CLOUDS_RANDOM_OSCILLATOR_H_
#define CLOUDS_RANDOM_OSCILLATOR_H_

using namespace stmlib;

namespace clouds
{
  const float kOscillationMinimumGap = 0.3f;

  class RandomOscillator
  {
  public:

    void Init() {
      value_ = 0.0f;
      next_value_ = Random::GetFloat() * 2.0f - 1.0f;
    }

    inline void set_slope(float slope) {
      phase_increment_ = 1.0f / fabs(next_value_ - value_) * slope;
      if (phase_increment_ > 1.0f)
        phase_increment_ = 1.0f;
    }

    float Next() {
      phase_ += phase_increment_;
      if (phase_ > 1.0f) {
        phase_--;
        value_ = next_value_;
        direction_ = !direction_;
        float rnd = (1.0f - kOscillationMinimumGap) * Random::GetFloat() + kOscillationMinimumGap;
        next_value_ = direction_ ?
          value_ + (1.0f - value_) * rnd :
          value_ - (1.0f + value_) * rnd;
      }

      float sin = Interpolate(lut_raised_cos, phase_, LUT_RAISED_COS_SIZE-1);
      return value_ + (next_value_ - value_) * sin;
    }

  private:
    float phase_;
    float phase_increment_;
    float value_;
    float next_value_;
    bool direction_;
  };
}

#endif
