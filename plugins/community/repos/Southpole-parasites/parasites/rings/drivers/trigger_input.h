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
// Driver for the trigger (strum) input.

#ifndef RINGS_DRIVERS_TRIGGER_INPUT_H_
#define RINGS_DRIVERS_TRIGGER_INPUT_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

namespace rings {

class TriggerInput {
 public:
  TriggerInput() { }
  ~TriggerInput() { }
  
  void Init();
  void Read();

  inline bool rising_edge() const {
    return trigger_ && !previous_trigger_;
  }
  
  inline bool value() const {
    return trigger_;
  }
  
  inline bool DummyRead() const {
    return !GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2);
  }
  
 private:
  bool previous_trigger_;
  bool trigger_;
  
  DISALLOW_COPY_AND_ASSIGN(TriggerInput);
};

}  // namespace rings

#endif  // RINGS_DRIVERS_TRIGGER_INPUT_H_
