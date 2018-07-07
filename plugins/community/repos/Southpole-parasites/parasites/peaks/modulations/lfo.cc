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
// LFO.

#include "peaks/modulations/lfo.h"

#include <cstdio>

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

#include "peaks/resources.h"

namespace peaks {

const uint16_t kSlopeBits = 12;
const uint32_t kSyncCounterMaxTime = 8 * 48000;

using namespace stmlib;

void Lfo::Init() {
  rate_ = 0;
  shape_ = LFO_SHAPE_SQUARE;
  parameter_ = 0;
  reset_phase_ = 0;
  sync_ = false;
  previous_parameter_ = 32767;
  sync_counter_ = kSyncCounterMaxTime;
  level_ = 32767;
  pattern_predictor_.Init();
}

const int16_t presets[7][2] = {
  { LFO_SHAPE_SINE, 0 },
  { LFO_SHAPE_TRIANGLE, 0 },
  { LFO_SHAPE_TRIANGLE, 32767 },
  { LFO_SHAPE_SQUARE, 0 },
  { LFO_SHAPE_STEPS, 0 },
  { LFO_SHAPE_NOISE, -32767 },
  { LFO_SHAPE_NOISE, 32767 },
};

void Lfo::set_shape_parameter_preset(uint16_t value) {
  value = (value >> 8) * 7 >> 8;
  set_shape(static_cast<LfoShape>(presets[value][0]));
  set_parameter(presets[value][1]);
}

void Lfo::FillBuffer(
    InputBuffer* input_buffer,
    OutputBuffer* output_buffer) {
  if (!sync_) {
    int32_t a = lut_lfo_increments[rate_ >> 8];
    int32_t b = lut_lfo_increments[(rate_ >> 8) + 1];
    phase_increment_ = a + (((b - a) >> 1) * (rate_ & 0xff) >> 7);
  }
  uint8_t size = kBlockSize;  
  while (size--) {
    ++sync_counter_;
    uint8_t control = input_buffer->ImmediateRead();
    if (control & CONTROL_GATE_RISING) {
      bool reset_phase = true;
      if (sync_) {
        if (sync_counter_ < kSyncCounterMaxTime) {
          uint32_t period = 0;
          if (sync_counter_ < 1920) {
            period = (3 * period_ + sync_counter_) >> 2;
            reset_phase = false;
          } else {
            period = pattern_predictor_.Predict(sync_counter_);
          }
          if (period != period_) {
            period_ = period;
            phase_increment_ = 0xffffffff / period_;
          }
        }
        sync_counter_ = 0;
      }
      if (reset_phase) {
        phase_ = reset_phase_;
      }
    }
    phase_ += phase_increment_;
    int32_t sample = (this->*compute_sample_fn_table_[shape_])();
    output_buffer->Overwrite(sample * level_ >> 15);
  }
}

int16_t Lfo::ComputeSampleSine() {
  uint32_t phase = phase_;
  int16_t sine = Interpolate1022(wav_sine, phase);
  int16_t sample;
  if (parameter_ > 0) {
    int32_t wf_balance = parameter_;
    int32_t wf_gain = 2048 + \
        (static_cast<int32_t>(parameter_) * (65535 - 2048) >> 15);
    int32_t original = sine;
    int32_t folded = Interpolate1022(
        wav_fold_sine, original * wf_gain + (1UL << 31));
    sample = original + ((folded - original) * wf_balance >> 15);
  } else {
    int32_t wf_balance = -parameter_;
    int32_t original = sine;
    phase += 1UL << 30;
    int32_t tri = phase < (1UL << 31) ? phase << 1 : ~(phase << 1);
    int32_t folded = Interpolate1022(wav_fold_power, tri);
    sample = original + ((folded - original) * wf_balance >> 15);
  }
  return sample;
}

int16_t Lfo::ComputeSampleTriangle() {
  if (parameter_ != previous_parameter_) {
    uint16_t slope_offset = parameter_ + 32768;
    if (slope_offset <= 1) {
      decay_factor_ = 32768 << kSlopeBits;
      attack_factor_ = 1 << (kSlopeBits - 1);
    } else {
      decay_factor_ = (32768 << kSlopeBits) / slope_offset;
      attack_factor_ = (32768 << kSlopeBits) / (65536 - slope_offset);
    }
    end_of_attack_ = (static_cast<uint32_t>(slope_offset) << 16);
    previous_parameter_ = parameter_;
  }
  
  uint32_t phase = phase_;
  uint32_t skewed_phase = phase;
  if (phase < end_of_attack_) {
    skewed_phase = (phase >> kSlopeBits) * decay_factor_;
  } else {
    skewed_phase = ((phase - end_of_attack_) >> kSlopeBits) * attack_factor_;
    skewed_phase += 1L << 31;
  }
  return skewed_phase < 1UL << 31
      ? -32768 + (skewed_phase >> 15)
      :  32767 - (skewed_phase >> 15);
}

int16_t Lfo::ComputeSampleSquare() {
  uint32_t threshold = static_cast<uint32_t>(parameter_ + 32768) << 16;
  if (threshold < (phase_increment_ << 1)) {
    threshold = phase_increment_ << 1;
  } else if (~threshold < (phase_increment_ << 1)) {
    threshold = ~(phase_increment_ << 1);
  }
  return phase_ < threshold ? 32767 : -32767;
}

int16_t Lfo::ComputeSampleSteps() {
  uint16_t quantization_levels = 2 + (((parameter_ + 32768) * 15) >> 16);
  uint16_t scale = 65535 / (quantization_levels - 1);
  uint32_t phase = phase_;
  uint32_t tri_phase = phase;
  uint32_t tri = tri_phase < (1UL << 31) ? tri_phase << 1 : ~(tri_phase << 1);
  return ((tri >> 16) * quantization_levels >> 16) * scale - 32768;
}

int16_t Lfo::ComputeSampleNoise() {
  uint32_t phase = phase_;
  if (phase < phase_increment_) {
    value_ = next_value_;
    next_value_ = Random::GetSample();
  }
  int16_t sample;
  int32_t linear_interpolation = value_ + \
      ((next_value_ - value_) * static_cast<int32_t>(phase >> 17) >> 15);
  if (parameter_ < 0) {
    int32_t balance = parameter_ + 32767;
    sample = value_ + ((linear_interpolation - value_) * balance >> 15);
  } else {
    int16_t raised_cosine = Interpolate824(lut_raised_cosine, phase) >> 1;
    int32_t smooth_interpolation = value_ + \
        ((next_value_ - value_) * raised_cosine >> 15);
    sample = linear_interpolation + \
        ((smooth_interpolation - linear_interpolation) * parameter_ >> 15);
  }
  return sample;
}

/* static */
Lfo::ComputeSampleFn Lfo::compute_sample_fn_table_[] = {
  &Lfo::ComputeSampleSine,
  &Lfo::ComputeSampleTriangle,
  &Lfo::ComputeSampleSquare,
  &Lfo::ComputeSampleSteps,
  &Lfo::ComputeSampleNoise
};

}  // namespace peaks
