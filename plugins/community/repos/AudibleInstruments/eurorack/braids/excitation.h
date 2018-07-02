// Copyright 2013 Olivier Gillet.
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
// Exponential decay excitation.

#ifndef BRAIDS_EXCITATION_H_
#define BRAIDS_EXCITATION_H_

#include "stmlib/stmlib.h"

namespace braids {

class Excitation {
 public:
  Excitation() { }
  ~Excitation() { }

  void Init() {
    delay_ = 0;
    decay_ = 4093;
    counter_ = 0;
    state_ = 0;
  }

  void set_delay(uint16_t delay) {
    delay_ = delay;
  }
  
  void set_decay(uint16_t decay) {
    decay_ = decay;
  }
  
  void Trigger(int32_t level) {
    level_ = level;
    counter_ = delay_ + 1;
  }
  
  bool done() {
    return counter_ == 0;
  }
  
  inline int32_t Process() {
    state_ = (state_ * decay_ >> 12);
    if (counter_ > 0) {
      --counter_;
      if (counter_ == 0) {
        state_ += level_ < 0 ? -level_ : level_;
      }
    }
    return level_ < 0 ? -state_ : state_;
  }
  
 private:
  uint32_t delay_;
  uint32_t decay_;
  int32_t counter_;
  int32_t state_;
  int32_t level_;

  DISALLOW_COPY_AND_ASSIGN(Excitation);
};

}  // namespace braids

#endif  // BRAIDS_EXCITATION_H_
