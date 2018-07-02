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
// Driver for the status LEDs.

#ifndef PEAKS_DRIVERS_LEDS_H_
#define PEAKS_DRIVERS_LEDS_H_

#include "stmlib/stmlib.h"

namespace peaks {

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();
  
  void set_function(uint8_t function) {
    function_ = 1 << function;
  }
  void set_pattern(uint8_t pattern) {
    function_ = pattern;
  }
  void set_twin_mode(bool twin_mode) {
    twin_mode_ = twin_mode;
  }
  void set_levels(uint8_t level_1, uint8_t level_2) {
    levels_[0] = level_1;
    levels_[1] = level_2;
  }
  void fade_levels(uint8_t level_1, uint8_t level_2) {
    levels_[0] = (255 * levels_[0] + level_1) >> 8;
    levels_[1] = (255 * levels_[1] + level_2) >> 8;
  }
  void Write();
  
 private:
  uint8_t function_;
  bool twin_mode_; 
  uint8_t levels_[2];
  
  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace peaks

#endif  // PEAKS_DRIVERS_LEDS_H_
