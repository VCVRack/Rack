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
// Driver for rotary encoder.

#ifndef BRAIDS_DRIVERS_ENCODER_H_
#define BRAIDS_DRIVERS_ENCODER_H_

#include <stm32f10x_conf.h>
#include "stmlib/stmlib.h"

namespace braids {

class Encoder {
 public:
  Encoder() { }
  ~Encoder() { }
  
  void Init();
  void Debounce();
  
  inline bool released() const {
    return switch_state_ == 0x7f;
  }
  
  inline bool just_pressed() const {
    return switch_state_ == 0x80;
  }

  inline bool pressed() const {
    return switch_state_ == 0x00;
  }
  
  inline bool pressed_immediate() const {
    return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
  }
  
  inline int32_t increment() const {
    int32_t increment = 0;
    uint8_t a = quadrature_decoding_state_[0];
    uint8_t b = quadrature_decoding_state_[1];
    if ((a & 0x03) == 0x02 && (b & 0x03) == 0x00) {
      increment = -1;
    } else {
      if ((b & 0x03) == 0x02 && (a & 0x03) == 0x00) {
        increment = 1;
      }
    }
    return increment;
  }
 
 private:
  uint8_t switch_state_;
  uint8_t quadrature_decoding_state_[2];
  
  DISALLOW_COPY_AND_ASSIGN(Encoder);
};

}  // namespace braids

#endif  // BRAIDS_DRIVERS_ENCODER_H_
