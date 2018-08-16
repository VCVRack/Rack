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
// SVF used for the envelope follower filter bank.

#ifndef STREAMS_SVF_H_
#define STREAMS_SVF_H_

#include "stmlib/stmlib.h"

namespace streams {

class Svf {
 public:
  Svf() { }
  ~Svf() { }
  
  void Init();
  
  void set_frequency(int16_t frequency) {
    dirty_ = dirty_ || (frequency_ != frequency);
    frequency_ = frequency;
  }
  
  void set_resonance(int16_t resonance) {
    resonance_ = resonance;
    dirty_ = true;
  }
  
  void Process(int32_t sample);
  inline int32_t lp() const { return lp_; }
  inline int32_t bp() const { return bp_; }
  inline int32_t hp() const { return hp_; }
  
 private:
  bool dirty_;
  
  int16_t frequency_;
  int16_t resonance_;
  
  int32_t f_;
  int32_t damp_;

  int32_t lp_;
  int32_t bp_;
  int32_t hp_;
  
  DISALLOW_COPY_AND_ASSIGN(Svf);
};

}  // namespace streams

#endif  // STREAMS_SVF_H_
