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
// Group of voices.

#ifndef ELEMENTS_DSP_PART_H_
#define ELEMENTS_DSP_PART_H_

#include "stmlib/stmlib.h"

#include "elements/dsp/fx/reverb.h"
#include "elements/dsp/ominous_voice.h"
#include "elements/dsp/patch.h"
#include "elements/dsp/voice.h"

namespace elements {

struct PerformanceState {
  bool gate;
  float note;
  float modulation;
  float strength;
};

// Polyphony is actually possible, but you have to reduce the number of modes
// to 16, and this doesn't sound very good...
const size_t kNumVoices = 1;

class Part {
 public:
  Part() { }
  ~Part() { }
  
  void Init(uint16_t* reverb_buffer);
  
  void Process(
      const PerformanceState& performance_state,
      const float* blow_in,
      const float* strike_in,
      float* main,
      float* aux,
      size_t n);

  inline Patch* mutable_patch() { return &patch_; }
  
  void Seed(uint32_t* seed, size_t size);
  void Panic();
  
  // For metering.
  inline float exciter_level() const { return scaled_exciter_level_; }
  inline float resonator_level() const { return scaled_resonator_level_; }
  inline bool gate() const { return previous_gate_; }
  inline bool bypass() const { return bypass_; }
  inline void set_bypass(bool bypass) { bypass_ = bypass; }

  inline bool easter_egg() const { return easter_egg_; }
  inline void set_easter_egg(bool easter_egg) { easter_egg_ = easter_egg; }

  inline ResonatorModel resonator_model() const { return resonator_model_; }
  inline void set_resonator_model(ResonatorModel r) { resonator_model_ = r; }
  
 private:
  Patch patch_;
  Voice voice_[kNumVoices];
  OminousVoice ominous_voice_[kNumVoices];
  
  bool panic_;
  bool bypass_;
  bool easter_egg_;
  bool previous_gate_;
  float note_[kNumVoices];
  
  size_t num_voices_;
  size_t active_voice_;
  
  float silence_[kMaxBlockSize];
  
  float raw_buffer_[kMaxBlockSize];
  float center_buffer_[kMaxBlockSize];
  float sides_buffer_[kMaxBlockSize];
  
  float scaled_exciter_level_;
  float scaled_resonator_level_;
  float resonator_level_;
  
  Reverb reverb_;
  
  ResonatorModel resonator_model_;
  
  DISALLOW_COPY_AND_ASSIGN(Part);
};

}  // namespace elements

#endif  // ELEMENTS_DSP_PART_H_
