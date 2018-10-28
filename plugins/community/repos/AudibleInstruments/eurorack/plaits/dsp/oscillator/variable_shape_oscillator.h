// Copyright 2016 Olivier Gillet.
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
// Continuously variable waveform: triangle > saw > square. Both square and
// triangle have variable slope / pulse-width. Additionally, the phase resets
// can be locked to a master frequency.

#ifndef PLAITS_DSP_OSCILLATOR_VARIABLE_SHAPE_OSCILLATOR_H_
#define PLAITS_DSP_OSCILLATOR_VARIABLE_SHAPE_OSCILLATOR_H_

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/polyblep.h"

#include <algorithm>

namespace plaits {

class VariableShapeOscillator {
 public:
  VariableShapeOscillator() { }
  ~VariableShapeOscillator() { }

  void Init() {
    master_phase_ = 0.0f;
    slave_phase_ = 0.0f;
    next_sample_ = 0.0f;
    previous_pw_ = 0.5f;
    high_ = false;
  
    master_frequency_ = 0.0f;
    slave_frequency_ = 0.01f;
    pw_ = 0.5f;
    waveshape_ = 0.0f;
  }
  
  template<bool enable_sync>
  void Render(
      float master_frequency,
      float frequency,
      float pw,
      float waveshape,
      float* out,
      size_t size) {
    if (master_frequency >= kMaxFrequency) {
      master_frequency = kMaxFrequency;
    }
    if (frequency >= kMaxFrequency) {
      frequency = kMaxFrequency;
    }
    
    if (frequency >= 0.25f) {
      pw = 0.5f;
    } else {
      CONSTRAIN(pw, frequency * 2.0f, 1.0f - 2.0f * frequency);
    }

    stmlib::ParameterInterpolator master_fm(
        &master_frequency_, master_frequency, size);
    stmlib::ParameterInterpolator fm(&slave_frequency_, frequency, size);
    stmlib::ParameterInterpolator pwm(&pw_, pw, size);
    stmlib::ParameterInterpolator waveshape_modulation(
        &waveshape_, waveshape, size);

    float next_sample = next_sample_;
    
    while (size--) {
      bool reset = false;
      bool transition_during_reset = false;
      float reset_time = 0.0f;

      float this_sample = next_sample;
      next_sample = 0.0f;
    
      const float master_frequency = master_fm.Next();
      const float slave_frequency = fm.Next();
      const float pw = pwm.Next();
      const float waveshape = waveshape_modulation.Next();
      const float square_amount = std::max(waveshape - 0.5f, 0.0f) * 2.0f;
      const float triangle_amount = std::max(1.0f - waveshape * 2.0f, 0.0f);
      const float slope_up = 1.0f / (pw);
      const float slope_down = 1.0f / (1.0f - pw);

      if (enable_sync) {
        master_phase_ += master_frequency;
        if (master_phase_ >= 1.0f) {
          master_phase_ -= 1.0f;
          reset_time = master_phase_ / master_frequency;
      
          float slave_phase_at_reset = slave_phase_ + \
              (1.0f - reset_time) * slave_frequency;
          reset = true;
          if (slave_phase_at_reset >= 1.0f) {
            slave_phase_at_reset -= 1.0f;
            transition_during_reset = true;
          }
          if (!high_ && slave_phase_at_reset >= pw) {
            transition_during_reset = true;
          }
          float value = ComputeNaiveSample(
              slave_phase_at_reset,
              pw,
              slope_up,
              slope_down,
              triangle_amount,
              square_amount);
          this_sample -= value * stmlib::ThisBlepSample(reset_time);
          next_sample -= value * stmlib::NextBlepSample(reset_time);
        }
      }
      
      slave_phase_ += slave_frequency;
      while (transition_during_reset || !reset) {
        if (!high_) {
          if (slave_phase_ < pw) {
            break;
          }
          float t = (slave_phase_ - pw) / (previous_pw_ - pw + slave_frequency);
          float triangle_step = (slope_up + slope_down) * slave_frequency;
          triangle_step *= triangle_amount;
          
          this_sample += square_amount * stmlib::ThisBlepSample(t);
          next_sample += square_amount * stmlib::NextBlepSample(t);
          this_sample -= triangle_step * stmlib::ThisIntegratedBlepSample(t);
          next_sample -= triangle_step * stmlib::NextIntegratedBlepSample(t);
          high_ = true;
        }
      
        if (high_) {
          if (slave_phase_ < 1.0f) {
            break;
          }
          slave_phase_ -= 1.0f;
          float t = slave_phase_ / slave_frequency;
          float triangle_step = (slope_up + slope_down) * slave_frequency;
          triangle_step *= triangle_amount;

          this_sample -= (1.0f - triangle_amount) * stmlib::ThisBlepSample(t);
          next_sample -= (1.0f - triangle_amount) * stmlib::NextBlepSample(t);
          this_sample += triangle_step * stmlib::ThisIntegratedBlepSample(t);
          next_sample += triangle_step * stmlib::NextIntegratedBlepSample(t);
          high_ = false;
        }
      }
    
      if (enable_sync && reset) {
        slave_phase_ = reset_time * slave_frequency;
        high_ = false;
      }
    
      next_sample += ComputeNaiveSample(
          slave_phase_,
          pw,
          slope_up,
          slope_down,
          triangle_amount,
          square_amount);
      previous_pw_ = pw;

      *out++ = (2.0f * this_sample - 1.0f);
    }
    
    next_sample_ = next_sample;
  }


 private:
  inline float ComputeNaiveSample(
      float phase,
      float pw,
      float slope_up,
      float slope_down,
      float triangle_amount,
      float square_amount) const {
    float saw = phase;
    float square = phase < pw ? 0.0f : 1.0f;
    float triangle = phase < pw
        ? phase * slope_up
        : 1.0f - (phase - pw) * slope_down;
    saw += (square - saw) * square_amount;
    saw += (triangle - saw) * triangle_amount;
    return saw;
  }

  // Oscillator state.
  float master_phase_;
  float slave_phase_;
  float next_sample_;
  float previous_pw_;
  bool high_;

  // For interpolation of parameters.
  float master_frequency_;
  float slave_frequency_;
  float pw_;
  float waveshape_;

  DISALLOW_COPY_AND_ASSIGN(VariableShapeOscillator);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_OSCILLATOR_VARIABLE_SHAPE_OSCILLATOR_H_
