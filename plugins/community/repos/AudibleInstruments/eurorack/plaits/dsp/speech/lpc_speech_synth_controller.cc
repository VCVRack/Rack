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

#include "plaits/dsp/speech/lpc_speech_synth_controller.h"

#include <algorithm>

#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "plaits/dsp/oscillator/oscillator.h"

namespace plaits {

using namespace std;
using namespace stmlib;

/* static */
uint8_t LPCSpeechSynthWordBank::energy_lut_[16] = {
  0x00, 0x02, 0x03, 0x04, 0x05, 0x07, 0x0a, 0x0f,
  0x14, 0x20, 0x29, 0x39, 0x51, 0x72, 0xa1, 0xff
};

/* static */
uint8_t LPCSpeechSynthWordBank::period_lut_[64] = {
  0, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 45, 47, 49, 51, 53,
 54, 57, 59, 61, 63, 66, 69, 71, 73, 77, 79, 81, 85, 87, 92, 95, 99,
 102, 106, 110, 115, 119, 123, 128, 133, 138, 143, 149, 154, 160
};

/* static */
int16_t LPCSpeechSynthWordBank::k0_lut_[32] = {
  -32064, -31872, -31808, -31680, -31552, -31424, -31232, -30848,
  -30592, -30336, -30016, -29696, -29376, -28928, -28480, -27968,
  -26368, -24256, -21632, -18368, -14528, -10048,  -5184,      0,
    5184,  10048,  14528,  18368,  21632,  24256,  26368,  27968
};

/* static */
int16_t LPCSpeechSynthWordBank::k1_lut_[32] = {
  -20992, -19328, -17536, -15552, -13440, -11200,  -8768,  -6272,
  -3712,   -1088,   1536,   4160,   6720,   9216,  11584,  13824,
  15936,   17856,  19648,  21248,  22656,  24000,  25152,  26176,
  27072,   27840,  28544,  29120,  29632,  30080,  30464,  32384
};

/* static */
int8_t LPCSpeechSynthWordBank::k2_lut_[16] = {
-110, -97, -83, -70, -56, -43, -29, -16, -2, 11, 25, 38, 52, 65, 79, 92
};

/* static */
int8_t LPCSpeechSynthWordBank::k3_lut_[16] = {
-82, -68, -54, -40, -26, -12, 1, 15, 29, 43, 57, 71, 85, 99, 113, 126
};

/* static */
int8_t LPCSpeechSynthWordBank::k4_lut_[16] = {
 -82, -70, -59, -47, -35, -24, -12, -1, 11, 23, 34, 46, 57, 69, 81, 92
};

/* static */
int8_t LPCSpeechSynthWordBank::k5_lut_[16] = {
  -64, -53, -42, -31, -20, -9, 3, 14, 25, 36, 47, 58, 69, 80, 91, 102
};

/* static */
int8_t LPCSpeechSynthWordBank::k6_lut_[16] = {
  -77, -65, -53, -41, -29, -17, -5, 7, 19, 31, 43, 55, 67, 79, 90, 102
};

/* static */
int8_t LPCSpeechSynthWordBank::k7_lut_[8] = {
-64, -40, -16, 7, 31, 55, 79, 102
};

/* static */
int8_t LPCSpeechSynthWordBank::k8_lut_[8] = {
  -64, -44, -24, -4, 16, 37, 57, 77
};

/* static */
int8_t LPCSpeechSynthWordBank::k9_lut_[8] = {
  -51, -33, -15, 4, 22, 32, 59, 77
};

void LPCSpeechSynthWordBank::Init(
    const LPCSpeechSynthWordBankData* word_banks,
    int num_banks,
    BufferAllocator* allocator) {
  word_banks_ = word_banks;
  num_banks_ = num_banks;
  frames_ = allocator->Allocate<LPCSpeechSynth::Frame>(
      kLPCSpeechSynthMaxFrames);
  Reset();
}

void LPCSpeechSynthWordBank::Reset() {
  loaded_bank_ = -1;
  num_frames_ = 0;
  num_words_ = 0;
  fill(
      &word_boundaries_[0],
      &word_boundaries_[kLPCSpeechSynthMaxWords], 0);
}

size_t LPCSpeechSynthWordBank::LoadNextWord(const uint8_t* data) {
  BitStream bitstream;
  bitstream.Init(data);

  LPCSpeechSynth::Frame frame;
  frame.energy = 0;
  frame.period = 0;
  frame.k0 = 0;
  frame.k1 = 0;
  frame.k2 = 0;
  frame.k3 = 0;
  frame.k4 = 0;
  frame.k5 = 0;
  frame.k6 = 0;
  frame.k7 = 0;
  frame.k8 = 0;
  frame.k9 = 0;

  while (true) {
    int energy = bitstream.GetBits(4);
    if (energy == 0) {
      frame.energy = 0;
    } else if (energy == 0xf) {
      bitstream.Flush();
      break;
    } else {
      frame.energy = energy_lut_[energy];
      bool repeat = bitstream.GetBits(1);
      frame.period = period_lut_[bitstream.GetBits(6)];
      if (!repeat) {
        frame.k0 = k0_lut_[bitstream.GetBits(5)];
        frame.k1 = k1_lut_[bitstream.GetBits(5)];
        frame.k2 = k2_lut_[bitstream.GetBits(4)];
        frame.k3 = k3_lut_[bitstream.GetBits(4)];
        if (frame.period) {
          frame.k4 = k4_lut_[bitstream.GetBits(4)];
          frame.k5 = k5_lut_[bitstream.GetBits(4)];
          frame.k6 = k6_lut_[bitstream.GetBits(4)];
          frame.k7 = k7_lut_[bitstream.GetBits(3)];
          frame.k8 = k8_lut_[bitstream.GetBits(3)];
          frame.k9 = k9_lut_[bitstream.GetBits(3)];
        }
      }
    }
    frames_[num_frames_++] = frame;
  }
  return bitstream.ptr() - data;
}

bool LPCSpeechSynthWordBank::Load(int bank) {
  if (bank == loaded_bank_ || bank >= num_banks_) {
    return false;
  }

  num_frames_ = 0;
  num_words_ = 0;
  
  const uint8_t* data = word_banks_[bank].data;
  size_t size = word_banks_[bank].size;
  
  while (size) {
    word_boundaries_[num_words_] = num_frames_;
    size_t consumed = LoadNextWord(data);

    data += consumed;
    size -= consumed;
    ++num_words_;
  }
  word_boundaries_[num_words_] = num_frames_;
  loaded_bank_ = bank;
  return true;
}

void LPCSpeechSynthController::Init(LPCSpeechSynthWordBank* word_bank) {
  word_bank_ = word_bank;
  
  clock_phase_ = 0.0f;
  playback_frame_ = -1;
  last_playback_frame_ = -1;
  remaining_frame_samples_ = 0;

  fill(&sample_[0], &sample_[2], 0.0f);
  fill(&next_sample_[0], &next_sample_[2], 0.0f);

  gain_ = 0.0f;
  
  synth_.Init();
}

void LPCSpeechSynthController::Render(
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
    size_t size) {
  const float rate_ratio = SemitonesToRatio((formant_shift - 0.5f) * 36.0f);
  const float rate = rate_ratio / 6.0f;
  
  // All utterances have been normalized for an average f0 of 100 Hz.
  const float pitch_shift = frequency / \
      (rate_ratio * kLPCSpeechSynthDefaultF0 / kCorrectedSampleRate);
  const float time_stretch = SemitonesToRatio(-speed * 24.0f +
        (formant_shift < 0.4f ? (formant_shift - 0.4f) * -45.0f
            : (formant_shift > 0.6f ? (formant_shift - 0.6f) * -45.0f : 0.0f)));
  
  if (bank != -1) {
    bool reset_everything = word_bank_->Load(bank);
    if (reset_everything) {
      playback_frame_ = -1;
      last_playback_frame_ = -1;
    }
  }
  
  const int num_frames = bank == -1
      ? kLPCSpeechSynthNumVowels
      : word_bank_->num_frames();

  const LPCSpeechSynth::Frame* frames = bank == -1
      ? phonemes_
      : word_bank_->frames();
  
  if (trigger) {
    if (bank == -1) {
      // Pick a pseudo-random consonant, and play it for the duration of a
      // frame.
      int r = (address + 3.0f * formant_shift + 7.0f * frequency) * 8.0f;
      playback_frame_ = (r % kLPCSpeechSynthNumConsonants);
      playback_frame_ += kLPCSpeechSynthNumVowels;
      last_playback_frame_ = playback_frame_ + 1;
    } else {
      word_bank_->GetWordBoundaries(
          address,
          &playback_frame_,
          &last_playback_frame_);
    }
    remaining_frame_samples_ = 0;
  }
  
  if (playback_frame_ == -1 && remaining_frame_samples_ == 0) {
    synth_.PlayFrame(
        frames,
        address * (static_cast<float>(num_frames) - 1.0001f),
        true);
  } else {
    if (remaining_frame_samples_ == 0) {
      synth_.PlayFrame(frames, float(playback_frame_), false);
      remaining_frame_samples_ = kSampleRate / kLPCSpeechSynthFPS * \
          time_stretch;
      ++playback_frame_;
      if (playback_frame_ >= last_playback_frame_) {
        bool back_to_scan_mode = bank == -1 || free_running;
        playback_frame_ = back_to_scan_mode ? -1 : last_playback_frame_;
      }
    }
    remaining_frame_samples_ -= min(size, remaining_frame_samples_);
  }
  
  ParameterInterpolator gain_modulation(&gain_, gain, size);
  
  while (size--) {
    float this_sample[2];
    copy(&next_sample_[0], &next_sample_[2], &this_sample[0]);
    fill(&next_sample_[0], &next_sample_[2], 0.0f);
    
    clock_phase_ += rate;
    if (clock_phase_ >= 1.0f) {
      clock_phase_ -= 1.0f;
      float reset_time = clock_phase_ / rate;
      float new_sample[2];
      
      synth_.Render(
          prosody_amount,
          pitch_shift,
          &new_sample[0],
          &new_sample[1], 1);
      
      float discontinuity[2] = {
        new_sample[0] - sample_[0],
        new_sample[1] - sample_[1]
      };
      this_sample[0] += discontinuity[0] * ThisBlepSample(reset_time);
      next_sample_[0] += discontinuity[0] * NextBlepSample(reset_time);
      this_sample[1] += discontinuity[1] * ThisBlepSample(reset_time);
      next_sample_[1] += discontinuity[1] * NextBlepSample(reset_time);
      copy(&new_sample[0], &new_sample[2], &sample_[0]);
    }
    next_sample_[0] += sample_[0];
    next_sample_[1] += sample_[1];
    const float gain = gain_modulation.Next();
    *excitation++ = this_sample[0] * gain;
    *output++ = this_sample[1] * gain;
  }
}

}  // namespace plaits
