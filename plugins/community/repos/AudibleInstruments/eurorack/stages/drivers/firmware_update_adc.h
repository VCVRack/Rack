// Copyright 2017 Olivier Gillet.
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
// Driver for the 12-bit SAR ADC, reading the last slider and the corresponding
// CV input.

#ifndef STAGES_DRIVERS_FIRMWARE_UPDATE_ADC_H_
#define STAGES_DRIVERS_FIRMWARE_UPDATE_ADC_H_

#include "stmlib/stmlib.h"

namespace stages {

class FirmwareUpdateAdc {
 public:
  FirmwareUpdateAdc() { }
  ~FirmwareUpdateAdc() { }
  
  void Init();
  void DeInit();
  void Convert();

  inline int32_t slider() const {
    return static_cast<int32_t>(adc_values_[0]);
  }

  inline int32_t sample() const {
    return static_cast<int32_t>(adc_values_[1]);
  }
  
 private:
  uint16_t adc_values_[2];
  
  DISALLOW_COPY_AND_ASSIGN(FirmwareUpdateAdc);
};

}  // namespace stages

#endif  // STAGES_DRIVERS_FIRMWARE_UPDATE_ADC_H_
