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
// Sequence of random values.

#ifndef MARBLES_RANDOM_RANDOM_SEQUENCE_H_
#define MARBLES_RANDOM_RANDOM_SEQUENCE_H_

#include "stmlib/stmlib.h"

#include "marbles/random/random_stream.h"

#include <algorithm>

namespace marbles {

const int kDejaVuBufferSize = 16;
const int kHistoryBufferSize = 16;

const float kMaxUint32 = 4294967296.0f;

class RandomSequence {
 public:
  RandomSequence() { }
  ~RandomSequence() { }
  
  inline void Init(RandomStream* random_stream) {
    random_stream_ = random_stream;
    for (int i = 0; i < kDejaVuBufferSize; ++i) {
      loop_[i] = random_stream_->GetFloat();
    }
    std::fill(&history_[0], &history_[kHistoryBufferSize], 0.0f);

    loop_write_head_ = 0;
    length_ = 8;
    step_ = 0;

    record_head_ = 0;
    replay_head_ = -1;
    replay_start_ = 0;
    deja_vu_ = 0.0f;
    replay_hash_ = replay_shift_ = 0;

    redo_read_ptr_ = &loop_[0];
    redo_write_ptr_ = NULL;
    redo_write_history_ptr_ = NULL;
  }
  
  inline void Record() {
    replay_start_ = record_head_;
    replay_head_ = -1;
  }
  
  inline void ReplayPseudoRandom(uint32_t hash) {
    replay_head_ = replay_start_;
    replay_hash_ = hash;
    replay_shift_ = 0;
  }

  inline void ReplayShifted(uint32_t shift) {
    replay_head_ = replay_start_;
    replay_hash_ = 0;
    replay_shift_ = shift;
  }
  
  inline float GetReplayValue() const {
    uint32_t h = (replay_head_ - 1 - replay_shift_ + \
        2 * kHistoryBufferSize) % kHistoryBufferSize;
    if (!replay_hash_) {
      return history_[h];
    } else {
      uint32_t word = static_cast<float>(history_[h] * kMaxUint32);
      word = (word ^ replay_hash_) * 1664525L + 1013904223L;
      return static_cast<float>(word) / kMaxUint32;
    }
  }
  
  inline float RewriteValue(float value) {
    // RewriteValue(x) returns what the most recent call to NextValue would have
    // returned if its second argument were x instead. This is used to "rewrite
    // history" when the module acquires data from an external source (ASR,
    // randomizer or quantizer mode).
    if (replay_head_ >= 0) {
      return GetReplayValue();
    }
    
    if (redo_write_ptr_) {
      *redo_write_ptr_ = 1.0f + value;
    }
    float result = *redo_read_ptr_;
    if (result >= 1.0f) {
      result -= 1.0f;
    } else {
      result = 0.5f;
    }
    if (redo_write_history_ptr_) {
      *redo_write_history_ptr_ = result;
    }
    return result;
  }
  
  inline float NextValue(bool deterministic, float value) {
    if (replay_head_ >= 0) {
      replay_head_ = (replay_head_ + 1) % kHistoryBufferSize;
      return GetReplayValue();
    }
    
    const float p_sqrt = 2.0f * deja_vu_ - 1.0f;
    const float p = p_sqrt * p_sqrt;

    if (random_stream_->GetFloat() <= p && deja_vu_ <= 0.5f) {
      // Generate a new value and put it at the end of the loop.
      redo_write_ptr_ = &loop_[loop_write_head_];
      *redo_write_ptr_ = deterministic
          ? 1.0f + value
          : random_stream_->GetFloat();
      loop_write_head_ = (loop_write_head_ + 1) % kDejaVuBufferSize;
      step_ = length_ - 1;
    } else {
      // Do not generate a new value, just replay the loop or jump randomly.
      // through it.
      redo_write_ptr_ = NULL;
      if (random_stream_->GetFloat() <= p) {
        step_ = static_cast<int>(
            random_stream_->GetFloat() * static_cast<float>(length_));
      } else {
        step_ = step_ + 1;
        if (step_ >= length_) {
          step_ = 0;
        }
      }
    }
    uint32_t i = loop_write_head_ + kDejaVuBufferSize - length_ + step_;
    redo_read_ptr_ = &loop_[i % kDejaVuBufferSize];
    float result = *redo_read_ptr_;
    if (result >= 1.0f) {
      result -= 1.0f;
    } else if (deterministic) {
      // We ask for a deterministic value (shift register), but the loop
      // contain random values. return 0.5f in this case!
      result = 0.5f;
    }
    redo_write_history_ptr_ = &history_[record_head_];
    *redo_write_history_ptr_ = result;
    record_head_ = (record_head_ + 1) % kHistoryBufferSize;
    return result;
  }
  
  inline void NextVector(float* destination, size_t size) {
    float seed = NextValue(false, 0.0f);
    uint32_t word = static_cast<float>(seed * kMaxUint32);
    while (size--) {
      *destination++ = static_cast<float>(word) / kMaxUint32;
      word = word * 1664525L + 1013904223L;
    }
  }
  
  inline void set_deja_vu(float deja_vu) {
    deja_vu_ = deja_vu;
  }
  
  inline void set_length(int length) {
    if (length < 1 || length > kDejaVuBufferSize) {
      return;
    }
    length_ = length;
    step_ = step_ % length;
  }
  
  inline float deja_vu() const {
    return deja_vu_;
  }
  
  inline int length() const {
    return length_;
  }

 private:
  RandomStream* random_stream_;
  float loop_[kDejaVuBufferSize];
  float history_[kHistoryBufferSize];
  int loop_write_head_;
  int length_;
  int step_;
  
  // Allows to go back in the past and get the same results again from NextValue
  // calls. Allows the 3 X channels to be locked to the same random loop.
  int record_head_;
  int replay_head_;
  int replay_start_;
  uint32_t replay_hash_;
  uint32_t replay_shift_;
  
  float deja_vu_;
  
  float* redo_read_ptr_;
  float* redo_write_ptr_;
  float* redo_write_history_ptr_;
  
  DISALLOW_COPY_AND_ASSIGN(RandomSequence);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_RANDOM_SEQUENCE_H_
