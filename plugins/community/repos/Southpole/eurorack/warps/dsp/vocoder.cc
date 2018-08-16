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
// Vocoder.

#include "warps/dsp/vocoder.h"

#include <algorithm>

#include "stmlib/dsp/cosine_oscillator.h"
#include "stmlib/dsp/units.h"

namespace warps {

using namespace std;
using namespace stmlib;

void Vocoder::Init(float sample_rate) {
  modulator_filter_bank_.Init(sample_rate);
  carrier_filter_bank_.Init(sample_rate);
  limiter_.Init();

  release_time_ = 0.5f;
  formant_shift_ = 0.5f;
  
  BandGain zero;
  zero.carrier = 0.0f;
  zero.vocoder = 0.0f;
  fill(&previous_gain_[0], &previous_gain_[kNumBands], zero);
  fill(&gain_[0], &gain_[kNumBands], zero);
  
  for (int32_t i = 0; i < kNumBands; ++i) {
    follower_[i].Init();
  }
}

void Vocoder::Process(
    const float* modulator,
    const float* carrier,
    float* out,
    size_t size) {
  // Run through filter banks.
  modulator_filter_bank_.Analyze(modulator, size);
  carrier_filter_bank_.Analyze(carrier, size);
  
  // Set the attack/release release_time of envelope followers.
  float f = 80.0f * SemitonesToRatio(-72.0f * release_time_);
  for (int32_t i = 0; i < kNumBands; ++i) {
    float decay = f / modulator_filter_bank_.band(i).sample_rate;
    follower_[i].set_attack(decay * 2.0f);
    follower_[i].set_decay(decay * 0.5f);
    follower_[i].set_freeze(release_time_ > 0.995f);
    f *= 1.2599f;  // 2 ** (4/12.0), a third octave.
  }
  
  // Compute the amplitude (or modulation amount) in all bands.
  float formant_shift_amount = 2.0f * fabs(formant_shift_ - 0.5f);
  formant_shift_amount *= (2.0f - formant_shift_amount);
  formant_shift_amount *= (2.0f - formant_shift_amount);
  float envelope_increment = 4.0f * SemitonesToRatio(-48.0f * formant_shift_);
  float envelope = 0.0f;
  const float kLastBand = kNumBands - 1.0001f;
  for (int32_t i = 0; i < kNumBands; ++i) {
    float source_band = envelope;
    CONSTRAIN(source_band, 0.0f, kLastBand);
    MAKE_INTEGRAL_FRACTIONAL(source_band);
    float a = follower_[source_band_integral].peak();
    float b = follower_[source_band_integral + 1].peak();
    float band_gain = (a + (b - a) * source_band_fractional);
    float attenuation = envelope - kLastBand;
    if (attenuation >= 0.0f) {
      band_gain *= 1.0f / (1.0f + 1.0f * attenuation);
    }
    envelope += envelope_increment;

    gain_[i].carrier = band_gain * formant_shift_amount;
    gain_[i].vocoder = 1.0f - formant_shift_amount;
  }
        
  for (int32_t i = 0; i < kNumBands; ++i) {
    size_t band_size = size / modulator_filter_bank_.band(i).decimation_factor;
    const float step = 1.0f / static_cast<float>(band_size);

    float* carrier = carrier_filter_bank_.band(i).samples;
    float* modulator = modulator_filter_bank_.band(i).samples;
    float* envelope = tmp_;

    follower_[i].Process(modulator, envelope, band_size);
    
    float vocoder_gain = previous_gain_[i].vocoder;
    float vocoder_gain_increment = (gain_[i].vocoder - vocoder_gain) * step;
    float carrier_gain = previous_gain_[i].carrier;
    float carrier_gain_increment = (gain_[i].carrier - carrier_gain) * step;
    for (size_t j = 0; j < band_size; ++j) {
      carrier[j] *= (carrier_gain + vocoder_gain * envelope[j]);
      vocoder_gain += vocoder_gain_increment;
      carrier_gain += carrier_gain_increment;
    }
    
    previous_gain_[i] = gain_[i];
  }

  carrier_filter_bank_.Synthesize(out, size);
  limiter_.Process(out, 1.4f, size);
}

}  // namespace warps
