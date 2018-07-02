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

#ifndef BRAIDS_SVF_H_
#define BRAIDS_SVF_H_

#include "stmlib/stmlib.h"

#include "braids/resources.h"
#include "stmlib/utils/dsp.h"

namespace braids {

enum SvfMode {
  SVF_MODE_LP,
  SVF_MODE_BP,
  SVF_MODE_HP
};

class Svf {
 public:
  Svf() { }
  ~Svf() { }
  
  void Init() {
    lp_ = 0;
    bp_ = 0;
    frequency_ = 33 << 7;
    resonance_ = 16384;
    dirty_ = true;
    punch_ = 0;
    mode_ = SVF_MODE_BP;
  }
  
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

  inline int32_t Process(int32_t in) {
    if (dirty_) {
      f_ = stmlib::Interpolate824(lut_svf_cutoff, frequency_ << 17);
      damp_ = stmlib::Interpolate824(lut_svf_damp, resonance_ << 17);
      dirty_ = false;
    }
    int32_t f = f_;
    int32_t damp = damp_;
    if (punch_) {
      int32_t punch_signal = lp_ > 4096 ? lp_ : 2048;
      f += ((punch_signal >> 4) * punch_) >> 9;
      damp += ((punch_signal - 2048) >> 3);
    }
    int32_t notch = in - (bp_ * damp >> 15);
    lp_ += f * bp_ >> 15;
    CLIP(lp_)
    int32_t hp = notch - lp_;
    bp_ += f * hp >> 15;
    CLIP(bp_)
    return mode_ == SVF_MODE_BP ? bp_ : (mode_ == SVF_MODE_HP ? hp : lp_);
  }
  
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

}  // namespace braids

#endif  // BRAIDS_SVF_H_
