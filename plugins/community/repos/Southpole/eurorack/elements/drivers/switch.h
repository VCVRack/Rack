// Copyright 2014 Olivier Gillet.
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
// Driver for the front panel switch.

#ifndef ELEMENTS_DRIVERS_SWITCH_H_
#define ELEMENTS_DRIVERS_SWITCH_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

namespace elements {

class Switch {
 public:
  Switch() { }
  ~Switch() { }
  
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
    return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
  }
  
 private:
  uint8_t switch_state_;
  
  DISALLOW_COPY_AND_ASSIGN(Switch);
};

}  // namespace elements

#endif  // ELEMENTS_DRIVERS_SWITCH_H_
