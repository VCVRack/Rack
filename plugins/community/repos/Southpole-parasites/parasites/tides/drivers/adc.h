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
// Driver for ADC.

#ifndef TIDES_DRIVERS_ADC_H_
#define TIDES_DRIVERS_ADC_H_

#include "stmlib/stmlib.h"

#include "tides/cv_scaler.h"

namespace tides {

class Adc {
 public:
  Adc() { }
  ~Adc() { }
  
  void Init(bool single_channel);
  void DeInit();
  inline uint16_t value(uint8_t channel) const { return values_[channel]; }
  inline const uint16_t* values() { return &values_[0]; }
  
 private:
  uint16_t values_[ADC_CHANNEL_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(Adc);
};

}  // namespace tides

#endif  // TIDES_DRIVERS_ADC_H_
