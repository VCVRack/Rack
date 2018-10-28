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
// Drivers for the 12-bit SAR ADC scanning pots and sliders.

#ifndef STAGES_DRIVERS_POTS_ADC_H_
#define STAGES_DRIVERS_POTS_ADC_H_

#include "stmlib/stmlib.h"

namespace stages {

const int kNumAdcChannels = 6;
const int kNumMuxAddresses = 8;

enum AdcGroup {
  ADC_GROUP_POT,
  ADC_GROUP_SLIDER = kNumAdcChannels,
};

class PotsAdc {
 public:
  PotsAdc() { }
  ~PotsAdc() { }
  
  void Init();
  void DeInit();
  void Convert();

  inline int32_t value(AdcGroup group, int channel) const {
    return static_cast<int32_t>(values_[channel + group]);
  }
  
  inline float float_value(AdcGroup group, int channel) const {
    return static_cast<float>(value(group, channel)) / 65536.0f;
  }
  
  inline uint8_t pot_index() const {
    return pot_index_;
  }
  
  inline uint8_t slider_index() const {
    return slider_index_;
  }
  
 private:
  uint16_t adc_values_[2];
  uint16_t values_[ADC_GROUP_SLIDER + kNumAdcChannels];
  int mux_address_;
  bool conversion_done_;
  uint8_t pot_index_;
  uint8_t slider_index_;
  
  static uint8_t mux_address_to_pot_index_[kNumMuxAddresses];
  static uint8_t mux_address_to_slider_index_[kNumMuxAddresses];
  
  DISALLOW_COPY_AND_ASSIGN(PotsAdc);
};

}  // namespace stages

#endif  // STAGES_DRIVERS_POTS_ADC_H_
