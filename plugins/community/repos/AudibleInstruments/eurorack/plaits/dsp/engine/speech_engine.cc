// Copyright 2016 Olivier Gillet.
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
// Various flavours of speech synthesis.

#include "plaits/dsp/engine/speech_engine.h"

#include "plaits/dsp/speech/lpc_speech_synth_words.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void SpeechEngine::Init(BufferAllocator* allocator) {
  sam_speech_synth_.Init();
  naive_speech_synth_.Init();
  lpc_speech_synth_word_bank_.Init(
      word_banks_,
      LPC_SPEECH_SYNTH_NUM_WORD_BANKS,
      allocator);
  lpc_speech_synth_controller_.Init(&lpc_speech_synth_word_bank_);
  word_bank_quantizer_.Init();
  
  temp_buffer_[0] = allocator->Allocate<float>(kMaxBlockSize);
  temp_buffer_[1] = allocator->Allocate<float>(kMaxBlockSize);
  
  prosody_amount_ = 0.0f;
  speed_ = 1.0f;
}

void SpeechEngine::Reset() {
  lpc_speech_synth_word_bank_.Reset();
}

void SpeechEngine::Render(
    const EngineParameters& parameters,
    float* out,
    float* aux,
    size_t size,
    bool* already_enveloped) {
  const float f0 = NoteToFrequency(parameters.note);
  
  const float group = parameters.harmonics * 6.0f;
  
  // Interpolates between the 3 models: naive, SAM, LPC.
  if (group <= 2.0f) {
    *already_enveloped = false;
    
    float blend = group;
    if (group <= 1.0f) {
      naive_speech_synth_.Render(
          parameters.trigger == TRIGGER_RISING_EDGE,
          f0,
          parameters.morph,
          parameters.timbre,
          temp_buffer_[0],
          aux,
          out,
          size);
    } else {
      lpc_speech_synth_controller_.Render(
          parameters.trigger & TRIGGER_UNPATCHED,
          parameters.trigger & TRIGGER_RISING_EDGE,
          -1,
          f0,
          0.0f,
          0.0f,
          parameters.morph,
          parameters.timbre,
          1.0f,
          aux,
          out,
          size);
      blend = 2.0f - blend;
    }
  
    sam_speech_synth_.Render(
        parameters.trigger == TRIGGER_RISING_EDGE,
        f0,
        parameters.morph,
        parameters.timbre,
        temp_buffer_[0],
        temp_buffer_[1],
        size);
    
    blend *= blend * (3.0f - 2.0f * blend);
    blend *= blend * (3.0f - 2.0f * blend);
    for (size_t i = 0; i < size; ++i) {
      aux[i] += (temp_buffer_[0][i] - aux[i]) * blend;
      out[i] += (temp_buffer_[1][i] - out[i]) * blend;
    }
  } else {
    // Change phonemes/words for LPC.
    const int word_bank = word_bank_quantizer_.Process(
        (group - 2.0f) * 0.275f,
        LPC_SPEECH_SYNTH_NUM_WORD_BANKS + 1) - 1;
    
    const bool replay_prosody = word_bank >= 0 && \
        !(parameters.trigger & TRIGGER_UNPATCHED);
    
    *already_enveloped = replay_prosody;
    
    lpc_speech_synth_controller_.Render(
        parameters.trigger & TRIGGER_UNPATCHED,
        parameters.trigger & TRIGGER_RISING_EDGE,
        word_bank,
        f0,
        prosody_amount_,
        speed_,
        parameters.morph,
        parameters.timbre,
        replay_prosody ? parameters.accent : 1.0f,
        aux,
        out,
        size);
  }
}

}  // namespace plaits
