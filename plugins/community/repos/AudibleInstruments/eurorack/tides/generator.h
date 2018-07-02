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

namespace tides {

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

const size_t kNumBlocks = 2;
const size_t kBlockSize = 16;

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
    clock_divider_ = range_ == GENERATOR_RANGE_LOW ? 4 : 1;
  }
  
  void set_mode(GeneratorMode mode) {
    mode_ = mode;
    if (mode_ == GENERATOR_MODE_LOOPING) {
      running_ = true;
    }
  }
  
  void set_pitch(int16_t pitch) {
    if (sync_) {
      ComputeFrequencyRatio(pitch);
    }
    pitch += (12 << 7) - (60 << 7) * static_cast<int16_t>(range_);
    if (range_ == GENERATOR_RANGE_LOW) {
      pitch -= (12 << 7);  // One extra octave of super LF stuff!
    }
    pitch_ = pitch;
  }
  
  void set_shape(int16_t shape) {
    shape_ = shape;
  }

  void set_slope(int16_t slope) {
    slope_ = slope;
  }

  void set_smoothness(int16_t smoothness) {
    smoothness_ = smoothness;
  }
  
  void set_frequency_ratio(FrequencyRatio ratio) {
    frequency_ratio_ = ratio;
  }
  
  void set_sync(bool sync) {
    if (!sync_ && sync) {
      pattern_predictor_.Init();
    }
    sync_ = sync;
    sync_edges_counter_ = 0;
  }
  
  inline GeneratorMode mode() const { return mode_; }
  inline GeneratorRange range() const { return range_; }
  inline bool sync() const { return sync_; }
  
  inline const GeneratorSample& Process(uint8_t control) {
    input_samples_[playback_block_][current_sample_] = control;
    const GeneratorSample& out = output_samples_[playback_block_][current_sample_];
    current_sample_ = current_sample_ + 1;
    if (current_sample_ >= kBlockSize) {
      current_sample_ = 0;
      playback_block_ = (playback_block_ + 1) % kNumBlocks;
    }
    return out;
  }
  
  inline bool writable_block() const {
    return render_block_ != playback_block_;
  }
  
  inline void Process(bool wavetableHack = false) {
    while (render_block_ != playback_block_) {
      uint8_t* in = input_samples_[render_block_];
      GeneratorSample* out = output_samples_[render_block_];
      if (!wavetableHack) {
        if (range_ == GENERATOR_RANGE_HIGH) {
          ProcessAudioRate(in, out, kBlockSize);
        } else {
          ProcessControlRate(in, out, kBlockSize);
        }
        ProcessFilterWavefolder(out, kBlockSize);
      }
      else {
        ProcessWavetable(in, out, kBlockSize);
      }
      render_block_ = (render_block_ + 1) % kNumBlocks;
    }
  }
  
  uint32_t clock_divider() const {
    return clock_divider_;
  }

 private:
  // There are two versions of the rendering code, one optimized for audio, with
  // band-limiting.
  void ProcessAudioRate(const uint8_t* in, GeneratorSample* out, size_t size);
  void ProcessControlRate(const uint8_t* in, GeneratorSample* out, size_t size);
  void ProcessWavetable(const uint8_t* in, GeneratorSample* out, size_t size);
  void ProcessFilterWavefolder(GeneratorSample* in_out, size_t size);

  int32_t ComputeAntialiasAttenuation(
        int16_t pitch,
        int16_t slope,
        int16_t shape,
        int16_t smoothness) const;
  
  inline void ClearFilterState() {
    uni_lp_state_[0] = uni_lp_state_[1] = 0;
    bi_lp_state_[0] = bi_lp_state_[1] = 0;
  }

  uint32_t ComputePhaseIncrement(int16_t pitch);
  int16_t ComputePitch(uint32_t phase_increment);
  int32_t ComputeCutoffFrequency(int16_t pitch, int16_t smoothness);
  void ComputeFrequencyRatio(int16_t pitch);
  
  inline int32_t NextIntegratedBlepSample(uint32_t t) const {
    if (t >= 65535) {
      t = 65535;
    }
    const int32_t t1 = t >> 1;
    const int32_t t2 = t1 * t1 >> 16;
    const int32_t t4 = t2 * t2 >> 16;
    return 12288 - t1 + (3 * t2 >> 1) - t4;
  }

  inline int32_t ThisIntegratedBlepSample(uint32_t t) const {
    if (t >= 65535) {
      t = 65535;
    }
    t = 65535 - t;
    const int32_t t1 = t >> 1;
    const int32_t t2 = t1 * t1 >> 16;
    const int32_t t4 = t2 * t2 >> 16;
    return 12288 - t1 + (3 * t2 >> 1) - t4;
  }
  
  GeneratorSample output_samples_[kNumBlocks][kBlockSize];
  uint8_t input_samples_[kNumBlocks][kBlockSize];
  size_t current_sample_;
  volatile size_t playback_block_;
  volatile size_t render_block_;
  
  GeneratorMode mode_;
  GeneratorRange range_;
  GeneratorSample previous_sample_;
  GeneratorSample buffer_[kBlockSize];
  
  uint32_t clock_divider_;
  
  uint16_t prescaler_;
  int16_t pitch_;
  int16_t previous_pitch_;
  int16_t shape_;
  int16_t slope_;
  int32_t smoothed_slope_;
  int16_t smoothness_;
  int16_t attenuation_;
  
  uint32_t phase_;
  uint32_t phase_increment_;
  uint16_t x_;
  uint16_t y_;
  uint16_t z_;
  bool wrap_;
  
  bool sync_;
  FrequencyRatio frequency_ratio_;
  
  // Time measurement and clock divider for PLL mode.
  uint32_t sync_counter_;
  uint32_t sync_edges_counter_;
  uint32_t local_osc_phase_;
  uint32_t local_osc_phase_increment_;
  uint32_t target_phase_increment_;
  uint32_t eor_counter_;
  
  stmlib::PatternPredictor<32, 8> pattern_predictor_;
  
  int64_t uni_lp_state_[2];
  int64_t bi_lp_state_[2];
  
  bool running_;
  
  // Polyblep status.
  int32_t next_sample_;
  bool slope_up_;
  uint32_t mid_point_;
  
  static const FrequencyRatio frequency_ratios_[];
  static const int16_t num_frequency_ratios_;
  
  DISALLOW_COPY_AND_ASSIGN(Generator);
};

}  // namespace tides

#endif  // TIDES_GENERATOR_H_
