// Copyright 2012 Olivier Gillet.
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
// Polyphonic voice allocator.

#ifndef STMLIB_ALGORITHMS_VOICE_ALLOCATOR_H_
#define STMLIB_ALGORITHMS_VOICE_ALLOCATOR_H_

#include "stmlib/stmlib.h"
#include <cstring>

namespace stmlib {

enum VoiceAllocatorFlags {
  NOT_ALLOCATED = 0xff,
  ACTIVE_NOTE = 0x80
};

template<uint8_t capacity>
class VoiceAllocator {
 public: 
  VoiceAllocator() { }

  void Init() {
    size_ = 0;
    Clear();
  }

  uint8_t NoteOn(uint8_t note) {
    if (size_ == 0) {
      return NOT_ALLOCATED;
    }

    // First, check if there is a voice currently playing this note. In this
    // case, this voice will be responsible for retriggering this note.
    // Hint: if you're more into string instruments than keyboard instruments,
    // you can safely comment those lines.
    uint8_t voice = Find(note);

    // Then, try to find the least recently touched, currently inactive voice.
    if (voice == NOT_ALLOCATED) {
      for (uint8_t i = 0; i < capacity; ++i) {
        if (lru_[i] < size_ && !(pool_[lru_[i]] & ACTIVE_NOTE)) {
          voice = lru_[i];
        }
      }
    }
    // If all voices are active, use the least recently played note
    // (voice-stealing).
    if (voice == NOT_ALLOCATED) {
      for (uint8_t i = 0; i < capacity; ++i) {
        if (lru_[i] < size_) {
          voice = lru_[i];
        }
      }
    }
    pool_[voice] = note | ACTIVE_NOTE;
    Touch(voice);
    return voice;
  }

  uint8_t NoteOff(uint8_t note) {
    uint8_t voice = Find(note);
    if (voice != NOT_ALLOCATED) {
      pool_[voice] &= 0x7f;
      Touch(voice);
    }
    return voice;
  }

  uint8_t Find(uint8_t note) const {
    for (uint8_t i = 0; i < size_; ++i) {
      if ((pool_[i] & 0x7f) == note) {
        return i;
      }
    }
    return NOT_ALLOCATED;
  }

  void Clear() {
    memset(&pool_, 0, sizeof(pool_));
    for (uint8_t i = 0; i < capacity; ++i) {
      lru_[i] = capacity - i - 1;
    }
  }

  inline void ClearNotes() {
    for (uint8_t i = 0; i < capacity; ++i) {
      pool_[i] &= 0x7f;
    }
  }

  inline void set_size(uint8_t size) {
    size_ = size;
  }

  inline uint8_t size() const { return size_; }

 private:
  void Touch(uint8_t voice) {
    int8_t source = capacity - 1;
    int8_t destination = capacity - 1;
    while (source >= 0) {
      if (lru_[source] != voice) {
        lru_[destination--] = lru_[source];
      }
      --source;
    }
    lru_[0] = voice;
  }
   
  uint8_t pool_[capacity];
  // Holds the indices of the voices sorted by most recent usage.
  uint8_t lru_[capacity];
  uint8_t size_;

  DISALLOW_COPY_AND_ASSIGN(VoiceAllocator);
};

}  // namespace stmlib

#endif  // STMLIB_ALGORITHMS_VOICE_ALLOCATOR_H_
