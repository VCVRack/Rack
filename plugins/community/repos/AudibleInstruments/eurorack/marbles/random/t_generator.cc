// Copyright 2015 Olivier Gillet.
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
// Generator for the T outputs.

#include "marbles/random/t_generator.h"

#include <algorithm>

#include "stmlib/dsp/units.h"

#include "marbles/resources.h"

namespace marbles {

using namespace std;
using namespace stmlib;

/* static */
DividerPattern TGenerator::divider_patterns[kNumDividerPatterns] = {
  { { { 1, 1 }, { 1, 1 } }, 1 },
  { { { 1, 1 }, { 2, 1 } }, 1 },
  { { { 1, 2 }, { 1, 1 } }, 2 },
  { { { 1, 1 }, { 4, 1 } }, 1 },
  { { { 1, 2 }, { 2, 1 } }, 2 },
  { { { 1, 1 }, { 3, 2 } }, 2 },
  { { { 1, 4 }, { 4, 1 } }, 4 },
  { { { 1, 4 }, { 2, 1 } }, 4 },
  { { { 1, 2 }, { 3, 2 } }, 2 },
  { { { 1, 1 }, { 8, 1 } }, 1 },
  { { { 1, 1 }, { 3, 1 } }, 1 },
  { { { 1, 3 }, { 1, 1 } }, 3 },
  { { { 1, 1 }, { 5, 4 } }, 4 },
  { { { 1, 2 }, { 5, 4 } }, 4 },
  { { { 1, 1 }, { 6, 1 } }, 1 },
  { { { 1, 3 }, { 2, 1 } }, 3 },
  { { { 1, 1 }, { 16, 1 } }, 1 },
};

/* static */
DividerPattern TGenerator::fixed_divider_patterns[kNumDividerPatterns] = {
  { { { 8, 1 }, { 1, 8 } }, 8 },
  { { { 6, 1 }, { 1, 6 } }, 6 },
  { { { 4, 1 }, { 1, 4 } }, 4 },
  { { { 3, 1 }, { 1, 3 } }, 3 },
  { { { 2, 1 }, { 1, 2 } }, 2 },
  { { { 3, 2 }, { 2, 3 } }, 6 },
  { { { 4, 3 }, { 3, 4 } }, 12 },
  { { { 5, 4 }, { 4, 5 } }, 20 },

  { { { 1, 1 }, { 1, 1 } }, 1 },

  { { { 4, 5 }, { 5, 4 } }, 20 },
  { { { 3, 4 }, { 4, 3 } }, 12 },
  { { { 2, 2 }, { 3, 2 } }, 6 },
  { { { 1, 2 }, { 2, 1 } }, 2 },
  { { { 1, 3 }, { 3, 1 } }, 3 },
  { { { 1, 4 }, { 4, 1 } }, 4 },
  { { { 1, 6 }, { 6, 1 } }, 6 },
  { { { 1, 8 }, { 8, 1 } }, 8 },
};

/* static */
Ratio TGenerator::input_divider_ratios[kNumInputDividerRatios] = {
  { 1, 4 },
  { 1, 3 },
  { 1, 2 },
  { 2, 3 },
  { 1, 1 },
  { 3, 2 },
  { 2, 1 },
  { 3, 1 },
  { 4, 1 },
};

/* static */
uint8_t TGenerator::drum_patterns[kNumDrumPatterns][kDrumPatternSize] = {
  { 1, 0, 0, 0, 2, 0, 0, 0 },
  { 0, 0, 1, 0, 2, 0, 0, 0 },

  { 1, 0, 1, 0, 2, 0, 0, 0 },
  { 0, 0, 1, 0, 2, 0, 0, 2 },

  { 1, 0, 1, 0, 2, 0, 1, 0 },
  { 0, 2, 1, 0, 2, 0, 0, 2 },

  { 1, 0, 0, 0, 2, 0, 1, 0 },
  { 0, 2, 1, 0, 2, 0, 1, 2 },

  { 1, 0, 0, 1, 2, 0, 0, 0 },
  { 0, 2, 1, 1, 2, 0, 1, 2 },

  { 1, 0, 0, 1, 2, 0, 1, 0 },
  { 0, 2, 1, 1, 2, 2, 1, 2 },

  { 1, 0, 0, 1, 2, 0, 1, 2 },
  { 0, 2, 0, 1, 2, 0, 1, 2 },

  { 1, 0, 1, 1, 2, 0, 1, 2 },
  { 2, 0, 1, 2, 0, 1, 2, 0 },

  { 1, 2, 1, 1, 2, 0, 1, 2 },
  { 2, 0, 1, 2, 0, 1, 2, 2 }
};

void TGenerator::Init(RandomStream* random_stream, float sr) {
  one_hertz_ = 1.0f / static_cast<float>(sr);
  model_ = T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI;
  range_ = T_GENERATOR_RANGE_1X;

  rate_ = 0.0f;
  bias_ = 0.5f;
  jitter_ = 0.0f;
  pulse_width_mean_ = 0.0f;
  pulse_width_std_ = 0.0f;

  master_phase_ = 0.0f;
  jitter_multiplier_ = 1.0f;
  phase_difference_ = 0.0f;
  previous_external_ramp_value_ = 0.0f;

  divider_pattern_length_ = 0;
  fill(&streak_counter_[0], &streak_counter_[kMarkovHistorySize], 0);
  fill(&markov_history_[0], &markov_history_[kMarkovHistorySize], 0);
  markov_history_ptr_ = 0;
  drum_pattern_step_ = 0;
  drum_pattern_index_ = 0;

  sequence_.Init(random_stream);
  ramp_divider_.Init();
  ramp_extractor_.Init(1000.0f / sr);
  ramp_generator_.Init();
  for (size_t i = 0; i < kNumTChannels; ++i) {
    slave_ramp_[i].Init();
  }
  bias_quantizer_.Init();
  rate_quantizer_.Init();
  
  use_external_clock_ = false;
}

int TGenerator::GenerateComplementaryBernoulli(const RandomVector& x) {
  int bitmask = 0;
  for (size_t i = 0; i < kNumTChannels; ++i) {
    if ((x.variables.u[i >> 1] > bias_) ^ (i & 1)) {
      bitmask |= 1 << i;
    }
  }
  return bitmask;
}

int TGenerator::GenerateIndependentBernoulli(const RandomVector& x) {
  int bitmask = 0;
  for (size_t i = 0; i < kNumTChannels; ++i) {
    if ((x.variables.u[i] > bias_) ^ (i & 1)) {
      bitmask |= 1 << i;
    }
  }
  return bitmask;
}

int TGenerator::GenerateThreeStates(const RandomVector& x) {
  int bitmask = 0;
  float p_none = 0.75f - fabs(bias_ - 0.5f);
  float threshold = p_none + (1.0f - p_none) * (0.25f + (bias_ * 0.5f));
  
  for (size_t i = 0; i < kNumTChannels; ++i) {
    float u = x.variables.u[i >> 1];
    if (u > p_none && ((u > threshold) ^ (i & 1))) {
      bitmask |= 1 << i;
    }
  }
  return bitmask;
}

int TGenerator::GenerateDrums(const RandomVector& x) {
  ++drum_pattern_step_;
  if (drum_pattern_step_ >= kDrumPatternSize) {
    drum_pattern_step_ = 0;
    float u = x.variables.u[0] * 2.0f * fabs(bias_ - 0.5f);
    drum_pattern_index_ = static_cast<int32_t>(kNumDrumPatterns * u);
    if (bias_ <= 0.5f) {
      drum_pattern_index_ -= drum_pattern_index_ % 2;
    }
  }
  return drum_patterns[drum_pattern_index_][drum_pattern_step_];
}

int TGenerator::GenerateMarkov(const RandomVector& x) {
  int bitmask = 0;
  float b = 1.5f * bias_ - 0.5f;
  markov_history_[markov_history_ptr_] = 0;
  const int32_t p = markov_history_ptr_;
  for (size_t i = 0; i < kNumTChannels; ++i) {
    int32_t mask = 1 << i;
    // 4 rules:
    // * We favor repeating what we played 8 ticks ago.
    // * We do not favor pulses appearing on both channels.
    // * We favor sparse patterns (no consecutive hits).
    // * We favor patterns in which one channel "echoes" what the other
    //   channel played 4 ticks before.
    bool periodic = markov_history_[(p + 8) % kMarkovHistorySize] & mask;
    bool simultaneous = markov_history_[(p + 8) % kMarkovHistorySize] & ~mask;
    bool dense = markov_history_[(p + 1) % kMarkovHistorySize] & mask;
    bool alternate = markov_history_[(p + 4) % kMarkovHistorySize] & ~mask;

    float logit = -1.5f;
    logit += streak_counter_[i] > 24 ? 10.0f : 0.0f;
    logit += 8.0f * fabs(b) * (periodic ? b : -b);
    logit -= 2.0f * (simultaneous ? b : -b);
    logit -= 1.0f * (dense ? b : 0.0f);
    logit += 1.0f * (alternate ? b : 0.0f);
    CONSTRAIN(logit, -10.0f, 10.0f);
    float probability = lut_logit[static_cast<int>(logit * 12.8f + 128.0f)];
    bool state = x.variables.u[i] < probability;
    
    if (sequence_.deja_vu() >= x.variables.p) {
      state = markov_history_[(p + sequence_.length()) % kMarkovHistorySize] & mask;
    }
    if (state) {
      bitmask |= mask;
      streak_counter_[i] = 0;
    } else {
      ++streak_counter_[i];
    }
  }
  markov_history_[p] |= bitmask;
  markov_history_ptr_ = (p + kMarkovHistorySize - 1) % kMarkovHistorySize;
  return bitmask;
}

void TGenerator::ScheduleOutputPulses(const RandomVector& x, int bitmask) {
  for (size_t i = 0; i < kNumTChannels; ++i) {
    slave_ramp_[i].Init(
        bitmask & 1,
        RandomPulseWidth(i, x.variables.pulse_width[i]),
        0.5f);
    bitmask >>= 1;
  }
}

void TGenerator::ConfigureSlaveRamps(const RandomVector& x) {
  switch (model_) {
    // Generate a bitmask that will describe which outputs are active
    // at this clock tick. Use this bitmask to actually schedule pulses on the
    // outputs.
    case T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI:
      ScheduleOutputPulses(x, GenerateComplementaryBernoulli(x));
      break;
    
    case T_GENERATOR_MODEL_INDEPENDENT_BERNOULLI:
      ScheduleOutputPulses(x, GenerateIndependentBernoulli(x));
      break;

    case T_GENERATOR_MODEL_THREE_STATES:
      ScheduleOutputPulses(x, GenerateThreeStates(x));
      break;
    
    case T_GENERATOR_MODEL_DRUMS:
      ScheduleOutputPulses(x, GenerateDrums(x));
      break;
    
    case T_GENERATOR_MODEL_MARKOV:
      ScheduleOutputPulses(x, GenerateMarkov(x));
      break;
    
    case T_GENERATOR_MODEL_CLUSTERS:
    case T_GENERATOR_MODEL_DIVIDER:
      --divider_pattern_length_;
      if (divider_pattern_length_ <= 0) {
        DividerPattern pattern;
        if (model_ == T_GENERATOR_MODEL_DIVIDER) {
          pattern = bias_quantizer_.Lookup(
              fixed_divider_patterns,
              bias_,
              kNumDividerPatterns);
        } else {
          float strength = fabs(bias_ - 0.5f) * 2.0f;
          float u = x.variables.u[0];
          u *= (u + strength * strength * (1.0f - u));
          u *= strength;
          pattern = divider_patterns[static_cast<size_t>(
              u * kNumDividerPatterns)];
          if (bias_ < 0.5f) {
            for (size_t i = 0; i < kNumTChannels / 2; ++i) {
              swap(pattern.ratios[i], pattern.ratios[kNumTChannels - 1 - i]);
            }
          }
        }
        for (size_t i = 0; i < kNumTChannels; ++i) {
          slave_ramp_[i].Init(
              pattern.length,
              pattern.ratios[i],
              RandomPulseWidth(i, x.variables.pulse_width[i]));
        }
        divider_pattern_length_ = pattern.length;
      }
      break;
  }
}

void TGenerator::Process(
    bool use_external_clock,
    const GateFlags* external_clock,
    Ramps ramps,
    bool* gate,
    size_t size) {
  
  float internal_frequency;
  if (use_external_clock) {
    if (!use_external_clock_) {
      ramp_extractor_.Reset();
    }
    
    Ratio ratio = rate_quantizer_.Lookup(
        input_divider_ratios, 
        1.05f * rate_ / 96.0f + 0.5f,
        kNumInputDividerRatios);
    if (range_ == T_GENERATOR_RANGE_0_25X) {
      ratio.q *= 4;
    } else if (range_ == T_GENERATOR_RANGE_4X) {
      ratio.p *= 4;
    }
    ratio.Simplify<2>();
    ramp_extractor_.Process(ratio, true, external_clock, ramps.external, size);
    internal_frequency = 0.0f;
  } else {
    float rate = 2.0f;
    if (range_ == T_GENERATOR_RANGE_4X) {
      rate = 8.0f;
    } else if (range_ == T_GENERATOR_RANGE_0_25X) {
      rate = 0.5f;
    }
    internal_frequency = rate * one_hertz_ * SemitonesToRatio(rate_);
  }
  
  use_external_clock_ = use_external_clock;
  
  while (size--) {
    float frequency = use_external_clock
        ? *ramps.external - previous_external_ramp_value_
        : internal_frequency;
    frequency += frequency < 0.0f ? 1.0f : 0.0f;

    float jittery_frequency = frequency * jitter_multiplier_;
    master_phase_ += jittery_frequency;
    phase_difference_ += frequency - jittery_frequency;
    
    if (master_phase_ > 1.0f) {
      master_phase_ -= 1.0f;
      
      RandomVector random_vector;
      sequence_.NextVector(
          random_vector.x,
          sizeof(random_vector.x) / sizeof(float));
      
      float jitter_amount = jitter_ * jitter_ * jitter_ * jitter_ * 36.0f;
      float x = FastBetaDistributionSample(random_vector.variables.jitter);
      float multiplier = SemitonesToRatio((x * 2.0f - 1.0f) * jitter_amount);
      
      // This step is crucial in making sure that the jittered clock does not
      // deviate too much from the master clock. The larger the phase difference
      // difference between the two, the more likely the jittery clock will
      // speed up or down to catch up with the straight clock.
      multiplier *= phase_difference_ > 0.0f
            ? 1.0f + phase_difference_
            : 1.0f / (1.0f - phase_difference_);
      
      jitter_multiplier_ = multiplier;
      ConfigureSlaveRamps(random_vector);
    }
    
    if (internal_frequency) {
      *ramps.external = master_phase_;
    }
    
    previous_external_ramp_value_ = *ramps.external;
    ramps.external++;
    *ramps.master++ = master_phase_;
    for (size_t j = 0; j < kNumTChannels; ++j) {
      slave_ramp_[j].Process(
          frequency * jitter_multiplier_,
          ramps.slave[j],
          gate);
      ramps.slave[j]++;
      gate++;
    }
  }
}

}  // namespace marbles
