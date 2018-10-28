// Copyright 2015 Olivier Gillet.
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

#ifndef MARBLES_TEST_FIXTURES_H_
#define MARBLES_TEST_FIXTURES_H_

#include <cstdlib>
#include <vector>

#include "marbles/ramp/ramp_divider.h"
#include "marbles/ramp/ramp_extractor.h"

const size_t kSampleRate = 32000;
const size_t kAudioBlockSize = 8;

namespace marbles {

using namespace std;
using namespace stmlib;

class PulseGenerator {
 public:
  PulseGenerator() {
    counter_ = 0;
    previous_state_ = 0;
  }
  ~PulseGenerator() { }
  
  void AddPulses(int total_duration, int on_duration, int num_repetitions) {
    Pulse p;
    p.total_duration = total_duration;
    p.on_duration = on_duration;
    p.num_repetitions = num_repetitions;
    pulses_.push_back(p);
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

enum PatternDifficulty {
  FRIENDLY_PATTERNS,
  FAST_PATTERNS,
  TRICKY_PATTERNS,
  PAUSE_PATTERNS,
};

class ClockGeneratorPatterns {
 public:
  ClockGeneratorPatterns(PatternDifficulty difficulty) {
    if (difficulty == FRIENDLY_PATTERNS) {
      pulse_generator_.AddPulses(800, 400, 100);
      pulse_generator_.AddPulses(400, 32, 100);
      for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 5; ++j) {
          pulse_generator_.AddPulses(600 - j * 100, 3, 2);
        }
      }
      for (int i = 0; i < 300; ++i) {
        int t = 200 + (rand() % 400);
        pulse_generator_.AddPulses(t, t / 4, 1);
      }

      // Completely random clock.
      for (int i = 0; i < 400; ++i) {
        int t = 200 + (rand() % 800);
        int pw = t / 4 + (rand() % (t / 2));
        pulse_generator_.AddPulses(t, pw, 1);
      }
      return;
    } else if (difficulty == FAST_PATTERNS) {
      pulse_generator_.AddPulses(32, 16, 100);
      pulse_generator_.AddPulses(16, 8, 100);
      pulse_generator_.AddPulses(12, 6, 100);
      pulse_generator_.AddPulses(8, 4, 100);
      pulse_generator_.AddPulses(6, 3, 100);
      pulse_generator_.AddPulses(4, 2, 100);
      pulse_generator_.AddPulses(8, 4, 100);
      pulse_generator_.AddPulses(12, 6, 100);
      return;
    } else if (difficulty == PAUSE_PATTERNS) {
      pulse_generator_.AddPulses(800, 400, 100);
      pulse_generator_.AddPulses(32000 * 5 + 10, 400, 1);
      pulse_generator_.AddPulses(800, 400, 100);
    }
    
    // Steady clock
    pulse_generator_.AddPulses(400, 200, 250);
    pulse_generator_.AddPulses(4000 + (rand() % 1000), 2000, 10);
    pulse_generator_.AddPulses(100, 10, 50);
    pulse_generator_.AddPulses(16, 5, 100);
  
    // Periodic clock with some jitter
    for (int i = 0; i < 50; ++i) {
      pulse_generator_.AddPulses(100, 10, 3);
      pulse_generator_.AddPulses(400 + (i % 4), 10, 1);
      pulse_generator_.AddPulses((i == 40) ? 40 : 300, 10, 1);
    }
    for (int i = 0; i < 50; ++i) {
      pulse_generator_.AddPulses(100 + (i % 10), 10, 3);
      pulse_generator_.AddPulses(200 + (i % 4), 10, 2);
      pulse_generator_.AddPulses(300, 10, 1);
    }

    // Really long pattern that hashes well
    for (int i = 0; i < 15; ++i) {
      for (int j = 0; j < 10; ++j) {
        pulse_generator_.AddPulses(100 + j * 30, 10, 1);
      }
    }
    for (int i = 0; i < 15; ++i) {
      for (int j = 0; j < 6; ++j) {
        pulse_generator_.AddPulses(300 - j * 50, 3, 2);
      }
    }
  
    // Random clock with reliable pulse width
    for (int i = 0; i < 300; ++i) {
      int t = 100 + (rand() % 400);
      pulse_generator_.AddPulses(t, t / 4, 1);
    }

    // Completely random clock.
    for (int i = 0; i < 400; ++i) {
      int t = 100 + (rand() % 400);
      int pw = t / 4 + (rand() % (t / 2));
      pulse_generator_.AddPulses(t, pw, 1);
    }
  }
  ~ClockGeneratorPatterns() { }
  
  void Render(size_t size) {
    pulse_generator_.Render(buffer_, size);
  }
  
  GateFlags* clock() { return buffer_; }
  
 private:
  GateFlags buffer_[kAudioBlockSize];
  PulseGenerator pulse_generator_;
  
  DISALLOW_COPY_AND_ASSIGN(ClockGeneratorPatterns);
};

class MasterSlaveRampGenerator {
 public:
  MasterSlaveRampGenerator() {
    ramp_extractor_.Init(4000.0f / kSampleRate);
    ramp_divider_[0].Init();
    ramp_divider_[1].Init();
  }
  ~MasterSlaveRampGenerator() { }
  
  void Process(const GateFlags* clock, size_t size) {
    Ratio r = { 1, 1 };
    ramp_extractor_.Process(r, true, clock, master_ramp_, size);
    
    r.q = 2;
    ramp_divider_[0].Process(r, master_ramp_, slave_ramp_1_, size);

    r.p = 1; r.q = 2;
    ramp_divider_[1].Process(r, master_ramp_, slave_ramp_2_, size);
  }
  
  Ramps ramps() {
    Ramps r;
    r.external = external_ramp_;
    r.master = master_ramp_;
    r.slave[0] = slave_ramp_1_;
    r.slave[1] = slave_ramp_2_;
    return r;
  }
  
  float* master_ramp() { return master_ramp_; }
  float* slave_ramp_1() { return slave_ramp_1_; }
  float* slave_ramp_2() { return slave_ramp_2_; }
 private:
  RampExtractor ramp_extractor_;
  RampDivider ramp_divider_[2];
  float external_ramp_[kAudioBlockSize];
  float master_ramp_[kAudioBlockSize];
  float slave_ramp_1_[kAudioBlockSize];
  float slave_ramp_2_[kAudioBlockSize];
  
  DISALLOW_COPY_AND_ASSIGN(MasterSlaveRampGenerator);
};

}  // namespace marbles

#endif  // MARBLES_TEST_FIXTURES_H_