// Copyright 2017 Olivier Gillet.
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
// Multi-stage envelope

#include "stages/segment_generator.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/dsp/units.h"

#include <cassert>
#include <cmath>
#include <algorithm>

#include "stages/resources.h"

namespace stages {

using namespace stmlib;
using namespace std;
using namespace segment;

// Duration of the "tooth" in the output when a trigger is received while the
// output is high.
const int kRetrigDelaySamples = 32;

void SegmentGenerator::Init() {
  process_fn_ = &SegmentGenerator::ProcessMultiSegment;
  
  phase_ = 0.0f;

  zero_ = 0.0f;
  half_ = 0.5f;
  one_ = 1.0f;

  start_ = 0.0f;
  value_ = 0.0f;
  lp_ = 0.0f;
  
  monitored_segment_ = 0;
  active_segment_ = 0;
  retrig_delay_ = 0;
  primary_ = 0;

  Segment s;
  s.start = &zero_;
  s.end = &zero_;
  s.time = &zero_;
  s.curve = &half_;
  s.portamento = &zero_;
  s.phase = NULL;
  s.if_rising = 0;
  s.if_falling = 0;
  s.if_complete = 0;
  fill(&segments_[0], &segments_[kMaxNumSegments + 1], s);
  
  Parameters p;
  p.primary = 0.0f;
  p.secondary = 0.0f;
  fill(&parameters_[0], &parameters_[kMaxNumSegments], p);

  ramp_division_quantizer_.Init();
  delay_line_.Init();
  
  num_segments_ = 0;
}

void SegmentGenerator::SetSampleRate(float sample_rate) {
  sample_rate_ = sample_rate;

  ramp_extractor_.Init(
      sample_rate_,
      1000.0f / sample_rate_);
}

inline float SegmentGenerator::WarpPhase(float t, float curve) const {
  curve -= 0.5f;
  const bool flip = curve < 0.0f;
  if (flip) {
    t = 1.0f - t;
  }
  const float a = 128.0f * curve * curve;
  t = (1.0f + a) * t / (1.0f + a * t);
  if (flip) {
    t = 1.0f - t;
  }
  return t;
}

inline float SegmentGenerator::RateToFrequency(float rate) const {
  int32_t i = static_cast<int32_t>(rate * 2048.0f);
  CONSTRAIN(i, 0, LUT_ENV_FREQUENCY_SIZE);
  return lut_env_frequency[i] * (31250.0f/sample_rate_);
}

inline float SegmentGenerator::PortamentoRateToLPCoefficient(float rate) const {
  int32_t i = static_cast<int32_t>(rate * 512.0f);
  return lut_portamento_coefficient[i] * (31250.0f/sample_rate_);
}

void SegmentGenerator::ProcessMultiSegment(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  float phase = phase_;
  float start = start_;
  float lp = lp_;
  float value = value_;
  
  while (size--) {
    const Segment& segment = segments_[active_segment_];

    if (segment.time) {
      phase += RateToFrequency(*segment.time);
    }
    
    bool complete = phase >= 1.0f;
    if (complete) {
      phase = 1.0f;
    }
    value = Crossfade(
        start,
        *segment.end,
        WarpPhase(segment.phase ? *segment.phase : phase, *segment.curve));
  
    ONE_POLE(lp, value, PortamentoRateToLPCoefficient(*segment.portamento));
  
    // Decide what to do next.
    int go_to_segment = -1;
    if (*gate_flags & GATE_FLAG_RISING) {
      go_to_segment = segment.if_rising;
    } else if (*gate_flags & GATE_FLAG_FALLING) {
      go_to_segment = segment.if_falling;
    } else if (complete) {
      go_to_segment = segment.if_complete;
    }
  
    if (go_to_segment != -1) {
      phase = 0.0f;
      const Segment& destination = segments_[go_to_segment];
      start = destination.start
          ? *destination.start
          : (go_to_segment == active_segment_ ? start : value);
      active_segment_ = go_to_segment;
    }
    
    out->value = lp;
    out->phase = phase;
    out->segment = active_segment_;
    ++gate_flags;
    ++out;
  }
  phase_ = phase;
  start_ = start;
  lp_ = lp;
  value_ = value;
}

void SegmentGenerator::ProcessDecayEnvelope(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  const float frequency = RateToFrequency(parameters_[0].primary);
  while (size--) {
    if (*gate_flags & GATE_FLAG_RISING) {
      phase_ = 0.0f;
      active_segment_ = 0;
    }
  
    phase_ += frequency;
    if (phase_ >= 1.0f) {
      phase_ = 1.0f;
      active_segment_ = 1;
    }
    lp_ = value_ = 1.0f - WarpPhase(phase_, parameters_[0].secondary);
    out->value = lp_;
    out->phase = phase_;
    out->segment = active_segment_;
    ++gate_flags;
    ++out;
  }
}

void SegmentGenerator::ProcessTimedPulseGenerator(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  const float frequency = RateToFrequency(parameters_[0].secondary);
  
  ParameterInterpolator primary(&primary_, parameters_[0].primary, size);
  while (size--) {
    if (*gate_flags & GATE_FLAG_RISING) {
      retrig_delay_ = active_segment_ == 0 ? kRetrigDelaySamples : 0;
      phase_ = 0.0f;
      active_segment_ = 0;
    }
    if (retrig_delay_) {
      --retrig_delay_;
    }
    phase_ += frequency;
    if (phase_ >= 1.0f) {
      phase_ = 1.0f;
      active_segment_ = 1;
    }
  
    const float p = primary.Next();
    lp_ = value_ = active_segment_ == 0 && !retrig_delay_ ? p : 0.0f;
    out->value = lp_;
    out->phase = phase_;
    out->segment = active_segment_;
    ++gate_flags;
    ++out;
  }
}

void SegmentGenerator::ProcessGateGenerator(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  ParameterInterpolator primary(&primary_, parameters_[0].primary, size);
  while (size--) {
    active_segment_ = *gate_flags & GATE_FLAG_HIGH ? 0 : 1;

    const float p = primary.Next();
    lp_ = value_ = active_segment_ == 0 ? p : 0.0f;
    out->value = lp_;
    out->phase = 0.5f;
    out->segment = active_segment_;
    ++gate_flags;
    ++out;
  }
}

void SegmentGenerator::ProcessSampleAndHold(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  const float coefficient = PortamentoRateToLPCoefficient(
      parameters_[0].secondary);
  ParameterInterpolator primary(&primary_, parameters_[0].primary, size);

  while (size--) {
    const float p = primary.Next();
    if (*gate_flags & GATE_FLAG_RISING) {
      value_ = p;
    }
    active_segment_ = *gate_flags & GATE_FLAG_HIGH ? 0 : 1;

    ONE_POLE(lp_, value_, coefficient);
    out->value = lp_;
    out->phase = 0.5f;
    out->segment = active_segment_;
    ++gate_flags;
    ++out;
  }
}

void SegmentGenerator::ProcessClockedSampleAndHold(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  const float frequency = RateToFrequency(parameters_[0].secondary);
  ParameterInterpolator primary(&primary_, parameters_[0].primary, size);
  while (size--) {
    phase_ += frequency;
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;

      const float reset_time = phase_ / frequency;
      value_ = primary.subsample(1.0f - reset_time);
    }
    primary.Next();
    active_segment_ = phase_ < 0.5f ? 0 : 1;
    out->value = value_;
    out->phase = phase_;
    out->segment = active_segment_;
    ++out;
  }
}

