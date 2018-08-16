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
// Circular buffer storing audio samples.

#ifndef CLOUDS_DSP_AUDIO_BUFFER_H_
#define CLOUDS_DSP_AUDIO_BUFFER_H_

#include <algorithm>

#include "stmlib/stmlib.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/utils/dsp.h"

#include "clouds/dsp/mu_law.h"

const int32_t kCrossFadeSize = 256;
const int32_t kInterpolationTail = 8;

namespace clouds {

enum Resolution {
  RESOLUTION_16_BIT,
  RESOLUTION_8_BIT,
  RESOLUTION_8_BIT_DITHERED,
  RESOLUTION_8_BIT_MU_LAW,
};

enum InterpolationMethod {
  INTERPOLATION_ZOH,
  INTERPOLATION_LINEAR,
  INTERPOLATION_HERMITE
};

template<Resolution resolution>
class AudioBuffer {
 public:
  AudioBuffer() { }
  ~AudioBuffer() { }
  
  void Init(
      void* buffer,
      int32_t size,
      int16_t* tail_buffer) {
    s16_ = static_cast<int16_t*>(buffer);
    s8_ = static_cast<int8_t*>(buffer);
    size_ = size - kInterpolationTail;
    write_head_ = 0;
    quantization_error_ = 0.0f;
    crossfade_counter_ = 0;
    if (resolution == RESOLUTION_16_BIT) {
      std::fill(&s16_[0], &s16_[size], 0);
    } else {
      std::fill(
          &s8_[0],
          &s8_[size],
          resolution == RESOLUTION_8_BIT_MU_LAW ? 127 : 0);
    }
    tail_ = tail_buffer;
  }
  
  inline void Resync(int32_t head) {
    write_head_ = head;
    crossfade_counter_ = 0;
  }
  
  inline void Write(float in) {
    if (resolution == RESOLUTION_16_BIT) {
      s16_[write_head_] = stmlib::Clip16(
            static_cast<int32_t>(in * 32768.0f));
    } else if (resolution == RESOLUTION_8_BIT_DITHERED) {
      float sample = in * 127.0f;
      sample += quantization_error_;
      int32_t quantized = static_cast<int32_t>(sample);
      if (quantized < -127) quantized = -127;
      else if (quantized > 127) quantized = 127;
      quantization_error_ = sample - static_cast<float>(in);
      s8_[write_head_] = quantized;
    } else if (resolution == RESOLUTION_8_BIT_MU_LAW) {
      int16_t sample = stmlib::Clip16(static_cast<int32_t>(in * 32768.0f));
      s8_[write_head_] = Lin2MuLaw(sample);
    } else {
      s8_[write_head_] = static_cast<int8_t>(
          stmlib::Clip16(in * 32768.0f) >> 8);
    }
    
    if (resolution == RESOLUTION_16_BIT) {
      if (write_head_ < kInterpolationTail) {
        s16_[write_head_ + size_] = s16_[write_head_];
      }
    } else {
      if (write_head_ < kInterpolationTail) {
        s8_[write_head_ + size_] = s8_[write_head_];
      }
    }
    ++write_head_;
    if (write_head_ >= size_) {
      write_head_ = 0;
    }
  }
  
  inline void WriteFade(
      const float* in,
      int32_t size,
      int32_t stride,
      bool write) {
    if (!write) {
      // Continue recording samples to have something to crossfade with
      // when recording resumes.
      if (crossfade_counter_ < kCrossFadeSize) {
        while (size--) {
          if (crossfade_counter_ < kCrossFadeSize) {
            tail_[crossfade_counter_++] = stmlib::Clip16(
                static_cast<int32_t>(*in * 32767.0f));
            in += stride;
          }
        }
      }
    } else if (write && !crossfade_counter_ && 
        resolution == RESOLUTION_16_BIT &&
        write_head_ >= kInterpolationTail && write_head_ < (size_ - size)) {
      // Fast write routine for the most common case.
      while (size--) {
        s16_[write_head_] = stmlib::Clip16(
            static_cast<int32_t>(*in * 32767.0f));
        ++write_head_;
        in += stride;
      }
    } else {
      while (size--) {
        float sample = *in;
        if (crossfade_counter_) {
          --crossfade_counter_;
          float tail_sample = tail_[kCrossFadeSize - crossfade_counter_];
          float gain = crossfade_counter_ * (1.0f / float(kCrossFadeSize));
          sample += (tail_sample / 32768.0f - sample) * gain;
        }
        Write(sample);
        in += stride;
      }
    }
  }
  
