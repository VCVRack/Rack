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
// Feeds frames to the LPC10 speech synth.

#ifndef PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_CONTROLLER_H_
#define PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_CONTROLLER_H_

#include "plaits/dsp/speech/lpc_speech_synth.h"

#include "stmlib/utils/buffer_allocator.h"

namespace plaits {

class BitStream {
 public:
  BitStream() { }
  ~BitStream() { }

  inline void Init(const uint8_t* p) {
    p_ = p;
    available_ = 0;
    bits_ = 0;
  }

  inline void Flush() {
    while (available_) {
      GetBits(1);
    }
  }

  inline uint8_t GetBits(int num_bits) {
    int shift = num_bits;
    if (num_bits > available_) {
      bits_ <<= available_;
      shift -= available_;
      bits_ |= Reverse(*p_++);
      available_ += 8;
    }
    bits_ <<= shift;
    uint8_t result = bits_ >> 8;
    bits_ &= 0xff;
    available_ -= num_bits;
    return result;
  }

  inline const uint8_t* ptr() const { return p_; }

 private:
  inline uint8_t Reverse(uint8_t b) const {
    b = (b >> 4) | (b << 4);
  	b = ((b & 0xcc) >> 2) | ((b & 0x33) << 2);
  	b = ((b & 0xaa) >> 1) | ((b & 0x55) << 1);
    return b;
  }

  const uint8_t* p_;
  int available_;
  uint16_t bits_;

  DISALLOW_COPY_AND_ASSIGN(BitStream);
};

const int kLPCSpeechSynthMaxWords = 32;
const int kLPCSpeechSynthMaxFrames = 1024;
const int kLPCSpeechSynthNumVowels = 5;
const int kLPCSpeechSynthNumConsonants = 10;
const int kLPCSpeechSynthNumPhonemes = \
    kLPCSpeechSynthNumVowels + kLPCSpeechSynthNumConsonants;
const float kLPCSpeechSynthFPS = 40.0f;

struct LPCSpeechSynthWordBankData {
  const uint8_t* data;
  size_t size;
};

class LPCSpeechSynthWordBank {
 public:
  LPCSpeechSynthWordBank() { }
  ~LPCSpeechSynthWordBank() { }

  void Init(
      const LPCSpeechSynthWordBankData* word_banks,
      int num_banks,
      stmlib::BufferAllocator* allocator);
  
  bool Load(int index);
  void Reset();
  
  inline int num_frames() const { return num_frames_; }
  inline const LPCSpeechSynth::Frame* frames() const { return frames_; }
  
  inline void GetWordBoundaries(float address, int* start, int* end) {
    if (num_words_ == 0) {
      *start = *end = -1;
    } else {
      int word = static_cast<int>(address * static_cast<float>(num_words_));
      if (word >= num_words_) {
        word = num_words_ - 1;
      }
      *start = word_boundaries_[word];
      *end = word_boundaries_[word + 1] - 1;
    }
  }
  
 private:
  size_t LoadNextWord(const uint8_t* data);
  
  const LPCSpeechSynthWordBankData* word_banks_;
  
  int num_banks_;
  int loaded_bank_;
  int num_frames_;
  int num_words_;
  int word_boundaries_[kLPCSpeechSynthMaxWords];
  
  LPCSpeechSynth::Frame* frames_;
  
  static uint8_t energy_lut_[16];
  static uint8_t period_lut_[64];
  static int16_t k0_lut_[32];
  static int16_t k1_lut_[32];
  static int8_t k2_lut_[16];
  static int8_t k3_lut_[16];
  static int8_t k4_lut_[16];
  static int8_t k5_lut_[16];
  static int8_t k6_lut_[16];
  static int8_t k7_lut_[8];
  static int8_t k8_lut_[8];
  static int8_t k9_lut_[8];
};

class LPCSpeechSynthController {
 public:
  LPCSpeechSynthController() { }
  ~LPCSpeechSynthController() { }
  
  void Init(LPCSpeechSynthWordBank* word_bank);
  
  void Render(
      bool free_running,
      bool trigger,
      int bank,
      float frequency,
      float prosody_amount,
      float speed,
      float address,
      float formant_shift,
      float gain,
      float* excitation,
      float* output,
      size_t size);
  
 private:
  float clock_phase_;
  float sample_[2];
  float next_sample_[2];
  float gain_;
  LPCSpeechSynth synth_;

  int playback_frame_;
  int last_playback_frame_;
  size_t remaining_frame_samples_;

  LPCSpeechSynthWordBank* word_bank_;
  
  static const LPCSpeechSynth::Frame phonemes_[kLPCSpeechSynthNumPhonemes];
  
  DISALLOW_COPY_AND_ASSIGN(LPCSpeechSynthController);
};

};  // namespace plaits

#endif  // PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_CONTROLLER_H_
