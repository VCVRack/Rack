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
// Driver for all the LEDs.

#ifndef MARBLES_DRIVERS_LEDS_H_
#define MARBLES_DRIVERS_LEDS_H_

#include "stmlib/stmlib.h"

namespace marbles {
  
enum Led {
  LED_T_DEJA_VU,
  LED_T_MODEL,
  LED_T_RANGE,
  LED_X_DEJA_VU,
  LED_X_CONTROL_MODE,
  LED_X_RANGE,
  LED_X_EXT,
  LED_LAST
};

enum LedColor {
  LED_COLOR_OFF = 0,
  LED_COLOR_RED = 0xff0000,
  LED_COLOR_GREEN = 0x00ff00,
  LED_COLOR_YELLOW = 0xffff00,
};

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();
  void Write();
  void Clear();
  
  void set(Led led, uint32_t color) {
    colors_[led] = color;
  }

 private:
  uint32_t colors_[LED_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace marbles

#endif  // MARBLES_DRIVERS_LEDS_H_
