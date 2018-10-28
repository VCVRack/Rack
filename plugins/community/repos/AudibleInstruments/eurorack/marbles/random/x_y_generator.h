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
// Generator for the X/Y outputs.

#ifndef MARBLES_RANDOM_X_Y_GENERATOR_H_
#define MARBLES_RANDOM_X_Y_GENERATOR_H_

#include "stmlib/stmlib.h"

#include "marbles/ramp/ramp_divider.h"
#include "marbles/ramp/ramp_extractor.h"
#include "marbles/random/output_channel.h"
#include "marbles/random/random_sequence.h"
#include "marbles/random/t_generator.h"

namespace marbles {

enum VoltageRange {
  VOLTAGE_RANGE_NARROW,  // +2V
  VOLTAGE_RANGE_POSITIVE,  // +5V
  VOLTAGE_RANGE_FULL  // +/- 5V
};

enum ClockSource {
  CLOCK_SOURCE_INTERNAL_T1_T2_T3,
  CLOCK_SOURCE_INTERNAL_T1,
  CLOCK_SOURCE_INTERNAL_T2,
  CLOCK_SOURCE_INTERNAL_T3,
  CLOCK_SOURCE_EXTERNAL
};

enum ControlMode {
  CONTROL_MODE_IDENTICAL,
  CONTROL_MODE_BUMP,
  CONTROL_MODE_TILT
};

enum OutputGroup {
  OUTPUT_GROUP_X,
  OUTPUT_GROUP_Y,
  OUTPUT_GROUP_LAST
};

const size_t kNumXChannels = 3;
const size_t kNumYChannels = 1;
const size_t kNumChannels = kNumXChannels + kNumYChannels;

struct GroupSettings {
  ControlMode control_mode;
  VoltageRange voltage_range;
  bool register_mode;
  float register_value;
  float spread;
  float bias;
  float steps;
  float deja_vu;
  int scale_index;
  int length;
  Ratio ratio;
};

class XYGenerator {
 public:
  XYGenerator() { }
  ~XYGenerator() { }
  
  void Init(RandomStream* random_stream, float sr);
  void Process(
      ClockSource clock_source,
      const GroupSettings& x_settings,
      const GroupSettings& y_settings,
      const stmlib::GateFlags* external_clock,
      const Ramps& ramps,
      float* output,
      size_t size);
  
  void LoadScale(int channel, int scale_index, const Scale& scale) {
    output_channel_[channel].LoadScale(scale_index, scale);
  }
  void LoadScale(int scale_index, const Scale& scale) {
    for (size_t i = 0; i < kNumXChannels; ++i) {
      output_channel_[i].LoadScale(scale_index, scale);
    }
  }
  
 private:
  RandomSequence random_sequence_[kNumChannels];
  OutputChannel output_channel_[kNumChannels];
  RampExtractor ramp_extractor_;
  RampDivider ramp_divider_;
  
  int external_clock_stabilization_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(XYGenerator);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_X_Y_GENERATOR_H_
