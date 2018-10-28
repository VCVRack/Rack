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
// SAM-inspired speech synth (as used in Shruthi/Ambika/Braids).

#ifndef PLAITS_DSP_SPEECH_SAM_SPEECH_SYNTH_H_
#define PLAITS_DSP_SPEECH_SAM_SPEECH_SYNTH_H_

#include "plaits/dsp/dsp.h"

namespace plaits {
  
const int kSAMNumFormants = 3;
const int kSAMNumVowels = 9;
const int kSAMNumConsonants = 8;
const int kSAMNumPhonemes = kSAMNumVowels + kSAMNumConsonants;

class SAMSpeechSynth {
 public:
  SAMSpeechSynth() { }
  ~SAMSpeechSynth() { }

  void Init();
  
  void Render(
      bool consonant,
      float frequency,
      float vowel,
      float formant_shift,
      float* excitation,
      float* output,
      size_t size);

 private:
  void InterpolatePhonemeData(
      float phoneme,
      float formant_shift,
      uint32_t* formant_frequency,
      float* formant_amplitude);
  
  struct Formant {
    uint8_t frequency;
    uint8_t amplitude;
  };
  struct Phoneme {
    Formant formant[kSAMNumFormants]; 
  };

  float phase_;
  float frequency_;

  float pulse_next_sample_;
  float pulse_lp_;

  uint32_t formant_phase_[3];
  size_t consonant_samples_;
  float consonant_index_;
  
  static Phoneme phonemes_[kSAMNumPhonemes];
  static float formant_amplitude_lut[16];
  
  DISALLOW_COPY_AND_ASSIGN(SAMSpeechSynth);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_SPEECH_SAM_SPEECH_SYNTH_H_
