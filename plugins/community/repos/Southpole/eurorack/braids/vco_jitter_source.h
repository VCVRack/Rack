// Copyright 2012 Olivier Gillet.
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
// A noise source used to add jitter to the VCO.

#ifndef BRAIDS_VCO_JITTER_SOURCE_H_
#define BRAIDS_VCO_JITTER_SOURCE_H_

#include "stmlib/stmlib.h"

#include <cstring>

#include "braids/resources.h"
#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

namespace braids {

using namespace stmlib;

class VcoJitterSource {
 public:
  VcoJitterSource() { }
  ~VcoJitterSource() { }
  
  inline void Init() {
    external_temperature_ = 0;
    room_temperature_ = 0;
    phase_ = 0;
    phase_step_ = 0;
  }
  
  inline int16_t Render(int32_t intensity) {
    // External temperature change, with 1-order filtering.
    uint16_t external_temperature_toss = Random::GetWord();
    if (external_temperature_toss == 0) {
      phase_step_ = phase_step_ * 1664525L + 1013904223L;
      phase_ += (phase_step_ >> 16) * (phase_step_ >> 16);
      external_temperature_ = wav_sine[phase_ >> 24] << 8;
    }
    room_temperature_ += (external_temperature_ - room_temperature_) >> 16;
    int32_t pitch_noise = room_temperature_ * intensity >> 19;
    return pitch_noise;
  }
  
 private:
  uint32_t phase_step_;
  uint32_t phase_;
  int32_t external_temperature_;
  int32_t room_temperature_;
   
  DISALLOW_COPY_AND_ASSIGN(VcoJitterSource);
};

}  // namespace braids

#endif // BRAIDS_VCO_JITTER_SOURCE_H_
