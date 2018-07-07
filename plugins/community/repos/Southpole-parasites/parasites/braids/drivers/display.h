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
// Driver for 4x14-segments display.

#ifndef BRAIDS_DRIVERS_DISPLAY_H_
#define BRAIDS_DRIVERS_DISPLAY_H_

#include <stm32f10x_conf.h>
#include "stmlib/stmlib.h"

namespace braids {

const uint8_t kDisplayWidth = 4;
const uint8_t kBrightnessLevels = 4;

class Display {
 public:
  Display() { }
  ~Display() { }
  
  void Init();
  void Refresh();
  void Print(const char* s);
  
  char* mutable_buffer() { return buffer_; }
  void set_brightness(uint16_t brightness) { brightness_ = brightness; }
 
 private:
  void Shift14SegmentsWord(uint16_t data);

  char buffer_[kDisplayWidth];
  uint16_t active_position_;
  uint16_t brightness_pwm_cycle_;
  uint16_t brightness_;
  
  DISALLOW_COPY_AND_ASSIGN(Display);
};

}  // namespace braids

#endif  // BRAIDS_DRIVERS_DISPLAY_H_
