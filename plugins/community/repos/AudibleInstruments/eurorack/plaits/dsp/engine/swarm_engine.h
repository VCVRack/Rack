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
// Swarm of sawtooths and sines.

#ifndef PLAITS_DSP_ENGINE_SWARM_ENGINE_H_
#define PLAITS_DSP_ENGINE_SWARM_ENGINE_H_

#include "stmlib/dsp/polyblep.h"
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "plaits/dsp/engine/engine.h"
#include "plaits/dsp/oscillator/oscillator.h"
#include "plaits/dsp/oscillator/string_synth_oscillator.h"
#include "plaits/dsp/oscillator/sine_oscillator.h"
#include "plaits/resources.h"

namespace plaits {

const int kNumSwarmVoices = 8;

class GrainEnvelope {
 public:
  GrainEnvelope() { }
  ~GrainEnvelope() { }
  
  void Init() {
    from_ = 0.0f;
    interval_ = 1.0f;
    phase_ = 1.0f;
    fm_ = 0.0f;
    amplitude_ = 0.5f;
    previous_size_ratio_ = 0.0f;
  }
  
  inline void Step(float rate, bool burst_mode, bool start_burst) {
    bool randomize = false;
    if (start_burst) {
      phase_ = 0.5f;
      fm_ = 16.0f;
      randomize = true;
    } else {
      phase_ += rate * fm_;
      if (phase_ >= 1.0f) {
        phase_ -= static_cast<float>(static_cast<int>(phase_));
        randomize = true;
      }
    }
    
    if (randomize) {
      from_ += interval_;
      interval_ = stmlib::Random::GetFloat() - from_;
      // Randomize the duration of the grain.
      if (burst_mode) {
        fm_ *= 0.8f + 0.2f * stmlib::Random::GetFloat();
      } else {
        fm_ = 0.5f + 1.5f * stmlib::Random::GetFloat();
      }
    }
  }
  
  inline float frequency(float size_ratio) const {
    // We approximate two overlapping grains of frequencies f1 and f2
    // By a continuous tone ramping from f1 to f2. This allows a continuous
    // transition between the "grain cloud" and "swarm of glissandi" textures.
    if (size_ratio < 1.0f) {
      return 2.0f * (from_ + interval_ * phase_) - 1.0f;
    } else {
      return from_;
    }
  }
  
  inline float amplitude(float size_ratio) {
    float target_amplitude = 1.0f;
    if (size_ratio >= 1.0f) {
      float phase = (phase_ - 0.5f) * size_ratio;
      CONSTRAIN(phase, -1.0f, 1.0f);
      float e = stmlib::InterpolateWrap(
          lut_sine, 0.5f * phase + 1.25f, 1024.0f);
      target_amplitude = 0.5f * (e + 1.0f);
    }
    
    if ((size_ratio >= 1.0f) ^ (previous_size_ratio_ >= 1.0f)) {
      filter_coefficient_ = 0.5f;
    }
    filter_coefficient_ *= 0.95f;
    
    previous_size_ratio_ = size_ratio;
    ONE_POLE(amplitude_, target_amplitude, 0.5f - filter_coefficient_);
    return amplitude_;
  }
  
 private:
  float from_;
  float interval_;
  float phase_;
  float fm_;
  float amplitude_;
  float previous_size_ratio_;
  float filter_coefficient_;
  
  DISALLOW_COPY_AND_ASSIGN(GrainEnvelope);
};

class AdditiveSawOscillator {
 public:
  AdditiveSawOscillator() { }
  ~AdditiveSawOscillator() { }

  inline void Init() {
    phase_ = 0.0f;
    next_sample_ = 0.0f;
    frequency_ = 0.01f;
    gain_ = 0.0f;
  }

  inline void Render(
      float frequency,
      float level,
      float* out,
      size_t size) {
    if (frequency >= kMaxFrequency) {
      frequency = kMaxFrequency;
    }
    stmlib::ParameterInterpolator fm(&frequency_, frequency, size);
    stmlib::ParameterInterpolator gain(&gain_, level, size);

    float next_sample = next_sample_;
    float phase = phase_;

    while (size--) {
      float this_sample = next_sample;
      next_sample = 0.0f;

      const float frequency = fm.Next();

      phase += frequency;
  
      if (phase >= 1.0f) {
        phase -= 1.0f;
        float t = phase / frequency;
        this_sample -= stmlib::ThisBlepSample(t);
        next_sample -= stmlib::NextBlepSample(t);
      }

      next_sample += phase;
      *out++ += (2.0f * this_sample - 1.0f) * gain.Next();
    }
    phase_ = phase;
    next_sample_ = next_sample;
  }

 private:
  // Oscillator state.
  float phase_;
  float next_sample_;

  // For interpolation of parameters.
  float frequency_;
  float gain_;

  DISALLOW_COPY_AND_ASSIGN(AdditiveSawOscillator);
};

class SwarmVoice {
 public:
  SwarmVoice() { }
  ~SwarmVoice() { }
  
  void Init(float rank) {
    rank_ = rank;
    envelope_.Init();
    saw_.Init();
    sine_.Init();
  }
  
  void Render(
      float f0,
      float density,
      bool burst_mode,
      bool start_burst,
      float spread,
      float size_ratio,
      float* saw,
      float* sine,
      size_t size) {
    envelope_.Step(density, burst_mode, start_burst);
    
    const float scale = 1.0f / kNumSwarmVoices;
    const float amplitude = envelope_.amplitude(size_ratio) * scale;

    const float expo_amount = envelope_.frequency(size_ratio);
    f0 *= stmlib::SemitonesToRatio(48.0f * expo_amount * spread * rank_);
    
    const float linear_amount = rank_ * (rank_ + 0.01f) * spread * 0.25f;
    f0 *= 1.0f + linear_amount;

    saw_.Render(f0, amplitude, saw, size);
    sine_.Render(f0, amplitude, sine, size);
  };
  
 private:
  float rank_;

  GrainEnvelope envelope_;
  AdditiveSawOscillator saw_;
  FastSineOscillator sine_;
};

class SwarmEngine : public Engine {
 public:
  SwarmEngine() { }
  ~SwarmEngine() { }
  
  virtual void Init(stmlib::BufferAllocator* allocator);
  virtual void Reset();
  virtual void Render(const EngineParameters& parameters,
      float* out,
      float* aux,
      size_t size,
      bool* already_enveloped);
  
 private:
  SwarmVoice swarm_voice_[kNumSwarmVoices];
  
  DISALLOW_COPY_AND_ASSIGN(SwarmEngine);
};

}  // namespace plaits

#endif  // PLAITS_DSP_ENGINE_SWARM_ENGINE_H_