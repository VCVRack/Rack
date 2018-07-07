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
// SVF used for modeling the bridged T-networks.

#ifndef PEAKS_DRUMS_SVF_H_
#define PEAKS_DRUMS_SVF_H_

#include "stmlib/stmlib.h"

namespace peaks {

enum SvfMode {
  SVF_MODE_LP,
  SVF_MODE_BP,
  SVF_MODE_HP
};

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
  
  void set_punch(uint16_t punch) {
    punch_ = (static_cast<uint32_t>(punch) * punch) >> 24;
  }
  
  void set_mode(SvfMode mode) {
    mode_ = mode;
  }

  int32_t Process(int32_t sample) IN_RAM;
  
 private:
  bool dirty_;
  
  int16_t frequency_;
  int16_t resonance_;
  
  int32_t punch_;
  int32_t f_;
  int32_t damp_;

  int32_t lp_;
  int32_t bp_;
  
  SvfMode mode_;

  DISALLOW_COPY_AND_ASSIGN(Svf);
};

}  // namespace peaks

#endif  // PEAKS_DRUMS_SVF_H_
