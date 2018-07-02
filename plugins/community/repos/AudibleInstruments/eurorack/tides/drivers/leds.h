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
// Driver for the 4 channels LEDs.

#ifndef TIDES_DRIVERS_LEDS_H_
#define TIDES_DRIVERS_LEDS_H_

#include "stmlib/stmlib.h"

namespace tides {

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();
  
  void set_mode(bool y) {
    mode_r_ = mode_g_ = y;
  }
  
  void set_mode(bool r, bool g) {
    mode_r_ = r;
    mode_g_ = g;
  }
  
  void set_rate(uint16_t y) {
    rate_r_ = rate_g_ = y;
  }
  
  void set_rate(uint16_t r, uint16_t g) {
    rate_r_ = r;
    rate_g_ = g;
  }
  
  void set_value(uint16_t r, uint16_t g) {
    value_r_ = r;
    value_g_ = g;
  }

  void set_value(uint16_t y) {
    value_r_ = value_g_ = y;
  }

  void Write();
  
 private:
  bool mode_r_;
  bool mode_g_;
  uint16_t rate_r_;
  uint16_t rate_g_;
  uint16_t value_r_;
  uint16_t value_g_;
  
  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace tides

#endif  // TIDES_DRIVERS_LEDS_H_
