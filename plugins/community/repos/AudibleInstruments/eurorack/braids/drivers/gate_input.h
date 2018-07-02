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
// Driver for gate input.

#ifndef BRAIDS_DRIVERS_GATE_INPUT_H_
#define BRAIDS_DRIVERS_GATE_INPUT_H_

#include <stm32f10x_conf.h>
#include "stmlib/stmlib.h"

namespace braids {

class GateInput {
 public:
  GateInput() { }
  ~GateInput() { }
  
  void Init();
  
  inline bool Read() {
    return !(GPIOA->IDR & GPIO_Pin_4);
  }
  
  inline bool raised() {
    bool new_state = Read();
    bool rising_edge = (!previous_state_ && new_state);
    previous_state_ = new_state;
    return rising_edge;
  }

  inline bool lowered() {
    bool new_state = Read();
    bool falling_edge = (previous_state_ && !new_state);
    previous_state_ = new_state;
    return falling_edge;
  }
  
  inline bool state() { return previous_state_; }
 
 private:
  bool previous_state_;
   
  DISALLOW_COPY_AND_ASSIGN(GateInput);
};

}  // namespace braids

#endif  // BRAIDS_DRIVERS_GATE_INPUT_H_
