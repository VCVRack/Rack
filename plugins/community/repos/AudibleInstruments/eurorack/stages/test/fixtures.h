// Copyright 2017 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef STAGES_TEST_FIXTURES_H_
#define STAGES_TEST_FIXTURES_H_

#include <cstdlib>
#include <vector>

#include "stmlib/test/wav_writer.h"
#include "stmlib/utils/gate_flags.h"

#include "stages/segment_generator.h"

namespace stages {

using namespace std;
using namespace stmlib;

class PulseGenerator {
 public:
  PulseGenerator() {
    counter_ = 0;
    previous_state_ = 0;
  }
  ~PulseGenerator() { }
  
  inline bool empty() const {
    return pulses_.size() == 0;
  }
  
  void AddPulses(int total_duration, int on_duration, int num_repetitions) {
    Pulse p;
    p.total_duration = total_duration;
    p.on_duration = on_duration;
    p.num_repetitions = num_repetitions;
    pulses_.push_back(p);
  }
  
  void CreateTestPattern() {
    AddPulses(16000, 4000, 3);
    AddPulses(16000, 8000, 3);
    AddPulses(32000, 4000, 3);
    AddPulses(32000, 16000, 3);
    AddPulses(32000, 24000, 3);
  }
  
  void Render(GateFlags* clock, size_t size) {
    while (size--) {
      bool current_state = pulses_.size() && counter_ < pulses_[0].on_duration;
      ++counter_;
      if (pulses_.size() && counter_ >= pulses_[0].total_duration) {
        counter_ = 0;
        --pulses_[0].num_repetitions;
        if (pulses_[0].num_repetitions == 0) {
          pulses_.erase(pulses_.begin());
        }
      }
      previous_state_ = *clock++ = ExtractGateFlags(previous_state_, current_state);
    }
  }
  
 private:
  struct Pulse {
    int total_duration;
    int on_duration;
    int num_repetitions;
  };
  int counter_;
  GateFlags previous_state_;
  
  vector<Pulse> pulses_;
  
  DISALLOW_COPY_AND_ASSIGN(PulseGenerator);
};


class SegmentGeneratorTest {
 public:
   SegmentGeneratorTest() {
    segment_generator_.Init();
  }
  ~SegmentGeneratorTest() { }

  PulseGenerator* pulses() { return &pulse_generator_; }
  SegmentGenerator* generator() { return &segment_generator_; }
  
  struct SegmentParameters {
    int index;
    float primary;
    float secondary;
  };
  
  void set_segment_parameters(int index, float primary, float secondary) {
    SegmentParameters p;
    p.index = index;
    p.primary = primary;
    p.secondary = secondary;
    segment_parameters_.push_back(p);
  }
  
  void Render(const char* file_name, int sr) {
    if (pulse_generator_.empty()) {
      pulse_generator_.CreateTestPattern();
    }
    
    stmlib::WavWriter wav_writer(4, sr, 20);
    wav_writer.Open(file_name);
    
    for (int i = 0; i < sr * 20; ++i) {
      GateFlags f;
      pulse_generator_.Render(&f, 1);
      SegmentGenerator::Output out;
      
      for (size_t j = 0; j < segment_parameters_.size(); ++j) {
        const SegmentParameters& p = segment_parameters_[j];
        segment_generator_.set_segment_parameters(
            p.index,
            p.primary >= 0.0f ? p.primary : wav_writer.triangle(-p.primary),
            p.secondary >= 0.0f ? p.secondary : wav_writer.triangle(-p.secondary));
      }
      
      segment_generator_.Process(&f, &out, 1);
      float s[4];
      s[0] = f & GATE_FLAG_HIGH ? 0.8f : 0.0f;
      s[1] = out.value;
      s[2] = out.segment * 0.1f;
      s[3] = out.phase;
      wav_writer.Write(s, 4, 32767.0f);
    }
  }
  
 private:
  SegmentGenerator segment_generator_;
  PulseGenerator pulse_generator_;
  vector<SegmentParameters> segment_parameters_;
  
  DISALLOW_COPY_AND_ASSIGN(SegmentGeneratorTest);
};

}  // namespace stages

#endif  // STAGES_TEST_FIXTURES_H_