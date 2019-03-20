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
// Tidal generator.

#ifndef TIDES_GENERATOR_H_
#define TIDES_GENERATOR_H_

#include "stmlib/stmlib.h"

#include "stmlib/algorithms/pattern_predictor.h"
#include "stmlib/utils/ring_buffer.h"

// #define WAVETABLE_HACK

namespace tides_parasites {

enum GeneratorRange {
  GENERATOR_RANGE_HIGH,
  GENERATOR_RANGE_MEDIUM,
  GENERATOR_RANGE_LOW
};

enum GeneratorMode {
  GENERATOR_MODE_AD,
  GENERATOR_MODE_LOOPING,
  GENERATOR_MODE_AR,
};

enum ControlBitMask {
  CONTROL_FREEZE = 1,
  CONTROL_GATE = 2,
  CONTROL_CLOCK = 4,
  CONTROL_CLOCK_RISING = 8,
  CONTROL_GATE_RISING = 16,
  CONTROL_GATE_FALLING = 32
};

enum FlagBitMask {
  FLAG_END_OF_ATTACK = 1,
  FLAG_END_OF_RELEASE = 2
};

struct GeneratorSample {
  uint16_t unipolar;
  int16_t bipolar;
  uint8_t flags;
};

const uint16_t kBlockSize = 16;

struct FrequencyRatio {
  uint32_t p;
  uint32_t q;
};

class Generator {
 public:
  Generator() { }
  ~Generator() { }
  
  void Init();

  void set_range(GeneratorRange range) {
    ClearFilterState();
    range_ = range;
    clock_divider_ =
      /* harmonic oscillator is sampled at 24kHz */
      feature_mode_ == FEAT_MODE_HARMONIC ? 2 :
      range_ == GENERATOR_RANGE_LOW ? 4 : 1;
  }
  
  void set_mode(GeneratorMode mode) {
    mode_ = mode;
    if (mode_ == GENERATOR_MODE_LOOPING) {
      running_ = true;
    }
  }
  
  void set_pitch_high_range(int16_t pitch, int16_t fm) {
    if (sync_) {
      ComputeFrequencyRatio(pitch);
    }
    pitch_ = pitch + (12 << 7) + fm;
  }

  void set_pitch(int16_t pitch, int16_t fm) {
    if (sync_) {
      ComputeFrequencyRatio(pitch);
    }

    pitch += (12 << 7) - (60 << 7) * static_cast<int16_t>(range_);
    if (range_ == GENERATOR_RANGE_LOW) {
      pitch -= (12 << 7);  // One extra octave of super LF stuff!
    }
    pitch_ = pitch + fm;
  }
  
  void set_shape(int16_t shape) {
    shape_ = shape;
  }

  void set_slope(int16_t slope) {
#ifndef WAVETABLE_HACK
    if (range_ == GENERATOR_RANGE_HIGH &&
        feature_mode_ != FEAT_MODE_HARMONIC) {
      CONSTRAIN(slope, -32512, 32512);
    }
#endif  // WAVETABLE_HACK
    slope_ = slope;
  }

  void set_smoothness(int16_t smoothness) {
    smoothness_ = smoothness;
  }
  
  void set_frequency_ratio(FrequencyRatio ratio) {
    frequency_ratio_ = ratio;
  }
  
  void set_waveshaper_antialiasing(bool antialiasing) {
    antialiasing_ = antialiasing;
  }
  
  void set_sync(bool sync) {
    if (!sync_ && sync) {
      pattern_predictor_.Init();
    }
    sync_ = sync;
    sync_edges_counter_ = 0;
  }

  void set_pulse_width(uint16_t pw) {
    pulse_width_ = pw;
  }

  inline GeneratorMode mode() const { return mode_; }
  inline GeneratorRange range() const { return range_; }
  inline bool sync() const { return sync_; }
  
  inline GeneratorSample Process(uint8_t control) {
    input_buffer_.Overwrite(control);
    return output_buffer_.ImmediateRead();
  }
  
