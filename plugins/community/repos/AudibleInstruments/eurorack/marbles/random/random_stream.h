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
// Stream of random values, filled from a hardware RNG, with a fallback
// mechanism.

#ifndef MARBLES_RANDOM_RANDOM_STREAM_H_
#define MARBLES_RANDOM_RANDOM_STREAM_H_

#include "stmlib/stmlib.h"

#include "stmlib/utils/ring_buffer.h"

#include "marbles/random/random_generator.h"

namespace marbles {

class RandomStream {
 public:
  RandomStream() { }
  ~RandomStream() { }
  
  inline void Init(RandomGenerator* fallback_generator) {
    fallback_generator_ = fallback_generator;
    buffer_.Init();
  }

  inline void Write(uint32_t value) {
    // buffer_.Swallow(1);
    // buffer_.Overwrite(value);
    if (buffer_.writable()) {
      buffer_.Overwrite(value);
    }
    fallback_generator_->Mix(value);
  }
  
  inline uint32_t GetWord() {
    if (buffer_.readable()) {
      return buffer_.ImmediateRead();
    } else {
      return fallback_generator_->GetWord();
    }
  }
  
  inline float GetFloat() {
    uint32_t word = GetWord();
    return static_cast<float>(word) / 4294967296.0f;
  }
  
 private:
  stmlib::RingBuffer<uint32_t, 128> buffer_;
  RandomGenerator* fallback_generator_;
  
  DISALLOW_COPY_AND_ASSIGN(RandomStream);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_RANDOM_STREAM_H_
