// Copyright 2014 Olivier Gillet.
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
// Naive playback of audio stored in a buffer.

#ifndef CLOUDS_DSP_LOOPING_SAMPLE_PLAYER_H_
#define CLOUDS_DSP_LOOPING_SAMPLE_PLAYER_H_

#include "stmlib/stmlib.h"

#include <algorithm>

#include "stmlib/dsp/units.h"

#include "clouds/dsp/audio_buffer.h"
#include "clouds/dsp/frame.h"
#include "clouds/dsp/parameters.h"

#include "clouds/resources.h"

namespace clouds {

const float kCrossfadeDuration = 64.0f;
const int kMultDivSteps = 16;
const float kMultDivs[kMultDivSteps] = {
  1.0f/16.0f, 3.0f/32.0f, 1.0f/8.0f, 3.0f/16.0f,
  1.0f/4.0f, 3.0f/8.0f, 1.0f/2.0f, 3.0f/4.0f,
  1.0f,
  3.0f/2.0f, 2.0f/1.0f, 3.0f/1.0f, 4.0f/1.0f,
  6.0f/1.0f, 8.0f/1.0f, 12.0f/1.0f
};

using namespace stmlib;

class LoopingSamplePlayer {
 public:
  LoopingSamplePlayer() { }
  ~LoopingSamplePlayer() { }
  
  void Init(int32_t num_channels) {
    num_channels_ = num_channels;
    phase_ = 0.0f;
    current_delay_ = 0.0f;
    loop_point_ = 0.0f;
    loop_duration_ = 0.0f;
    tap_delay_ = 0;
    tap_delay_counter_ = 0;
    synchronized_ = false;
    tail_duration_ = 1.0f;
  }
  
  inline bool synchronized() const { return synchronized_; }
  
  template<Resolution resolution>
  void Play(
      const AudioBuffer<resolution>* buffer,
      const Parameters& parameters,
      float* out, size_t size) {

    int32_t max_delay = buffer->size() - kCrossfadeDuration;
    tap_delay_counter_ += size;
    if (tap_delay_counter_ > max_delay) {
      tap_delay_ = 0;
      tap_delay_counter_ = 0;
      synchronized_ = false;
    }
    if (parameters.trigger) {
      if(tap_delay_counter_ > 128) {
        synchronized_ = true;
        tap_delay_ = tap_delay_counter_;
        loop_reset_ = phase_;
        phase_ = 0.0f;
      }
      tap_delay_counter_ = 0;
    }

    if (synchronized_)
      smoothed_tap_delay_ += 0.01f * (tap_delay_ - smoothed_tap_delay_);

    float target_delay = parameters.position * parameters.position * max_delay;
    if (synchronized_) {
      int index = roundf(parameters.position *
                         static_cast<float>(kMultDivSteps));
      CONSTRAIN(index, 0, kMultDivSteps-1);
      do target_delay = kMultDivs[index--] * static_cast<float>(smoothed_tap_delay_);
      while (target_delay > max_delay && index >= 0);
    }

    const float swap_channels = parameters.stereo_spread;

    if (!parameters.freeze) {
      while (size--) {
        float error = (target_delay - current_delay_);
        float delay = current_delay_ + 0.0005f * error;
        current_delay_ = delay;
        int32_t delay_int = (buffer->head() - 4 - size + buffer->size()) << 12;
        delay_int -= static_cast<int32_t>(delay * 4096.0f);
        
        float l = buffer[0].ReadHermite((delay_int >> 12), delay_int << 4);
        if (num_channels_ == 1) {
          *out++ = l;
          *out++ = l;
        } else if (num_channels_ == 2) {
          float r = buffer[1].ReadHermite((delay_int >> 12), delay_int << 4);
          *out++ = l + (r - l) * swap_channels;
          *out++ = r + (l - r) * swap_channels;
        }
      }
      phase_ = 0.0f;
    } else {
      float loop_point = parameters.position * max_delay * 15.0f / 16.0f;
      loop_point += kCrossfadeDuration;
      float d = parameters.size;
      float loop_duration = (0.01f + 0.99f * d * d) * max_delay;
      if (synchronized_) {
        int index = roundf(d * static_cast<float>(kMultDivSteps));
        CONSTRAIN(index, 0, kMultDivSteps-1);
        do loop_duration = kMultDivs[index--] * static_cast<float>(smoothed_tap_delay_);
        while (loop_duration > max_delay && index >= 0);
      }
      if (loop_point + loop_duration >= max_delay) {
        loop_point = max_delay - loop_duration;
      }
      float phase_increment = synchronized_
          ? 1.0f
          : SemitonesToRatio(parameters.pitch);

      while (size--) {
        ONE_POLE(smoothed_tap_delay_, tap_delay_, 0.00001f);

        if (phase_ >= loop_duration_ || phase_ == 0.0f) {
          if (phase_ >= loop_duration_) {
            loop_reset_ = loop_duration_;
          }
          if (loop_reset_ >= loop_duration_) {
            loop_reset_ = loop_duration_;
          }
          tail_start_ = loop_duration_ - loop_reset_ + loop_point_;
          phase_ = 0.0f;
          tail_duration_ = std::min(
              kCrossfadeDuration,
              kCrossfadeDuration * phase_increment);
          loop_point_ = loop_point;
          loop_duration_ = loop_duration;
        }
        phase_ += phase_increment;
        
        float gain = 1.0f;
        if (tail_duration_ != 0.0f) {
          gain = phase_ / tail_duration_;
          CONSTRAIN(gain, 0.0f, 1.0f);
        }
        int32_t delay_int = (buffer->head() - 4 + buffer->size()) << 12;

        float ph = parameters.granular.reverse ?
          loop_duration_ - phase_ :
          phase_;

        int32_t position = delay_int - static_cast<int32_t>(
          (loop_duration_ - ph + loop_point_) * 4096.0f);
        float l = buffer[0].ReadHermite((position >> 12), position << 4);
        if (num_channels_ == 1) {
          out[0] = l * gain;
          out[1] = l * gain;
        } else if (num_channels_ == 2) {
          float r = buffer[1].ReadHermite((position >> 12), position << 4);
          out[0] = (l + (r - l) * swap_channels) * gain;
          out[1] = (r + (l - r) * swap_channels) * gain;
        }
        
        if (gain != 1.0f) {
          gain = 1.0f - gain;
          int32_t position = delay_int - static_cast<int32_t>(
                (-phase_ + tail_start_) * 4096.0f);
        
          float l = buffer[0].ReadHermite((position >> 12), position << 4);
          if (num_channels_ == 1) {
            out[0] += l * gain;
            out[1] += l * gain;
          } else if (num_channels_ == 2) {
            float r = buffer[1].ReadHermite((position >> 12), position << 4);
            out[0] += (l + (r - l) * swap_channels) * gain;
            out[1] += (r + (l - r) * swap_channels) * gain;
          }
        }
        out += 2;
      }
    }
  }
  
 private:
  float phase_;
  float current_delay_;

  float loop_point_;
  float loop_duration_;
  float tail_start_;
  float tail_duration_;
  float loop_reset_;

  bool synchronized_;
  
  int32_t num_channels_;
  int32_t elapsed_;
  int32_t tap_delay_;
  int32_t smoothed_tap_delay_;
  int32_t tap_delay_counter_;

  DISALLOW_COPY_AND_ASSIGN(LoopingSamplePlayer);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_LOOPING_SAMPLE_PLAYER_H_
