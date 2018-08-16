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
// Part for the string synth easter egg.

#ifndef RINGS_DSP_STRING_SYNTH_PART_H_
#define RINGS_DSP_STRING_SYNTH_PART_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/filter.h"

#include "rings/dsp/dsp.h"
#include "rings/dsp/fx/chorus.h"
#include "rings/dsp/fx/ensemble.h"
#include "rings/dsp/fx/reverb.h"
#include "rings/dsp/limiter.h"
#include "rings/dsp/note_filter.h"
#include "rings/dsp/patch.h"
#include "rings/dsp/performance_state.h"
#include "rings/dsp/string_synth_envelope.h"
#include "rings/dsp/string_synth_voice.h"

namespace rings {

const int32_t kMaxStringSynthPolyphony = 4;
const int32_t kStringSynthVoices = 12;
const int32_t kMaxChordSize = 8;
const int32_t kNumHarmonics = 3;
const int32_t kNumFormants = 3;

enum FxType {
  FX_FORMANT,
  FX_CHORUS,
  FX_REVERB,
  FX_FORMANT_2,
  FX_ENSEMBLE,
  FX_REVERB_2,
  FX_LAST
};

struct VoiceGroup {
  float tonic;
  StringSynthEnvelope envelope;
  int32_t chord;
  float structure;
};

class StringSynthPart {
 public:
  StringSynthPart() { }
  ~StringSynthPart() { }
  
  void Init(uint16_t* reverb_buffer);
  
  void Process(
      const PerformanceState& performance_state,
      const Patch& patch,
      const float* in,
      float* out,
      float* aux,
      size_t size);

  inline void set_polyphony(int32_t polyphony) {
    int32_t old_polyphony = polyphony_;
    polyphony_ = std::min(polyphony, kMaxStringSynthPolyphony);
    for (int32_t i = old_polyphony; i < polyphony_; ++i) {
      group_[i].tonic = group_[0].tonic + i * 0.01f;
    }
    if (active_group_ >= polyphony_) {
      active_group_ = 0;
    }
  }
  
  inline void set_fx(FxType fx_type) {
    if ((fx_type % 3) != (fx_type_ % 3)) {
      clear_fx_ = true;
    }
    fx_type_ = fx_type;
  }
  
 private:
  void ProcessEnvelopes(float shape, uint8_t* flags, float* values);
  void ComputeRegistration(float gain, float registration, float* amplitudes);

  void ProcessFormantFilter(float vowel, float shift, float resonance,
                            float* out, float* aux, size_t size);
  
  StringSynthVoice<kNumHarmonics> voice_[kStringSynthVoices];
  VoiceGroup group_[kMaxStringSynthPolyphony];
  
  stmlib::Svf formant_filter_[kNumFormants];
  Ensemble ensemble_;
  Reverb reverb_;
  Chorus chorus_;
  Limiter limiter_;

  int32_t num_voices_;
  int32_t active_group_;
  uint32_t step_counter_;
  int32_t polyphony_;
  int32_t acquisition_delay_;
  
  FxType fx_type_;
  
  NoteFilter note_filter_;
  
  float filter_in_buffer_[kMaxBlockSize];
  float filter_out_buffer_[kMaxBlockSize];
  
  bool clear_fx_;
  
  DISALLOW_COPY_AND_ASSIGN(StringSynthPart);
};

}  // namespace rings

#endif  // RINGS_DSP_STRING_SYNTH_VOICE_H_
