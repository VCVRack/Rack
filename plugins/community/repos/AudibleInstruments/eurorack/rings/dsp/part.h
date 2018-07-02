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
// Group of voices.

#ifndef RINGS_DSP_PART_H_
#define RINGS_DSP_PART_H_

#include <algorithm>

#include "stmlib/stmlib.h"
#include "stmlib/dsp/cosine_oscillator.h"
#include "stmlib/dsp/delay_line.h"

#include "rings/dsp/dsp.h"
#include "rings/dsp/fm_voice.h"
#include "rings/dsp/fx/reverb.h"
#include "rings/dsp/limiter.h"
#include "rings/dsp/note_filter.h"
#include "rings/dsp/patch.h"
#include "rings/dsp/performance_state.h"
#include "rings/dsp/plucker.h"
#include "rings/dsp/resonator.h"
#include "rings/dsp/string.h"

namespace rings {

enum ResonatorModel {
  RESONATOR_MODEL_MODAL,
  RESONATOR_MODEL_SYMPATHETIC_STRING,
  RESONATOR_MODEL_STRING,
  
  // Bonus!
  RESONATOR_MODEL_FM_VOICE,
  RESONATOR_MODEL_SYMPATHETIC_STRING_QUANTIZED,
  RESONATOR_MODEL_STRING_AND_REVERB,
  RESONATOR_MODEL_LAST
};

const int32_t kMaxPolyphony = 4;
const int32_t kNumStrings = kMaxPolyphony * 2;

class Part {
 public:
  Part() { }
  ~Part() { }
  
  void Init(uint16_t* reverb_buffer);
  
  void Process(
      const PerformanceState& performance_state,
      const Patch& patch,
      const float* in,
      float* out,
      float* aux,
      size_t size);

  inline bool bypass() const { return bypass_; }
  inline void set_bypass(bool bypass) { bypass_ = bypass; }

  inline int32_t polyphony() const { return polyphony_; }
  inline void set_polyphony(int32_t polyphony) {
    int32_t old_polyphony = polyphony_;
    polyphony_ = std::min(polyphony, kMaxPolyphony);
    for (int32_t i = old_polyphony; i < polyphony_; ++i) {
      note_[i] = note_[0] + i * 0.05f;
    }
    dirty_ = true;
  }
  
  inline ResonatorModel model() const { return model_; }
  inline void set_model(ResonatorModel model) {
    if (model != model_) {
      model_ = model;
      dirty_ = true;
    }
  }

 private:
  void ConfigureResonators();
  void RenderModalVoice(
      int32_t voice,
      const PerformanceState& performance_state,
      const Patch& patch,
      float frequency,
      float filter_cutoff,
      size_t size);
  void RenderStringVoice(
      int32_t voice,
      const PerformanceState& performance_state,
      const Patch& patch,
      float frequency,
      float filter_cutoff,
      size_t size);
  void RenderFMVoice(
      int32_t voice,
      const PerformanceState& performance_state,
      const Patch& patch,
      float frequency,
      float filter_cutoff,
      size_t size);
  

  inline float Squash(float x) const {
    if (x < 0.5f) {
      x *= 2.0f;
      x *= x;
      x *= x;
      x *= x;
      x *= x;
      x *= 0.5f;
    } else {
      x = 2.0f - 2.0f * x;
      x *= x;
      x *= x;
      x *= x;
      x *= x;
      x = 1.0f - 0.5f * x;
    }
    return x;
  }

  void ComputeSympatheticStringsNotes(
      float tonic,
      float note,
      float parameter,
      float* destination,
      size_t num_strings);
  
  bool bypass_;
  bool dirty_;

  ResonatorModel model_;

  int32_t num_voices_;
  int32_t active_voice_;
  uint32_t step_counter_;
  int32_t polyphony_;
  
  Resonator resonator_[kMaxPolyphony];
  String string_[kNumStrings];
  stmlib::CosineOscillator lfo_[kNumStrings];
  FMVoice fm_voice_[kMaxPolyphony];
  
  stmlib::Svf excitation_filter_[kMaxPolyphony];
  stmlib::DCBlocker dc_blocker_[kMaxPolyphony];
  Plucker plucker_[kMaxPolyphony];

  float note_[kMaxPolyphony];
  NoteFilter note_filter_;
  
  float resonator_input_[kMaxBlockSize];
  float sympathetic_resonator_input_[kMaxBlockSize];
  float noise_burst_buffer_[kMaxBlockSize];
  
  float out_buffer_[kMaxBlockSize];
  float aux_buffer_[kMaxBlockSize];
  
  Reverb reverb_;
  Limiter limiter_;
  
  static float model_gains_[RESONATOR_MODEL_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(Part);
};

}  // namespace rings

#endif  // RINGS_DSP_PART_H_
