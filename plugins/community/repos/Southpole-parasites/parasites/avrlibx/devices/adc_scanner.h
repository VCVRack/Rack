// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Abstraction around an ADC port/channel pair to get stable readings. Uses a
// deadband around the latest stable value (more memory efficient than averaging
// past values).

#ifndef AVRLIBX_DEVICES_ADC_SCANNER_H_
#define AVRLIBX_DEVICES_ADC_SCANNER_H_

#include "avrlibx/io/adc.h"
#include "avrlibx/utils/op.h"

namespace avrlibx {

template<
    typename Port,
    uint8_t channel,
    uint8_t first_input = 0,
    uint8_t last_input = 7,
    uint8_t threshold = 128,
    uint8_t resolution = 10>
class ADCScanner {
 public:
  enum {
    num_inputs = last_input - first_input + 1
  };

  ADCScanner() { }

  static inline void Init() {
    scan_cycle_ = 0;
    ADCChannel::Init();
    ADCChannel::StartConversion(scan_cycle_ + first_input);
    Lock(threshold);
  }
 
  static void Lock(uint16_t locked_threshold) {
    for (uint8_t i = 0; i < num_inputs; ++i) {
      thresholds_[i] = locked_threshold;
    }
  }

  static inline void Read() {
    int16_t value = Clip(ADCChannel::Read(), 0, 32767);
    int32_t delta = static_cast<int16_t>(value_[scan_cycle_]) - 
        static_cast<int16_t>(value);
    if (delta < 0) {
      delta = -delta;
    }
    if (delta >= thresholds_[scan_cycle_]) {
      thresholds_[scan_cycle_] = threshold;
      value_[scan_cycle_] = value;
    }
    ++scan_cycle_;
    if (scan_cycle_ == num_inputs) {
      scan_cycle_ = 0;
    }
    ADCChannel::StartConversion(scan_cycle_ + first_input);
  }

  static inline uint16_t value(uint8_t index) {
    uint16_t shift = (15 - resolution);
    return value_[index] >> shift;
  }
  
  static inline uint16_t value() {
    return value(index());
  }

  static inline uint8_t index() {
    if (scan_cycle_ == 0) {
      return num_inputs - 1;
    } else {
      return scan_cycle_ - 1;
    }
  }

 private:
  typedef ADC<Port, channel> ADCChannel;
  
  static uint8_t scan_cycle_;
  static uint16_t value_[num_inputs];
  static uint16_t thresholds_[num_inputs];

  DISALLOW_COPY_AND_ASSIGN(ADCScanner);
};

/* static */
template<typename P, uint8_t c, uint8_t f, uint8_t l, uint8_t t, uint8_t r>
uint8_t ADCScanner<P, c, f, l, t, r>::scan_cycle_;

/* static */
template<typename P, uint8_t c, uint8_t f, uint8_t l, uint8_t t, uint8_t r>
uint16_t ADCScanner<P, c, f, l, t, r>::value_[num_inputs];

/* static */
template<typename P, uint8_t c, uint8_t f, uint8_t l, uint8_t t, uint8_t r>
uint16_t ADCScanner<P, c, f, l, t, r>::thresholds_[num_inputs];

}  // namespace avrlibx

#endif  // AVRLIBX_DEVICES_ADC_SCANNER_H_
