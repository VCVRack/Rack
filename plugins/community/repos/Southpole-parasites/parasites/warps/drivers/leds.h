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
// Driver for the tricolor LED under the big knob, and the bicolor osc LED.

#ifndef WARPS_DRIVERS_LEDS_H_
#define WARPS_DRIVERS_LEDS_H_

#include <stm32f4xx_conf.h>

#include "stmlib/stmlib.h"

namespace warps {

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();
  
  void set_main(uint8_t red, uint8_t green, uint8_t blue) {
    main_red_ = red;
    main_green_ = green;
    main_blue_ = blue;
  }

  void set_osc(uint8_t red, uint8_t green) {
    osc_red_ = red;
    osc_green_ = green;
  }

  void Clear();
  void Write();

 private:
  uint8_t main_red_;
  uint8_t main_green_;
  uint8_t main_blue_;
  uint8_t osc_red_;
  uint8_t osc_green_;
  uint8_t osc_pwm_counter_;
   
  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace warps

#endif  // WARPS_DRIVERS_LEDS_H_
