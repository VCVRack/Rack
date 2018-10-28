// Copyright 2016 Olivier Gillet.
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
// Drivers for the 12-bit ADC used for firmware updates through the MODEL CV in.

#ifndef PLAITS_DRIVERS_FIRMWARE_UPDATE_ADC_H_
#define PLAITS_DRIVERS_FIRMWARE_UPDATE_ADC_H_

#include "stmlib/stmlib.h"

namespace plaits {

class FirmwareUpdateAdc {
 public:
  FirmwareUpdateAdc() { }
  ~FirmwareUpdateAdc() { }
  
  void Init();
  void DeInit();
  void Convert();
  
  inline uint16_t gain_pot() const {
    return values_[0];
  }
  
  inline uint16_t sample() const {
    return values_[1];
  }

 private:
  uint16_t values_[2];
  DISALLOW_COPY_AND_ASSIGN(FirmwareUpdateAdc);
};

}  // namespace plaits

#endif  // PLAITS_DRIVERS_FIRMWARE_UPDATE_ADC_H_