Ratio divider_ratios[] = {
  { 0.249999f, 4 },
  { 0.333333f, 3 },
  { 0.499999f, 2 },
  { 0.999999f, 1 },
  { 1.999999f, 1 },
  { 2.999999f, 1 },
  { 3.999999f, 1 },
};

void SegmentGenerator::ProcessTapLFO(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  float ramp[12];
  Ratio r = ramp_division_quantizer_.Lookup(
      divider_ratios, parameters_[0].primary * 1.03f, 7);
  ramp_extractor_.Process(r, gate_flags, ramp, size);
  for (size_t i = 0; i < size; ++i) {
    out[i].phase = ramp[i];
  }
  ShapeLFO(parameters_[0].secondary, out, size);
  active_segment_ = out[size - 1].segment;
}

void SegmentGenerator::ProcessFreeRunningLFO(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  float f = 96.0f * (parameters_[0].primary - 0.5f);
  CONSTRAIN(f, -128.0f, 127.0f);
  const float frequency = SemitonesToRatio(f) * 2.0439497f / sample_rate_;

  active_segment_ = 0;
  for (size_t i = 0; i < size; ++i) {
    phase_ += frequency;
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
    }
    out[i].phase = phase_;
  }
  ShapeLFO(parameters_[0].secondary, out, size);
  active_segment_ = out[size - 1].segment;
}

