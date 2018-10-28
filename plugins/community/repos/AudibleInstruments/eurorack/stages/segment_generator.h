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
// Multi-stage envelope.

#ifndef STAGES_SEGMENT_GENERATOR_H_
#define STAGES_SEGMENT_GENERATOR_H_

#include "stmlib/dsp/hysteresis_quantizer.h"
#include "stmlib/utils/gate_flags.h"

#include "stages/delay_line_16_bits.h"

#include "stages/ramp_extractor.h"

namespace stages {

// Each segment generator can handle up to 36 segments. That's a bit of a waste
// of RAM because the 6 generators running on a module will never have to deal
// with 36 segments each. But it was a bit too much to have a shared pool of
// pre-allocated Segments shared by all SegmentGenerators!
const int kMaxNumSegments = 36;

const size_t kMaxDelay = 768;

#define DECLARE_PROCESS_FN(X) void Process ## X \
      (const stmlib::GateFlags* gate_flags, Output* out, size_t size);

namespace segment {

// High level descriptions / parameters.
enum Type {
  TYPE_RAMP = 0,
  TYPE_STEP = 1,
  TYPE_HOLD = 2,
};

struct Configuration {
  Type type;
  bool loop;
};

struct Parameters {
  // Segment type  | Main  | Secondary
  //
  // RAMP          | Time  | Shape (or level if followed by RAMP)
  // HOLD          | Level | Time
  // STEP          | Level | Shape (portamento)
  float primary;
  float secondary;
};

}  // namespace segment

class SegmentGenerator {
 public:
  SegmentGenerator() { }
  ~SegmentGenerator() { }
  
  struct Output {
    float value;
    float phase;
    int32_t segment;
  };
  
  struct Segment {
    // Low level state.
    
    float* start;  // NULL if we should start from the current value.
    float* time;  // NULL if the segment has infinite duration.
    float* curve;
    float* portamento;
    float* end;
    float* phase;
    
    int8_t if_rising;
    int8_t if_falling;
    int8_t if_complete;
  };
  
  void Init();
  void SetSampleRate(float sample_rate);

  typedef void (SegmentGenerator::*ProcessFn)(
      const stmlib::GateFlags* gate_flags, Output* out, size_t size);
  
  bool Process(
      const stmlib::GateFlags* gate_flags, Output* out, size_t size) {
    (this->*process_fn_)(gate_flags, out, size);
    return active_segment_ == 0;
  }
  
  void Configure(
      bool has_trigger,
      const segment::Configuration* segment_configuration,
      int num_segments);

  inline void ConfigureSingleSegment(
      bool has_trigger,
      segment::Configuration segment_configuration) {
    int i = has_trigger ? 2 : 0;
    i += segment_configuration.loop ? 1 : 0;
    i += int(segment_configuration.type) * 4;
    process_fn_ = process_fn_table_[i];
    num_segments_ = 1;
  }
  
  inline void ConfigureSlave(int i) {
    monitored_segment_ = i;
    process_fn_ = &SegmentGenerator::ProcessSlave;
    num_segments_ = 0;
  }

  void set_segment_parameters(int index, float primary, float secondary) {
    // assert (primary >= -1.0f && primary <= 2.0f)
    // assert (secondary >= 0.0f && secondary <= 1.0f)
    parameters_[index].primary = primary;
    parameters_[index].secondary = secondary;
  }
  
  inline int num_segments() {
    return num_segments_;
  }

 private:
  // Process function for the general case.
  DECLARE_PROCESS_FN(MultiSegment);
  DECLARE_PROCESS_FN(DecayEnvelope);
  DECLARE_PROCESS_FN(TimedPulseGenerator);
  DECLARE_PROCESS_FN(GateGenerator);
  DECLARE_PROCESS_FN(SampleAndHold);
  DECLARE_PROCESS_FN(TapLFO);
  DECLARE_PROCESS_FN(FreeRunningLFO);
  DECLARE_PROCESS_FN(Delay);
  DECLARE_PROCESS_FN(Portamento);
  DECLARE_PROCESS_FN(Zero);
  DECLARE_PROCESS_FN(ClockedSampleAndHold);
  DECLARE_PROCESS_FN(Slave);
  
  void ShapeLFO(float shape, Output* in_out, size_t size);
  float WarpPhase(float t, float curve) const;
  float RateToFrequency(float rate) const;
  float PortamentoRateToLPCoefficient(float rate) const;
  
  float sample_rate_;

  float phase_;
  float aux_;
  float previous_delay_sample_;

  float start_;
  float value_;
  float lp_;
  float primary_;
  
  float zero_;
  float half_;
  float one_;
  
  int active_segment_;
  int monitored_segment_;
  int retrig_delay_;
  
  int num_segments_;
  
  ProcessFn process_fn_;
  
  RampExtractor ramp_extractor_;
  stmlib::HysteresisQuantizer ramp_division_quantizer_;
  
  Segment segments_[kMaxNumSegments + 1];  // There's a sentinel!
  segment::Parameters parameters_[kMaxNumSegments];
  
  DelayLine16Bits<kMaxDelay> delay_line_;
  
  static ProcessFn process_fn_table_[12];
  
  DISALLOW_COPY_AND_ASSIGN(SegmentGenerator);
};

}  // namespace stages

#endif  // STAGES_SEGMENT_GENERATOR_H_
