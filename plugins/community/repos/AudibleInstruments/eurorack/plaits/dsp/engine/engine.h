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
// Base class for all engines.

#ifndef PLAITS_DSP_ENGINE_ENGINE_H_
#define PLAITS_DSP_ENGINE_ENGINE_H_

#include "plaits/dsp/dsp.h"

#include "stmlib/dsp/units.h"
#include "stmlib/utils/buffer_allocator.h"

namespace plaits {

inline float NoteToFrequency(float midi_note) {
  midi_note -= 9.0f;
  CONSTRAIN(midi_note, -128.0f, 127.0f);
  return a0 * 0.25f * stmlib::SemitonesToRatio(midi_note);
}

enum TriggerState {
  TRIGGER_LOW = 0,
  TRIGGER_RISING_EDGE = 1,
  TRIGGER_UNPATCHED = 2,
  TRIGGER_UNPATCHED_AUTOTRIGGED = 3,
};

struct EngineParameters {
  TriggerState trigger;
  float note;
  float timbre;
  float morph;
  float harmonics;
  float accent;
};

struct PostProcessingSettings {
  // A negative value indicates that a limiter must be used.
  float out_gain;
  float aux_gain;
  
  // When this flag is set to true, the engine declares that it will 
  // render a signal that already has an envelope (eg: modal drum, 808 kick).
  // By reporting this information, the synthesis voice upstream will
  // bypass the internal envelope/LPG.
  //
  // This parameter can be changed on a per-call basis when calling Render()
  // This is used by the speech synthesis engine, which renders either
  // a continuous vowel sound (which needs to be enveloped by the LPG)
  // or a word/sentence (which is already enveloped).
  bool already_enveloped;
};

class Engine {
 public:
  Engine() { }
  ~Engine() { }
  virtual void Init(stmlib::BufferAllocator* allocator) = 0;
  virtual void Reset() = 0;
  virtual void Render(
      const EngineParameters& parameters,
      float* out,
      float* aux,
      size_t size,
      bool* already_enveloped) = 0;
  PostProcessingSettings post_processing_settings;
};

template<int max_size>
class EngineRegistry {
 public:
  EngineRegistry() { }
  ~EngineRegistry() { }
  
  void Init() {
    num_engines_ = 0;
  }

  inline Engine* get(int index) {
    return engine_[index];
  }
  
  void RegisterInstance(
      Engine* instance,
      bool already_enveloped,
      float out_gain,
      float aux_gain) {
    if (num_engines_ >= max_size) {
      return;
    }
    engine_[num_engines_] = instance;
    PostProcessingSettings* s = &instance->post_processing_settings;
    s->already_enveloped = already_enveloped;
    s->out_gain = out_gain;
    s->aux_gain = aux_gain;
    ++num_engines_;
  }
  
  inline int size() const { return num_engines_; }

 private:
  Engine* engine_[max_size];
  int num_engines_;
};

}  // namespace plaits

#endif  // PLAITS_DSP_ENGINE_ENGINE_H_