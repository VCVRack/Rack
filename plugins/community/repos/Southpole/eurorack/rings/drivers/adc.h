// Copyright 2015 Olivier Gillet.
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
// Driver for ADC1 - used for scanning CVs, and ADC2 - used for scanning pots.

#ifndef RINGS_DRIVERS_ADC_H_
#define RINGS_DRIVERS_ADC_H_

#include "stmlib/stmlib.h"

namespace rings {

enum AdcChannel {
  ADC_CHANNEL_CV_FREQUENCY,
  ADC_CHANNEL_CV_STRUCTURE,
  ADC_CHANNEL_CV_BRIGHTNESS,
  ADC_CHANNEL_CV_DAMPING,
  ADC_CHANNEL_CV_POSITION,
  ADC_CHANNEL_CV_V_OCT,
  ADC_CHANNEL_POT_FREQUENCY,
  ADC_CHANNEL_POT_STRUCTURE,
  ADC_CHANNEL_POT_BRIGHTNESS,
  ADC_CHANNEL_POT_DAMPING,
  ADC_CHANNEL_POT_POSITION,
  ADC_CHANNEL_ATTENUVERTER_FREQUENCY,
  ADC_CHANNEL_ATTENUVERTER_STRUCTURE,
  ADC_CHANNEL_ATTENUVERTER_BRIGHTNESS,
  ADC_CHANNEL_ATTENUVERTER_DAMPING,
  ADC_CHANNEL_ATTENUVERTER_POSITION,
  ADC_CHANNEL_LAST,

  ADC_CHANNEL_LAST_DIRECT = ADC_CHANNEL_POT_STRUCTURE,
  ADC_CHANNEL_FIRST_MUXED = ADC_CHANNEL_POT_BRIGHTNESS,
  ADC_CHANNEL_NUM_DIRECT = ADC_CHANNEL_FIRST_MUXED,
  ADC_CHANNEL_NUM_MUXED = ADC_CHANNEL_LAST - ADC_CHANNEL_FIRST_MUXED,
  ADC_CHANNEL_NUM_OFFSETS = ADC_CHANNEL_CV_POSITION + 1
};

class Adc {
 public:
  Adc() { }
  ~Adc() { }
  
  void Init();
  void DeInit();
  void Convert();
  inline const uint16_t* values() { return &values_[0]; }
  inline int32_t value(int32_t channel) const {
    return static_cast<int32_t>(values_[channel]);
  }
  inline float float_value(int32_t index) const {
    return static_cast<float>(values_[index]) / 65536.0f;
  }
  inline int32_t last_read() const {
    return last_read_ + ADC_CHANNEL_FIRST_MUXED;
  }
  
 private:
  static uint8_t addresses_[ADC_CHANNEL_NUM_MUXED];
  
  uint16_t values_[ADC_CHANNEL_LAST];
  int32_t index_;
  int32_t last_read_;
  bool state_;
  
  DISALLOW_COPY_AND_ASSIGN(Adc);
};

}  // namespace rings

#endif  // RINGS_DRIVERS_ADC_H_
