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
// Driver for ADC.

#ifndef BRAIDS_DRIVERS_INTERNAL_ADC_H_
#define BRAIDS_DRIVERS_INTERNAL_ADC_H_

#include "stmlib/stmlib.h"

namespace braids {

class InternalAdc {
 public:
  InternalAdc() { }
  ~InternalAdc() { }
  
  void Init();
  
  inline int32_t value() {
    if (disabled_) {
      return 0;
    } else {
      int32_t v = (static_cast<int32_t>(value_) - 32768) << 8;
      int32_t delta = v - state_;
      state_ += (delta >> 8);
      return state_ >> 8;
    }
  }
  
 private:
  bool disabled_;
  uint16_t value_;
  int32_t state_;
  
  DISALLOW_COPY_AND_ASSIGN(InternalAdc);
};

}  // namespace braids

#endif  // BRAIDS_DRIVERS_INTERNAL_ADC_H_