  inline void Write(const float* in, int32_t size, int32_t stride) {
    if (resolution == RESOLUTION_16_BIT
        && write_head_ >= kInterpolationTail && write_head_ < (size_ - size)) {
      // Fast write routine for the most common case.
      while (size--) {
        s16_[write_head_] = stmlib::Clip16(
            static_cast<int32_t>(*in * 32768.0f));
        ++write_head_;
        in += stride;
      }
    } else {
      while (size--) {
        Write(*in);
        in += stride;
      }
    }
  }
  
  template<InterpolationMethod method>
  inline float Read(int32_t integral, uint16_t fractional) const {
    if (method == INTERPOLATION_ZOH) {
      return ReadZOH(integral, fractional);
    } else if (method == INTERPOLATION_LINEAR) {
      return ReadLinear(integral, fractional);
    } else if (method == INTERPOLATION_HERMITE) {
      return ReadHermite(integral, fractional);
    }
  }
  
  inline float ReadZOH(int32_t integral, uint16_t fractional) const {
    if (integral >= size_) {
      integral -= size_;
    }
    
    float x0, scale;
    if (resolution == RESOLUTION_16_BIT) {
      x0 = s16_[integral];
      scale = 1.0f / 32768.0f;
    } else if (resolution == RESOLUTION_8_BIT_MU_LAW) {
      x0 = MuLaw2Lin(s8_[integral]);
      scale = 1.0f / 32768.0f;
    } else {
      x0 = s8_[integral];
      scale = 1.0f / 128.0f;
    }
    return x0 * scale;
  }
  
  inline float ReadLinear(int32_t integral, uint16_t fractional) const {
    if (integral >= size_) {
      integral -= size_;
    }
    
    // assert(integral >= 0 && integral < size_);
    
    float x0, x1, scale;
    float t = static_cast<float>(fractional) / 65536.0f;
    if (resolution == RESOLUTION_16_BIT) {
      x0 = s16_[integral];
      x1 = s16_[integral + 1];
      scale = 1.0f / 32768.0f;
    } else if (resolution == RESOLUTION_8_BIT_MU_LAW) {
      x0 = MuLaw2Lin(s8_[integral]);
      x1 = MuLaw2Lin(s8_[integral + 1]);
      scale = 1.0f / 32768.0f;
    } else {
      x0 = s8_[integral];
      x1 = s8_[integral + 1];
      scale = 1.0f / 128.0f;
    }
    return (x0 + (x1 - x0) * t) * scale;
  }
  
  inline float ReadHermite(int32_t integral, uint16_t fractional) const {
    if (integral >= size_) {
      integral -= size_;
    }
    
    // assert(integral >= 0 && integral < size_);
    
    float xm1, x0, x1, x2, scale;
    float t = static_cast<float>(fractional) / 65536.0f;
    
    if (resolution == RESOLUTION_16_BIT) {
      xm1 = s16_[integral];
      x0 = s16_[integral + 1];
      x1 = s16_[integral + 2];
      x2 = s16_[integral + 3];
      scale = 1.0f / 32768.0f;
    } else if (resolution == RESOLUTION_8_BIT_MU_LAW) {
      xm1 = MuLaw2Lin(s8_[integral]);
      x0 = MuLaw2Lin(s8_[integral + 1]);
      x1 = MuLaw2Lin(s8_[integral + 2]);
      x2 = MuLaw2Lin(s8_[integral + 3]);
      scale = 1.0f / 32768.0f;
    } else {
      xm1 = s8_[integral];
      x0 = s8_[integral + 1];
      x1 = s8_[integral + 2];
      x2 = s8_[integral + 3];
      scale = 1.0f / 128.0f;
    }
    
    // Laurent de Soras's Hermite interpolator.
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return ((((a * t) - b_neg) * t + c) * t + x0) * scale;
  }
  
  inline int32_t size() const { return size_; }
  inline int32_t head() const { return write_head_; }
  
 private:
  int16_t* s16_;
  int8_t* s8_;
  
  float quantization_error_;
  
  int16_t tail_ptr_;

  int32_t size_;
  int32_t write_head_;
  
  int16_t* tail_;
  int32_t crossfade_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(AudioBuffer);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_AUDIO_BUFFER_H_
