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
// Driver for ADC1 - used for scanning CVs.

#ifndef ELEMENTS_DRIVERS_CV_ADC_H_
#define ELEMENTS_DRIVERS_CV_ADC_H_

#include "stmlib/stmlib.h"

namespace elements {

enum CvAdcChannel {
  CV_ADC_V_OCT,
  CV_ADC_FM,
  CV_ADC_PRESSURE,
  CV_ADC_EXCITER_BOW_TIMBRE,
  CV_ADC_EXCITER_BLOW_META,
  CV_ADC_EXCITER_BLOW_TIMBRE,
  CV_ADC_EXCITER_STRIKE_META,
  CV_ADC_EXCITER_STRIKE_TIMBRE,
  CV_ADC_RESONATOR_DAMPING,
  CV_ADC_RESONATOR_GEOMETRY,
  CV_ADC_RESONATOR_POSITION,
  CV_ADC_RESONATOR_BRIGHTNESS,
  CV_ADC_SPACE,
  CV_ADC_CHANNEL_LAST
};

class CvAdc {
 public:
  CvAdc() { }
  ~CvAdc() { }
  
  void Init() { Init(true); }
  void Init(bool auto_convert);
  void DeInit();
  void Convert();
  inline const uint16_t* values() { return &values_[0]; }
  inline int32_t value(uint8_t channel) const { return values_[channel]; }
  inline float float_value(uint8_t index) const {
    return static_cast<float>(values_[index]) / 65536.0f;
  }
  
 private:
  uint16_t values_[CV_ADC_CHANNEL_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(CvAdc);
};

}  // namespace elements

#endif  // ELEMENTS_DRIVERS_CV_ADC_H_