void SegmentGenerator::ProcessDelay(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  const float max_delay = static_cast<float>(kMaxDelay - 1);
  
  float delay_time = SemitonesToRatio(
      2.0f * (parameters_[0].secondary - 0.5f) * 36.0f) * 0.5f * sample_rate_;
  float clock_frequency = 1.0f;
  float delay_frequency = 1.0f / delay_time;
  
  if (delay_time >= max_delay) {
    clock_frequency = max_delay * delay_frequency;
    delay_time = max_delay;
  }
  ParameterInterpolator primary(&primary_, parameters_[0].primary, size);
  
  active_segment_ = 0;
  while (size--) {
    phase_ += clock_frequency;
    ONE_POLE(lp_, primary.Next(), clock_frequency);
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
      delay_line_.Write(lp_);
    }
    
    aux_ += delay_frequency;
    if (aux_ >= 1.0f) {
      aux_ -= 1.0f;
    }
    active_segment_ = aux_ < 0.5f ? 0 : 1;
    
    ONE_POLE(
        value_,
        delay_line_.Read(delay_time - phase_),
        clock_frequency);
    out->value = value_;
    out->phase = aux_;
    out->segment = active_segment_;
    ++out;
  }
}

void SegmentGenerator::ProcessPortamento(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  const float coefficient = PortamentoRateToLPCoefficient(
      parameters_[0].secondary);
  ParameterInterpolator primary(&primary_, parameters_[0].primary, size);
  
  active_segment_ = 0;
  while (size--) {
    value_ = primary.Next();
    ONE_POLE(lp_, value_, coefficient);
    out->value = lp_;
    out->phase = 0.5f;
    out->segment = active_segment_;
    ++out;
  }
}

void SegmentGenerator::ProcessZero(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  
  value_ = 0.0f;
  active_segment_ = 1;
  while (size--) {
    out->value = 0.0f;
    out->phase = 0.5f;
    out->segment = 1;
    ++out;
  }
}

void SegmentGenerator::ProcessSlave(
    const GateFlags* gate_flags, SegmentGenerator::Output* out, size_t size) {
  while (size--) {
    active_segment_ = out->segment == monitored_segment_ ? 0 : 1;
    out->value = active_segment_ ? 0.0f : 1.0f - out->phase;
    ++out;
  }
}

/* static */
void SegmentGenerator::ShapeLFO(
    float shape,
    SegmentGenerator::Output* in_out,
    size_t size) {
  shape -= 0.5f;
  shape = 2.0f + 9.999999f * shape / (1.0f + 3.0f * fabs(shape));
  
  const float slope = min(shape * 0.5f, 0.5f);
  const float plateau_width = max(shape - 3.0f, 0.0f);
  const float sine_amount = max(
      shape < 2.0f ? shape - 1.0f : 3.0f - shape, 0.0f);
  
  const float slope_up = 1.0f / slope;
  const float slope_down = 1.0f / (1.0f - slope);
  const float plateau = 0.5f * (1.0f - plateau_width);
  const float normalization = 1.0f / plateau;
  const float phase_shift = plateau_width * 0.25f;
  
  while (size--) {
    float phase = in_out->phase - phase_shift;
    if (phase < 0.0f) {
      phase += 1.0f;
    }
    float triangle = phase < slope
        ? slope_up * phase
        : 1.0f - (phase - slope) * slope_down;
    triangle -= 0.5f;
    CONSTRAIN(triangle, -plateau, plateau);
    triangle = triangle * normalization;
    float sine = InterpolateWrap(lut_sine, phase + 0.75f, 1024.0f);
    in_out->value = 0.5f * Crossfade(triangle, sine, sine_amount) + 0.5f;
    in_out->segment = phase < 0.5f ? 0 : 1;
    ++in_out;
  }
}

