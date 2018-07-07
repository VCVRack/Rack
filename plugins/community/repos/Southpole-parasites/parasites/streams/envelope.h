// Copyright 2014 Olivier Gillet.
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
// Simple AD envelope - adapted from Peaks' multistage envelope.

#ifndef STREAMS_ENVELOPE_H_
#define STREAMS_ENVELOPE_H_

#include "stmlib/stmlib.h"

#include "streams/meta_parameters.h"

namespace streams {

enum EnvelopeShape {
  ENV_SHAPE_LINEAR,
  ENV_SHAPE_EXPONENTIAL,
  ENV_SHAPE_QUARTIC
};

const uint16_t kMaxNumSegments = 8;

class Envelope {
 public:
  Envelope() { }
  ~Envelope() { }
  
  void Init();
  void Process(
      int16_t audio,
      int16_t excite,
      uint16_t* gain,
      uint16_t* frequency);

  void Configure(bool alternate, int32_t* parameters, int32_t* globals) {
    uint16_t a, d;
    if (globals) {
      a = globals[0];
      d = globals[2];
      ComputeAmountOffset(
          parameters[1],
          &target_frequency_amount_,
          &target_frequency_offset_);
    } else {
      ComputeAttackDecay(parameters[0], &a, &d);
      ComputeAmountOffset(
          parameters[1],
          &target_frequency_amount_,
          &target_frequency_offset_);
    }

    if (a != attack_ || d != decay_ || alternate != alternate_) {
      attack_ = a;
      decay_ = d;
      alternate_ = alternate;
      if (alternate_) {
        set_ar(a, d);
      } else {
        set_ad(a, d);
      }
      set_hard_reset(true);
    }
  }
  
  inline void set_time(uint16_t segment, uint16_t time) {
    time_[segment] = time;
  }
  
  inline void set_level(uint16_t segment, int16_t level) {
    level_[segment] = level;
  }
  
  inline void set_num_segments(uint16_t num_segments) {
    num_segments_ = num_segments;
  }
  
  inline void set_sustain_point(uint16_t sustain_point) {
    sustain_point_ = sustain_point;
  }
  
  inline void set_ad(uint16_t attack, uint16_t decay) {
    num_segments_ = 2;
    sustain_point_ = 0;

    level_[0] = 0;
    level_[1] = 32767;
    level_[2] = 0;

    time_[0] = attack;
    time_[1] = decay;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_EXPONENTIAL;
  }
  
  inline void set_adr(
      uint16_t attack,
      uint16_t decay,
      uint16_t sustain,
      uint16_t release) {
    num_segments_ = 3;
    sustain_point_ = 0;

    level_[0] = 0;
    level_[1] = 32767;
    level_[2] = sustain;
    level_[3] = 0;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = release;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
  }
  
  inline void set_ar(uint16_t attack, uint16_t decay) {
    num_segments_ = 2;
    sustain_point_ = 1;

    level_[0] = 0;
    level_[1] = 32767;
    level_[2] = 0;

    time_[0] = attack;
    time_[1] = decay;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
  }
  
  inline void set_adsar(
      uint16_t attack,
      uint16_t decay,
      uint16_t sustain,
      uint16_t release) {
    num_segments_ = 4;
    sustain_point_ = 2;

    level_[0] = 0;
    level_[1] = 32767;
    level_[2] = sustain;
    level_[3] = 32767;
    level_[4] = 0;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = attack;
    time_[3] = release;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
    shape_[3] = ENV_SHAPE_LINEAR;
  }
  
  inline void set_adar(
      uint16_t attack,
      uint16_t decay,
      uint16_t sustain,
      uint16_t release) {
    num_segments_ = 4;
    sustain_point_ = 0;

    level_[0] = 0;
    level_[1] = 32767;
    level_[2] = sustain;
    level_[3] = 32767;
    level_[4] = 0;

    time_[0] = attack;
    time_[1] = decay;
    time_[2] = attack;
    time_[3] = release;
    
    shape_[0] = ENV_SHAPE_LINEAR;
    shape_[1] = ENV_SHAPE_LINEAR;
    shape_[2] = ENV_SHAPE_LINEAR;
    shape_[3] = ENV_SHAPE_LINEAR;
  }

  inline void set_hard_reset(bool hard_reset) {
    hard_reset_ = hard_reset;
  }
  
 private:
  bool gate_;
   
  int16_t level_[kMaxNumSegments];
  uint16_t time_[kMaxNumSegments];
  EnvelopeShape shape_[kMaxNumSegments];
  
  int16_t segment_;
  int16_t start_value_;
  int16_t value_;

  uint32_t phase_;
  uint32_t phase_increment_;
  
  uint16_t num_segments_;
  uint16_t sustain_point_;

  int32_t target_frequency_amount_;
  int32_t target_frequency_offset_;
  int32_t frequency_amount_;
  int32_t frequency_offset_;
  
  uint16_t attack_;
  uint16_t decay_;
  
  bool alternate_;
  bool hard_reset_;
  
  int32_t rate_modulation_;
  int32_t gate_level_;
  
  DISALLOW_COPY_AND_ASSIGN(Envelope);
};

}  // namespace streams

#endif  // STREAMS_ENVELOPE_H_
