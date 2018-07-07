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
// Compressor.

#include "streams/compressor.h"

// #include <cmath>

#include "stmlib/utils/dsp.h"

namespace streams {

using namespace stmlib;

// 256 LSB <=> 1.55dB
const int32_t kGainConstant = 1 / (1.55 / 6.0 * 65536.0 / 256.0) * 65536;

void Compressor::Init() {
  detector_ = 0;
}

/* static */
int32_t Compressor::Log2(int32_t value) {
  if (value <= 0) {
    value = 1;
  }
  int32_t log_value = 0;
  while (value >= 512) {
    value >>= 1;
    log_value += 65536;
  }
  while (value < 256) {
    value <<= 1;
    log_value -= 65536;
  }
  // Value is between 256 and 512, we can use the LUT.
  return log_value + lut_log2[value - 256];
}

/* static */
int32_t Compressor::Exp2(int32_t value) {
  int32_t num_shifts = 0;
  while (value >= 65536) {
    ++num_shifts;
    value -= 65536;
  }
  while (value < 0) {
    --num_shifts;
    value += 65536;
  }
  
  // Value is between 0 and 65535, we can use the LUT.
  int32_t a = lut_exp2[value >> 8];
  int32_t b = lut_exp2[(value >> 8) + 1];
  int32_t mantissa = a + ((b - a) * (value & 0xff) >> 8);
  return num_shifts >= 0 ? mantissa << num_shifts : mantissa >> -num_shifts;
}

/* static */
int32_t Compressor::Compress(
    int32_t squared_level,
    int32_t threshold,
    int32_t ratio,
    bool soft_knee) {
  int32_t level = (Log2(squared_level) >> 1) - 15 * 65536;  // 15-bit peak
  int32_t position = level - threshold;
  
  if (position < 0) {
    return 0;
  }
  
  int32_t attenuation = position - (position * ratio >> 8);
  if (attenuation < 65535 && soft_knee) {
    int32_t a = lut_soft_knee[attenuation >> 8];
    int32_t b = lut_soft_knee[(attenuation >> 8) + 1];
    int32_t soft_knee = a + ((b - a) * (attenuation & 0xff) >> 8);
    attenuation += \
        (soft_knee - attenuation) * ((65535 - attenuation) >> 1) >> 15;
  }
  return -attenuation;
}

void Compressor::Process(
    int16_t audio,
    int16_t excite,
    uint16_t* gain,
    uint16_t* frequency) {
  int32_t energy;
  int64_t error;
  
  // Detect the RMS level on the EXCITE input.
  energy = excite;
  energy *= energy;
  error = energy - sidechain_signal_detector_;
  if (error > 0) {
    sidechain_signal_detector_ += error;
  } else {
    // Decay time: 5s.
    sidechain_signal_detector_ += error * 14174 >> 31;
  }
  
  // If there is no signal on the "excite" input, disable sidechain and
  // compress by metering input.
  if (sidechain_signal_detector_ < (1024 * 1024)) {
    energy = audio;
    energy *= energy;
  }
  
  // Detect the RMS level on the EXCITE or AUDIO input - whichever active.
  error = energy - detector_;
  if (error > 0) {
    if (attack_coefficient_ == -1) {
      detector_ += error;
    } else {
      detector_ += error * attack_coefficient_ >> 31;
    }
  } else {
    detector_ += error * decay_coefficient_ >> 31;
  }
  
  int32_t g = Compress(detector_, threshold_, ratio_, soft_knee_);
  gain_reduction_ = g >> 3;
  g = kUnityGain + ((g + makeup_gain_) * kGainConstant >> 16);
  if (g > 65535) {
    g = 65535;
  }
  
  *gain = g;
  // float ogain = powf(10.0f, 1.55f / 20.0f * (g - kUnityGain) / 256.0f);
  // printf("%f %f\n", gain_reduction_ / 32768.0 * 24, 20 * logf(ogain) / logf(10.0f));
  *frequency = 65535;
}

}  // namespace streams
