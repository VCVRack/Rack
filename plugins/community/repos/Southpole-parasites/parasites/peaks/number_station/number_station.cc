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
// Number station.

#include "peaks/number_station/number_station.h"

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

#include "peaks/resources.h"

namespace peaks {

const uint8_t kDownsample = 4;
const uint16_t kSlopeBits = 12;
const uint32_t kSyncCounterMaxTime = 8 * 48000;

using namespace stmlib;

void NumberStation::Init() {
  tone_amplitude_ = 0;
  phase_ = 0;
  lp_.Init();
  lp_.set_frequency(120 << 7);
  lp_.set_resonance(16000);
  lp_.set_mode(SVF_MODE_LP);

  hp_.Init();
  hp_.set_frequency(70 << 7);
  hp_.set_resonance(8000);
  hp_.set_mode(SVF_MODE_HP);

  previous_inner_sample_ = 0;
  previous_outer_sample_ = 0;
}

size_t voice_digits[] = {
  0, 4913, 7830, 11306, 14601, 18651, 22308, 26438, 30495, 33296, 36801
};

void NumberStation::FillBuffer(
    InputBuffer* input_buffer,
    OutputBuffer* output_buffer) {
  uint32_t phase_increment;

  if (voice_) {
    phase_increment = pitch_shift_;
  } else {
    uint16_t frequency = tone_;
    int32_t a = lut_lfo_increments[frequency >> 8];
    int32_t b = lut_lfo_increments[(frequency >> 8) + 1];
    phase_increment = a + (((b - a) >> 1) * (frequency & 0xff) >> 7);
    phase_increment <<= 6;
    phase_increment *= digit_ + 1;
  }

  int32_t drift_target = Random::GetSample();
  if (drift_target > drift_) {
    drift_ += (drift_target - drift_) >> 13;
  } else {
    drift_ -= (drift_ - drift_target) >> 13;
  }
  int32_t slow_noise = drift_ << 5;
  CLIP(slow_noise);

  uint8_t size = kBlockSize / kDownsample;
  while (size--) {
    for (uint8_t i = 0; i < kDownsample; ++i) {
      uint8_t control = input_buffer->ImmediateRead();
      if (control & CONTROL_GATE_RISING) {
        uint16_t random = Random::GetSample();
        if (random < transition_probability_) {
          digit_ = random >> 2;
          digit_ = voice_ ? digit_ % 10 : digit_ & 3;
        }
        if (voice_) {
          phase_ = voice_digits[digit_] << 16;
        }
      }
      if (control & CONTROL_GATE) {
        tone_amplitude_ += (32767 - tone_amplitude_) >> 6;
      } else {
        tone_amplitude_ -= tone_amplitude_ >> 6;
        if (tone_amplitude_ < 64 && tone_amplitude_) {
          --tone_amplitude_;
        }
      }
    }
  
    // Generate a distorted sine wave with fluctuating frequency.    
    int32_t digit;
    if (voice_) {
      uint16_t integral = phase_ >> 16;
      uint16_t fractional = phase_ & 0xffff;
      if (integral < voice_digits[digit_ + 1]) {
        uint8_t mask_a = integral * 53;
        uint8_t mask_b = mask_a + 53;
        int32_t a = static_cast<int32_t>(wav_digits[integral] ^ mask_a);
        int32_t b = static_cast<int32_t>(wav_digits[integral + 1] ^ mask_b);
        digit = (a << 8) + (((b - a) * fractional) >> 8);
        digit -= 32768;
        phase_ += phase_increment;
        gate_ = true;
      } else {
        digit = 0;
        gate_ = false;
      }
    } else {
      phase_ += phase_increment + (lp_noise_ << 10);
      digit = Interpolate1022(wav_sine, phase_);
      digit = digit * tone_amplitude_ >> 16;
      gate_ = tone_amplitude_ > 0;
    }
    digit = digit + ((digit - ((digit + 4096) ^ 0x055a)) * distortion_ >> 15);
    
    // Generate narrow-band noise.
    int32_t random_sample = Random::GetSample();
    lp_noise_ += (random_sample - lp_noise_) >> 6;
    noise_phase_ += 238370685;
    int32_t noise = lp_noise_ * Interpolate1022(wav_sine, noise_phase_) >> 12;

    // Generate an interference tone.
    interference_phase_ += 710101260;
    noise += distortion_ * wav_sine[interference_phase_ >> 22] >> 18;

    // Mix signal and noise.
    int32_t inner_sample = digit + ((noise - digit) * noise_ >> 15);

    if (random_sample >= 32767 - (noise_ >> 7)) {
      inner_sample = 0;
    }

    // Final ringmod.
    ringmod_phase_ += 38654706 + (38654706 * drift_ >> 10);
    int32_t ringmod = Interpolate1022(wav_sine, ringmod_phase_) >> 1;
    ringmod = ringmod * inner_sample >> 15;
    inner_sample += ringmod * distortion_ >> 15;
    CLIP(inner_sample)

    // And a pass of wavefolding...
    inner_sample = Interpolate1022(
        wav_fold_sine,
        inner_sample * 8192 + (1UL << 31));

    int32_t outer_sample ;
    outer_sample = lp_.Process(
        hp_.Process((inner_sample + previous_inner_sample_) >> 1));
    output_buffer->Overwrite((previous_outer_sample_ + outer_sample) >> 1);
    output_buffer->Overwrite(outer_sample);
    previous_outer_sample_ = outer_sample;

    outer_sample = lp_.Process(hp_.Process(inner_sample));
    output_buffer->Overwrite((previous_outer_sample_ + outer_sample) >> 1);
    output_buffer->Overwrite(outer_sample);
    previous_outer_sample_ = outer_sample;

    previous_inner_sample_ = inner_sample;
  }
}

}  // namespace peaks
