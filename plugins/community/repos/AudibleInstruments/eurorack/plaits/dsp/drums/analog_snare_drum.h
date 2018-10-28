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
// 808 snare drum model, revisited.

#ifndef PLAITS_DSP_DRUMS_ANALOG_SNARE_DRUM_H_
#define PLAITS_DSP_DRUMS_ANALOG_SNARE_DRUM_H_

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "plaits/dsp/dsp.h"
#include "plaits/dsp/oscillator/sine_oscillator.h"

namespace plaits {

class AnalogSnareDrum {
 public:
  AnalogSnareDrum() { }
  ~AnalogSnareDrum() { }

  static const int kNumModes = 5;

  void Init() {
    pulse_remaining_samples_ = 0;
    pulse_ = 0.0f;
    pulse_height_ = 0.0f;
    pulse_lp_ = 0.0f;
    noise_envelope_ = 0.0f;
    sustain_gain_ = 0.0f;

    for (int i = 0; i < kNumModes; ++i) {
      resonator_[i].Init();
      oscillator_[i].Init();
    }
    noise_filter_.Init();
  }
  
  void Render(
      bool sustain,
      bool trigger,
      float accent,
      float f0,
      float tone,
      float decay,
      float snappy,
      float* out,
      size_t size) {
    const float decay_xt = decay * (1.0f + decay * (decay - 1.0f));
    const int kTriggerPulseDuration = 1.0e-3 * kSampleRate;
    const float kPulseDecayTime = 0.1e-3 * kSampleRate;
    const float q = 2000.0f * stmlib::SemitonesToRatio(decay_xt * 84.0f);
    const float noise_envelope_decay = 1.0f - 0.0017f * \
        stmlib::SemitonesToRatio(-decay * (50.0f + snappy * 10.0f));
    const float exciter_leak = snappy * (2.0f - snappy) * 0.1f;
    
    snappy = snappy * 1.1f - 0.05f;
    CONSTRAIN(snappy, 0.0f, 1.0f);
    
    if (trigger) {
      pulse_remaining_samples_ = kTriggerPulseDuration;
      pulse_height_ = 3.0f + 7.0f * accent;
      noise_envelope_ = 2.0f;
    }
    
    static const float kModeFrequencies[kNumModes] = {
        1.00f,
        2.00f,
        3.18f,
        4.16f,
        5.62f};
    
    float f[kNumModes];
    float gain[kNumModes];
    
    for (int i = 0; i < kNumModes; ++i) {
      f[i] = std::min(f0 * kModeFrequencies[i], 0.499f);
      resonator_[i].set_f_q<stmlib::FREQUENCY_FAST>(
          f[i],
          1.0f + f[i] * (i == 0 ? q : q * 0.25f));
    }
    
    if (tone < 0.666667f) {
      // 808-style (2 modes)
      tone *= 1.5f;
      gain[0] = 1.5f + (1.0f - tone) * (1.0f - tone) * 4.5f;
      gain[1] = 2.0f * tone + 0.15f;
      std::fill(&gain[2], &gain[kNumModes], 0.0f);
    } else {
      // What the 808 could have been if there were extra modes!
      tone = (tone - 0.666667f) * 3.0f;
      gain[0] = 1.5f - tone * 0.5f;
      gain[1] = 2.15f - tone * 0.7f;
      for (int i = 2; i < kNumModes; ++i) {
        gain[i] = tone;
        tone *= tone;
      }
    }

    float f_noise = f0 * 16.0f;
    CONSTRAIN(f_noise, 0.0f, 0.499f);
    noise_filter_.set_f_q<stmlib::FREQUENCY_FAST>(
        f_noise, 1.0f + f_noise * 1.5f);
        
    
    stmlib::ParameterInterpolator sustain_gain(
        &sustain_gain_,
        accent * decay,
        size);
    
    while (size--) {
      // Q45 / Q46
      float pulse = 0.0f;
      if (pulse_remaining_samples_) {
        --pulse_remaining_samples_;
        pulse = pulse_remaining_samples_ ? pulse_height_ : pulse_height_ - 1.0f;
        pulse_ = pulse;
      } else {
        pulse_ *= 1.0f - 1.0f / kPulseDecayTime;
        pulse = pulse_;
      }
      
      float sustain_gain_value = sustain_gain.Next();
      
      // R189 / C57 / R190 + C58 / C59 / R197 / R196 / IC14
      ONE_POLE(pulse_lp_, pulse, 0.75f);
      
      float shell = 0.0f;
      for (int i = 0; i < kNumModes; ++i) {
        float excitation = i == 0
            ? (pulse - pulse_lp_) + 0.006f * pulse
            : 0.026f * pulse;
        shell += gain[i] * (sustain
            ? oscillator_[i].Next(f[i]) * sustain_gain_value * 0.25f
            : resonator_[i].Process<stmlib::FILTER_MODE_BAND_PASS>(
                  excitation) + excitation * exciter_leak);
      }
      shell = stmlib::SoftClip(shell);
      
      // C56 / R194 / Q48 / C54 / R188 / D54
      float noise = 2.0f * stmlib::Random::GetFloat() - 1.0f;
      if (noise < 0.0f) noise = 0.0f;
      noise_envelope_ *= noise_envelope_decay;
      noise *= (sustain ? sustain_gain_value : noise_envelope_) * snappy * 2.0f;

      // C66 / R201 / C67 / R202 / R203 / Q49
      noise = noise_filter_.Process<stmlib::FILTER_MODE_BAND_PASS>(noise);
      
      // IC13
      *out++ = noise + shell * (1.0f - snappy);
    }
  }

 private:
  int pulse_remaining_samples_;
  float pulse_;
  float pulse_height_;
  float pulse_lp_;
  float noise_envelope_;
  float sustain_gain_;
  
  stmlib::Svf resonator_[kNumModes];
  stmlib::Svf noise_filter_;

  // Replace the resonators in "free running" (sustain) mode.
  SineOscillator oscillator_[kNumModes];
  
  DISALLOW_COPY_AND_ASSIGN(AnalogSnareDrum);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_DRUMS_ANALOG_SNARE_DRUM_H_
