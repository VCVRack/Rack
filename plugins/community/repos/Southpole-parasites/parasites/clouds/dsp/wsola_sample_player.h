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
// WSOLA playback.

#ifndef CLOUDS_DSP_WSOLA_SAMPLE_PLAYER_H_
#define CLOUDS_DSP_WSOLA_SAMPLE_PLAYER_H_

#include <algorithm>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "stmlib/stmlib.h"
#include "stmlib/dsp/units.h"

#include "clouds/dsp/audio_buffer.h"
#include "clouds/dsp/correlator.h"
#include "clouds/dsp/frame.h"
#include "clouds/dsp/window.h"
#include "clouds/dsp/parameters.h"
#include "clouds/resources.h"

namespace clouds {

const int32_t kMaxWSOLASize = 4096;

using namespace stmlib;

class WSOLASamplePlayer {
 public:
  WSOLASamplePlayer() { }
  ~WSOLASamplePlayer() { }
  
  void Init(
      Correlator* correlator,
      int32_t num_channels) {
    correlator_ = correlator;
    num_channels_ = num_channels;

    pitch_ = 0.0f;
    position_ = 0.0f;
    smoothed_pitch_ = 0.0f;

    windows_[0].Init();
    windows_[1].Init();

    next_pitch_ratio_ = 1.0f;
    correlator_loaded_ = true;
    search_source_ = 0;
    search_target_ = 0;
    
    window_size_ = kMaxWSOLASize / 2;
    env_phase_ = 0.0f;
    env_phase_increment_ = 0.5f;

    tap_delay_ = 0;
    tap_delay_counter_ = 0;
    synchronized_ = false;
  }
  
  template<Resolution resolution>
  void Play(
      const AudioBuffer<resolution>* buffer,
      const Parameters& parameters,
      float* out,
      size_t size) {

    int32_t max_delay = buffer->size() - 2 * window_size_;
    tap_delay_counter_ += size;
    if (tap_delay_counter_ > max_delay) {
      tap_delay_ = 0;
      tap_delay_counter_ = 0;
      synchronized_ = false;
    }
    if (parameters.trigger && !parameters.freeze) {
      if(tap_delay_counter_ > 128) {
        synchronized_ = true;
        tap_delay_ = tap_delay_counter_;
      }
      tap_delay_counter_ = 0;
    }

    env_phase_ += env_phase_increment_;
    if (env_phase_ >= 1.0f) {
      env_phase_ = 1.0;
    }
    position_ = parameters.position;
    position_ += (1.0f - env_phase_) * (1.0f - position_);

    pitch_ = parameters.pitch;
    size_factor_ = parameters.size;
    
    if (windows_[0].done() && windows_[1].done()) {
      windows_[1].MarkAsRegenerated();
      ScheduleAlignedWindow(buffer, &windows_[0]);
    }

    const float swap_channels = parameters.stereo_spread;

    while (size--) {
      // Sum the two windows.
      std::fill(&out[0], &out[kMaxNumChannels], 0);
      for (int32_t i = 0; i < 2; ++i) {
        windows_[i].OverlapAdd(buffer, out, num_channels_, swap_channels);
      }

      // Regenerate expired windows.
      for (int32_t i = 0; i < 2; ++i) {
        if (windows_[i].needs_regeneration()) {
          windows_[i].MarkAsRegenerated();
          ScheduleAlignedWindow(buffer, &windows_[1 - i]);
          windows_[1 - i].OverlapAdd(buffer, out, num_channels_, swap_channels);
        }
      }
      out += 2;
    }
  }
  
  template<int32_t num_channels, Resolution resolution>
  int32_t ReadSignBits(
      const AudioBuffer<resolution>* buffer,
      int32_t phase_increment,
      int32_t source,
      int32_t size,
      uint32_t* destination) {
    int32_t phase = 0;
    uint32_t bits = 0;
    uint32_t bit_counter = 0;
    int32_t num_samples = 0;
    if (source < 0) {
      source += buffer->size();
    }
    while ((phase >> 16) < size) {
      int32_t integral = source + (phase >> 16);
      uint16_t fractional = phase & 0xffff;
      float s = buffer[0].ReadLinear(integral, fractional);
      if (num_channels == 2) {
        s += buffer[1].ReadLinear(integral, fractional);
      }
      bits |= s > 0.0f ? 1 : 0;
      if ((bit_counter & 0x1f) == 0x1f) {
        destination[bit_counter >> 5] = bits;
        num_samples += 32;
      }
      ++bit_counter;
      bits <<= 1;
      phase += phase_increment;
    }
    while (bit_counter & 0x1f) {
      if ((bit_counter & 0x1f) == 0x1f) {
        destination[bit_counter >> 5] = bits;
        num_samples += 32;
      }
      ++bit_counter;
      bits <<= 1;
    }
    return num_samples;
  }
  
