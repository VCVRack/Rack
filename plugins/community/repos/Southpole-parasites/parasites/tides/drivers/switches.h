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
// Driver for the Mode and Range switches.

#ifndef TIDES_DRIVERS_SWITCHES_H_
#define TIDES_DRIVERS_SWITCHES_H_

#include "stmlib/stmlib.h"

#include <stm32f10x_conf.h>

namespace tides {

const uint8_t kNumSwitches = 2;

class Switches {
 public:
  Switches() { }
  ~Switches() { }
  
  void Init();
  void Debounce();
  
  inline bool released(uint8_t index) const {
    return switch_state_[index] == 0x7f;
  }
  
  inline bool just_pressed(uint8_t index) const {
    return switch_state_[index] == 0x80;
  }

  inline bool pressed(uint8_t index) const {
    return switch_state_[index] == 0x00;
  }
  
  inline bool pressed_immediate(uint8_t index) const {
    return !GPIO_ReadInputDataBit(GPIOA, index ? GPIO_Pin_1 : GPIO_Pin_0);
  }
  
 private:
  uint8_t switch_state_[kNumSwitches];
  
  DISALLOW_COPY_AND_ASSIGN(Switches);
};

}  // namespace tides

#endif  // TIDES_DRIVERS_SWITCHES_H_
