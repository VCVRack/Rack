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
// Driver for the main RGB LED.

#ifndef FRAMES_DRIVERS_RGB_LED_H_
#define FRAMES_DRIVERS_RGB_LED_H_

#include "stmlib/stmlib.h"

namespace frames {

class RgbLed {
 public:
  RgbLed() { }
  ~RgbLed() { }
  
  void Init();
  
  inline void set_color(uint8_t r, uint8_t g, uint8_t b) {
    rgb_[0] = r;
    rgb_[1] = g;
    rgb_[2] = b;
  }
  
  inline void set_color(const uint8_t* rgb) {
    rgb_[0] = rgb[0];
    rgb_[1] = rgb[1];
    rgb_[2] = rgb[2];
  }
  
  inline void Dim(uint16_t brightness) {
    rgb_[0] = rgb_[0] * brightness >> 16;
    rgb_[1] = rgb_[1] * brightness >> 16;
    rgb_[2] = rgb_[2] * brightness >> 16;
  }

  
  void Write();
  
 private:
  uint8_t rgb_[3];
  
  DISALLOW_COPY_AND_ASSIGN(RgbLed);
};

}  // namespace frames

#endif  // FRAMES_DRIVERS_RGB_LED_H_