  template<Resolution resolution>
  void LoadCorrelator(const AudioBuffer<resolution>* buffer) {
    if (correlator_loaded_) {
      return;
    }
    float stride = window_size_ / 2048.0f;
    CONSTRAIN(stride, 1.0f, 2.0f);
    stride *= 65536.0f;
    int32_t increment = static_cast<int32_t>(
          stride * (next_pitch_ratio_ < 1.25f ? 1.25f : next_pitch_ratio_));
    int32_t num_samples = 0;
    if (num_channels_ == 1) {
      num_samples = ReadSignBits<1>(
          buffer,
          increment,
          search_source_,
          window_size_,
          correlator_->source());
      ReadSignBits<1>(
          buffer,
          increment,
          search_target_ - window_size_,
          window_size_ * 2,
          correlator_->destination());
    } else {
      num_samples = ReadSignBits<2>(
          buffer,
          increment,
          search_source_,
          window_size_,
          correlator_->source());
      ReadSignBits<2>(
          buffer,
          increment,
          search_target_ - window_size_,
          window_size_ * 2,
          correlator_->destination());
    }
    correlator_->StartSearch(
        num_samples,
        search_target_ - window_size_ + (window_size_ >> 1),
        increment);
    correlator_loaded_ = true;
  }


  inline bool synchronized() const { return synchronized_; }

 private:
  template<Resolution resolution>
  void ScheduleAlignedWindow(
      const AudioBuffer<resolution>* buffer,
      Window* window) {
    int32_t next_window_position = correlator_->best_match();
    correlator_loaded_ = false;
    window->Start(
        buffer->size(),
        next_window_position - (window_size_ >> 1),
        window_size_,
        static_cast<uint32_t>(next_pitch_ratio_ * 65536.0f));
    
    float pitch_error = pitch_ - smoothed_pitch_;
    float pitch_error_sign = pitch_error < 0.0f ? -1.0 : 1.0;
    pitch_error *= pitch_error_sign;
    if (pitch_error >= 12.0f) {
      pitch_error = 12.0f;
    }
    smoothed_pitch_ += pitch_error * pitch_error_sign;
    float pitch_ratio = SemitonesToRatio(smoothed_pitch_);
    float inv_pitch_ratio = SemitonesToRatio(-smoothed_pitch_);
    next_pitch_ratio_ = pitch_ratio;
    
    float size_factor = SemitonesToRatio((size_factor_ - 1.0f) * 60.0f);
    int32_t new_window_size = static_cast<int32_t>(size_factor * kMaxWSOLASize);
    if (abs(new_window_size - window_size_) > 64) {
      int32_t error = (new_window_size - window_size_) >> 5;
      new_window_size = window_size_ + error;
      window_size_ = new_window_size - (new_window_size % 4);
    }
    
    // The center offset of the window we want to mix in.
    int32_t limit = buffer->size();
    limit -= static_cast<int32_t>(2.0f * window_size_ * inv_pitch_ratio);
    limit -= 2 * window_size_;
    if (limit < 0) {
      limit = 0;
    }
    
    float position = limit * position_;

    if (synchronized_) {
      int index = roundf(position_ * static_cast<float>(kMultDivSteps));
      CONSTRAIN(index, 0, kMultDivSteps-1);
      do position = kMultDivs[index--] * static_cast<float>(tap_delay_);
      while (position > limit && index >= 0);
      /* to compensate partially for the size of the windows.
       * TODO: this is still not completely right... */
      position -= window_size_ * 2;
      if (position < 0) position = 0;
    }

    int32_t target_position = buffer->head();
    target_position -= static_cast<int32_t>(position);
    target_position -= window_size_;

    search_source_ = next_window_position;
    search_target_ = target_position;
  }

  Correlator* correlator_;

  Window windows_[2];

  int32_t window_size_;
  int32_t num_channels_;
  
  float pitch_;
  float smoothed_pitch_;
  float position_;
  float size_factor_;
  
  float next_pitch_ratio_;
  bool correlator_loaded_;
  int32_t search_source_;
  int32_t search_target_;
  
  float env_phase_;
  float env_phase_increment_;

  int32_t tap_delay_;
  int32_t tap_delay_counter_;
  bool synchronized_;
  
  DISALLOW_COPY_AND_ASSIGN(WSOLASamplePlayer);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_WSOLA_SAMPLE_PLAYER_H_
