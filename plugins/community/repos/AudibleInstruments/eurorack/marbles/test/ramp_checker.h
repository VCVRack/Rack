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

#ifndef MARBLES_TEST_RAMP_CHECKER_H_
#define MARBLES_TEST_RAMP_CHECKER_H_

#include <cstdio>

namespace marbles {

class RampChecker {
 public:
  RampChecker() {
    previous_phase_ = 0.0f;
    counter_ = 0;
    average_frequency_ = 0.1f;
    stall_counter_ = 0;
  }
  ~RampChecker() { }
  
  void Error(const char* message) {
    printf("RAMP ERROR: %.5f %s\n", float(counter_) / kSampleRate, message);
  }
  
  void Check(float* ramp, size_t size) {
    while (size--) {
      float phase = *ramp++;
      float frequency = phase - previous_phase_;
      if (phase < 0.0f || phase > 1.0f) {
        Error("incorrect ramp value");
      }
      if (frequency < 0.0f) {
        if (average_frequency_ < 0.05f) {
          if (previous_phase_ < 0.75f) {
            Error("reset without having reached maximum value");
          }
          if (phase > 0.25f) {
            Error("does not reset to zero");
          }
        }
      } else {
        average_frequency_ += 0.1f * (frequency - average_frequency_);
      }
      
      if (frequency == 0.0f && phase < 0.01f) {
        ++stall_counter_;
        if (stall_counter_ == 40) {
          Error("Ramp stalls after reset");
        }
      } else {
        stall_counter_ = 0;
      }
      
      if (counter_ > 10 && frequency > 0.5f) {
        Error("High slope");
      }
      
      previous_phase_ = phase;
      ++counter_;
    }
  }
 private:
  size_t stall_counter_;
  float previous_phase_;
  float average_frequency_;
  size_t counter_;
  DISALLOW_COPY_AND_ASSIGN(RampChecker);
};

}  // namespace marbles

#endif  // MARBLES_TEST_RAMP_CHECKER_H_