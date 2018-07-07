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
// Driver for the 3 LEDs.

#ifndef ELEMENTS_DRIVERS_LEDS_H_
#define ELEMENTS_DRIVERS_LEDS_H_

#include <stm32f4xx_conf.h>

#include "stmlib/stmlib.h"

namespace elements {

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();
  
  void set_gate(bool gate) {
    gate_ = gate;
  }
  
  void set_exciter(uint8_t value) {
    exciter_ = value;
  }

  void set_resonator(uint8_t value) {
    resonator_ = value;
  }
  
  void Write();

 private:
  bool gate_;
  uint8_t exciter_;
  uint8_t resonator_;
   
  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace elements

#endif  // ELEMENTS_DRIVERS_LEDS_H_
