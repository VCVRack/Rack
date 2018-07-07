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
// Vactrol.

#include "streams/vactrol.h"

#include "stmlib/utils/dsp.h"

#include "streams/gain.h"
#include "streams/resources.h"

namespace streams {

using namespace stmlib;

void Vactrol::Init() {
  state_[0] = 0;
  state_[1] = 0;
  state_[2] = 0;
  state_[3] = 0;
  
  excite_ = 0;
}

void Vactrol::Process(
    int16_t audio,
    int16_t excite,
    uint16_t* gain,
    uint16_t* frequency) {
  // Smooth frequency amount parameters.
  frequency_amount_ += (target_frequency_amount_ - frequency_amount_) >> 8;
  frequency_offset_ += (target_frequency_offset_ - frequency_offset_) >> 8;

  int32_t input;
  int32_t error;
  int64_t coefficient = 0;

  if (excite < 0) {
    excite = 0;
  }
  
  // Simple plucked mode.
  if (plucked_) {
    if (gate_ == false) {
      if (excite > kSchmittTriggerThreshold) {
        gate_ = true;
        state_[0] = 32767 << 16;
        state_[1] = 32767 << 16;
      }
    } else {
      if (excite < (kSchmittTriggerThreshold >> 1)) {
        gate_ = false;
      }
    }
    
    // Filter the excitation pulses.
    state_[0] -= static_cast<int64_t>(
        state_[0]) * fast_decay_coefficient_ >> 31;
    state_[1] -= static_cast<int64_t>(
        state_[1]) * decay_coefficient_ >> 31;
    
    // VCF envelope.
    error = state_[0] - state_[2];
    coefficient = error > 0
        ? fast_attack_coefficient_ : fast_decay_coefficient_;
    state_[2] += static_cast<int64_t>(error) * coefficient >> 31;
    
    // VCA envelope.
    error = state_[1] - state_[3];
    coefficient = error > 0 ? fast_attack_coefficient_ : decay_coefficient_;
    // Increase the duration of the tail
    int64_t strength = error > 0 ? error : -error;
    coefficient = (coefficient >> 1) + (coefficient * strength >> 31);
    state_[3] += static_cast<int64_t>(error) * coefficient >> 31;

    uint16_t vcf_amount = state_[2] >> 16;
    uint16_t vca_mount = Interpolate1022(wav_gompertz, (state_[3] >> 2) * 3);
    
    *gain = kAboveUnityGain * vca_mount >> 15;
    *frequency = frequency_offset_ + \
         (frequency_amount_ * vcf_amount >> 15);

    return;
  }
  
  // Low-pass filter the negative edges to prevent fast pulse to immediately
  // decay before the vactrol has started reacting. This allows the EXCITE
  // input to be used for both controlling the vactrol or just plucking it
  // from a trigger.
  error = excite - excite_;
  coefficient = error > 0 ? (1 << 30) : (decay_coefficient_ << 1);
  excite_ += static_cast<int64_t>(error) * coefficient >> 31;
  excite = excite_;
  
  input = frequency_offset_;
  input += frequency_amount_ >> 1;
  input = (65535 + input) >> 1;
  input *= excite;
  
  state_[3] += static_cast<int64_t>(input - state_[3]) * 67976239 >> 31;
  
  error = input - state_[0];
  coefficient = 0;
  if (error > 0) {
    if (state_[1] > 0) {
      coefficient = attack_coefficient_;
      // Increase attack time when the photocell has been desensitized.
      coefficient += coefficient * (255 - (state_[2] >> 23)) >> 6;
    } else {
      coefficient = fast_attack_coefficient_;
    }
  } else {
    if (state_[1] < 0) {
      coefficient = decay_coefficient_;
    } else {
      coefficient = fast_decay_coefficient_;
    }
  }
  // First order.
  state_[0] += static_cast<int64_t>(error) * coefficient >> 31;
  
  // Second order.
  state_[1] += static_cast<int64_t>(error - state_[1]) * coefficient >> 31;
  
  // Memory effect.
  int32_t sensitivity = state_[0];
  if (sensitivity > (1 << 28)) {
    sensitivity = 1 << 31;
  } else {
    sensitivity <<= 3;
  }
  error = sensitivity - state_[2];
  if (error > 0) {
    // Get into the "sensitized" state in 1s.
    state_[2] += static_cast<int64_t>(error) * 138132 >> 31;
  } else {
    // Get out of the "sensitized" state in 60s.
    state_[2] += static_cast<int64_t>(error) * 1151 >> 31;
  }
  
  // Apply non-linearity.
  int32_t index = state_[0] >> 1;
  
  // A little hack to add overshoot...
  index += (state_[3] >> 15) * (state_[1] >> 15) >> 1;
  if (index < 0) {
    index = 0;
  } else if (index >= (1 << 30)) {
    index = (1 << 30) - 1;
  }
  uint16_t amplitude = index < 536870912
      ? Interpolate1022(wav_gompertz, static_cast<uint32_t>(index) << 3)
      : 32767;
  uint16_t cutoff = index >> 14;
  if (cutoff >= 32767) cutoff = 32767;
  cutoff = cutoff * cutoff >> 15;
  *gain = kAboveUnityGain * amplitude >> 15;
  *frequency = frequency_offset_ + \
       (frequency_amount_ * cutoff >> 15);
}

}  // namespace streams