void SegmentGenerator::Configure(
    bool has_trigger,
    const Configuration* segment_configuration,
    int num_segments) {
  if (num_segments == 1) {
    ConfigureSingleSegment(has_trigger, segment_configuration[0]);
    return;
  }
  num_segments_ = num_segments;
  
  // assert(has_trigger);
  
  process_fn_ = &SegmentGenerator::ProcessMultiSegment;
  
  // A first pass to collect loop points, and check for STEP segments.
  int loop_start = -1;
  int loop_end = -1;
  bool has_step_segments = false;
  int last_segment = num_segments - 1;
  int first_ramp_segment = -1;
  
  for (int i = 0; i <= last_segment; ++i) {
    has_step_segments = has_step_segments || \
        segment_configuration[i].type == TYPE_STEP;
    if (segment_configuration[i].loop) {
      if (loop_start == -1) {
        loop_start = i;
      }
      loop_end = i;
    }
    if (segment_configuration[i].type == TYPE_RAMP) {
      if (first_ramp_segment == -1) {
        first_ramp_segment = i;
      }
    }
  }
  
  // Check if there are step segments inside the loop.
  bool has_step_segments_inside_loop = false;
  if (loop_start != -1) {
    for (int i = loop_start; i <= loop_end; ++i) {
      if (segment_configuration[i].type == TYPE_STEP) {
        has_step_segments_inside_loop = true;
        break;
      }
    }
  }
  
  for (int i = 0; i <= last_segment; ++i) {
    Segment* s = &segments_[i];
    if (segment_configuration[i].type == TYPE_RAMP) {
      s->start = (num_segments == 1) ? &one_ : NULL;
      s->time = &parameters_[i].primary;
      s->curve = &parameters_[i].secondary;
      s->portamento = &zero_;
      s->phase = NULL;
      
      if (i == last_segment) {
        s->end = &zero_;
      } else if (segment_configuration[i + 1].type != TYPE_RAMP) {
        s->end = &parameters_[i + 1].primary;
      } else if (i == first_ramp_segment) {
        s->end = &one_;
      } else {
        s->end = &parameters_[i].secondary;
        // The whole "reuse the curve from other segment" thing
        // is a bit too complicated...
        //
        // for (int j = i + 1; j <= last_segment; ++j) {
        //   if (segment_configuration[j].type == TYPE_RAMP) {
        //     if (j == last_segment ||
        //         segment_configuration[j + 1].type != TYPE_RAMP) {
        //       s->curve = &parameters_[j].secondary;
        //       break;
        //     }
        //   }
        // }
        s->curve = &half_;
      }
    } else {
      s->start = s->end = &parameters_[i].primary;
      s->curve = &half_;
      if (segment_configuration[i].type == TYPE_STEP) {
        s->portamento = &parameters_[i].secondary;
        s->time = NULL;
        // Sample if there is a loop of length 1 on this segment. Otherwise
        // track.
        s->phase = i == loop_start && i == loop_end ? &zero_ : &one_;
      } else {
        s->portamento = &zero_;
        // Hold if there's a loop of length 1 of this segment. Otherwise, use
        // the programmed time.
        s->time = i == loop_start && i == loop_end
            ? NULL : &parameters_[i].secondary;
        s->phase = &one_;  // Track the changes on the slider.
      }
    }

    s->if_complete = i == loop_end ? loop_start : i + 1;
    s->if_falling = loop_end == -1 || loop_end == last_segment || has_step_segments ? -1 : loop_end + 1;
    s->if_rising = 0;
    
    if (has_step_segments) {
      if (!has_step_segments_inside_loop && i >= loop_start && i <= loop_end) {
        s->if_rising = (loop_end + 1) % num_segments;
      } else {
        // Just go to the next stage.
        // s->if_rising = (i == loop_end) ? loop_start : (i + 1) % num_segments;
        
        // Find the next STEP segment.
        bool follow_loop = loop_end != -1;
        int next_step = i;
        while (segment_configuration[next_step].type != TYPE_STEP) {
          ++next_step;
          if (follow_loop && next_step == loop_end + 1) {
            next_step = loop_start;
            follow_loop = false;
          }
          if (next_step >= num_segments) {
            next_step = num_segments - 1;
            break;
          }
        }
        s->if_rising = next_step == loop_end
            ? loop_start
            : (next_step + 1) % num_segments;
      }
    }
  }
  
  Segment* sentinel = &segments_[num_segments];
  sentinel->end = sentinel->start = segments_[num_segments - 1].end;
  sentinel->time = &zero_;
  sentinel->curve = &half_;
  sentinel->portamento = &zero_;
  sentinel->if_rising = 0;
  sentinel->if_falling = -1;
  sentinel->if_complete = loop_end == last_segment ? 0 : -1;
  
  // After changing the state of the module, we go to the sentinel.
  active_segment_ = num_segments;
}

/* static */
SegmentGenerator::ProcessFn SegmentGenerator::process_fn_table_[12] = {
  // RAMP
  &SegmentGenerator::ProcessZero,
  &SegmentGenerator::ProcessFreeRunningLFO,
  &SegmentGenerator::ProcessDecayEnvelope,
  &SegmentGenerator::ProcessTapLFO,
  
  // STEP
  &SegmentGenerator::ProcessPortamento,
  &SegmentGenerator::ProcessPortamento,
  &SegmentGenerator::ProcessSampleAndHold,
  &SegmentGenerator::ProcessSampleAndHold,
  
  // HOLD
  &SegmentGenerator::ProcessDelay,
  &SegmentGenerator::ProcessDelay,
  // &SegmentGenerator::ProcessClockedSampleAndHold,
  &SegmentGenerator::ProcessTimedPulseGenerator,
  &SegmentGenerator::ProcessGateGenerator
};

}  // namespace stages