  inline bool writable_block() const {
    return output_buffer_.writable() >= kBlockSize;
  }
  
  inline bool FillBufferSafe() {
    if (!writable_block()) {
      return false;
    } else {
      FillBuffer();
      return true;
    }
  }

  void FillBuffer();

  uint32_t clock_divider() const {
    return clock_divider_;
  }

  enum FeatureMode {
    FEAT_MODE_FUNCTION,
    FEAT_MODE_HARMONIC,
    FEAT_MODE_RANDOM,
  };

  FeatureMode feature_mode_;

 private:
  // There are two versions of the rendering code, one optimized for audio, with
  // band-limiting.
  void FillBufferAudioRate();
  void FillBufferControlRate();
  void FillBufferWavetable();
  template<GeneratorMode mode> void FillBufferHarmonic();
  void FillBufferRandom();
  int32_t ComputeAntialiasAttenuation(
        int16_t pitch,
        int16_t slope,
        int16_t shape,
        int16_t smoothness);
  
  inline void ClearFilterState() {
    uni_lp_state_[0] = uni_lp_state_[1] = 0;
    bi_lp_state_[0] = bi_lp_state_[1] = 0;
  }

  int32_t ComputePhaseIncrement(int16_t pitch);
  int16_t ComputePitch(int32_t phase_increment);
  int32_t ComputeCutoffFrequency(int16_t pitch, int16_t smoothness);
  void ComputeFrequencyRatio(int16_t pitch);

  stmlib::RingBuffer<uint8_t, kBlockSize * 2> input_buffer_;
  stmlib::RingBuffer<GeneratorSample, kBlockSize * 2> output_buffer_;
   
  GeneratorMode mode_;
  GeneratorRange range_;
  GeneratorSample previous_sample_;
  
  uint32_t clock_divider_;
  
  int16_t pitch_;
  int16_t previous_pitch_;
  int16_t shape_;
  int16_t slope_;
  int32_t smoothed_slope_;
  int16_t smoothness_;
  bool antialiasing_;
  uint16_t final_gain_;
  
  uint32_t phase_;
  int32_t phase_increment_;
  uint32_t sub_phase_;
  uint16_t x_;
  uint16_t y_;
  uint16_t z_;
  bool wrap_;

  uint16_t pulse_width_;
  uint32_t divided_phase_;
  uint32_t divider_;
  uint32_t divider_counter_;
  uint32_t delayed_phase_;
  int32_t delayed_phase_increment_;
  uint32_t delay_;
  uint32_t delay_counter_;
  int16_t previous_slope_;
  uint32_t delayed_threshold_;

  uint16_t current_value_[2];
  uint16_t next_value_[2];
  bool walk_direction_[2];
  uint16_t value_[2];

  int16_t old_slope_;
  int16_t old_pitch_;

  bool sync_;
  FrequencyRatio frequency_ratio_;
  
  // Time measurement and clock divider for PLL mode.
  uint32_t sync_counter_;
  uint32_t sync_edges_counter_;
  uint32_t local_osc_phase_;
  int32_t local_osc_phase_increment_;
  int32_t target_phase_increment_;
  uint32_t eor_counter_;

  stmlib::PatternPredictor<32, 8> pattern_predictor_;
  
  int64_t uni_lp_state_[2];
  int64_t bi_lp_state_[2];

  bool running_;
  bool previous_freeze_;
  bool previous_clock_;

  static const FrequencyRatio frequency_ratios_[];
  static const int16_t num_frequency_ratios_;

  static const uint8_t kNumHarmonics = 16;
  static const uint8_t kNumHarmonicsPowers = 12;

  uint16_t envelope_[kNumHarmonics];
  uint16_t envelope_increment_[kNumHarmonics];
  uint8_t harm_permut_[kNumHarmonics];

  void RandomizeDelay();
  void RandomizeDivider();
  void RandomizeHarmonicPhase();
  void RandomizeHarmonicDistribution();

  DISALLOW_COPY_AND_ASSIGN(Generator);
};

}  // namespace tides_parasites

#endif  // TIDES_GENERATOR_H_
