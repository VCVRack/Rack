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
// Drivers for the 16-bit SDADC reading the CVs.

#ifndef STAGES_DRIVERS_CV_ADC_H_
#define STAGES_DRIVERS_CV_ADC_H_

#include "stmlib/stmlib.h"

namespace stages {

const int kNumCvAdcChannels = 6;

class CvAdc {
 public:
  CvAdc() { }
  ~CvAdc() { }
  
  void Init();
  void DeInit();
  void Convert();

  inline int16_t value(int channel) const {
    return values_[channel_map_[channel]];
  }
  
  inline float float_value(int channel) const {
    return static_cast<float>(value(channel)) / 32768.0f;
  }
  
 private:
  int channel_map_[kNumCvAdcChannels];
  int16_t values_[kNumCvAdcChannels];
  
  DISALLOW_COPY_AND_ASSIGN(CvAdc);
};

}  // namespace stages

#endif  // STAGES_DRIVERS_CV_ADC_H_
