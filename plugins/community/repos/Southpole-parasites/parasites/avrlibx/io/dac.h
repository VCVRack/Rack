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

#ifndef AVRLIBX_IO_DAC_H_
#define AVRLIBX_IO_DAC_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/io/gpio.h"

namespace avrlibx {

template<typename Port>
struct DACWrapper { };

#define WRAP_DAC(port) \
template<> \
struct DACWrapper<Port ## port> { \
  static inline DAC_t& dac() { return DAC ## port; } \
  static inline uint8_t status() { return DAC ## port ## _STATUS; } \
  template<int channel> \
  struct Channel { \
    static inline void Write(uint16_t value) { \
      if (channel == 0) { \
        DAC ## port ## _CH0DATA = value; \
      } else { \
        DAC ## port ## _CH1DATA = value; \
      } \
    } \
    static volatile void* dma_data() { return data(); } \
    static volatile void* data() { \
      if (channel == 0) { \
        return &(DAC ## port ## _CH0DATA); \
      } else { \
        return &(DAC ## port ## _CH1DATA); \
      } \
    } \
  }; \
  typedef uint16_t Value; \
};

WRAP_DAC(B)

enum DACReference {
  DAC_REF_INTERNAL_1V = 0,
  DAC_REF_AVCC = 1,
  DAC_REF_PORTA_AREF = 2,
  DAC_REF_PORTB_AREF = 3
};

template<
    typename Port,
    bool ch0 = true,
    bool ch1 = false,
    DACReference reference = DAC_REF_INTERNAL_1V>
struct DAC {
  typedef DACWrapper<Port> dac;

  static inline void Init() {
    uint8_t ctrl_a = DAC_ENABLE_bm;
    if (ch1) {
      ctrl_a |= DAC_CH1EN_bm;
    }
    if (ch0) {
      ctrl_a |= DAC_CH0EN_bm;
    }
    dac::dac().CTRLB = ch0 && !ch1 ? 0 : DAC_CHSEL1_bm;
    dac::dac().CTRLC = reference << DAC_REFSEL_gp | 0;  // Right-adjust
    set_conversion_time(0x06);  // 96 CLK between conversions
    set_refresh_time(0x01);  // Very fast auto refresh for accurate timing.
    dac::dac().CTRLA = ctrl_a;
  }
  
  static inline void set_conversion_time(uint8_t conversion_time) {
    dac::dac().TIMCTRL = (dac::dac().TIMCTRL & ~DAC_CONINTVAL_gm) \
        | (conversion_time << DAC_CONINTVAL_gp);
  }
  
  static inline void set_refresh_time(uint8_t refresh_time) {
    dac::dac().TIMCTRL = (dac::dac().TIMCTRL & ~DAC_REFRESH_gm) \
        | (refresh_time << DAC_REFRESH_gp);
  }
  
  template<int channel>
  struct Channel {
    static inline uint8_t writable() {
      if (channel == 0) {
        return dac::status() & DAC_CH0DRE_bm;
      } else {
        return dac::status() & DAC_CH1DRE_bm;
      }
    }
    
    static inline void Write(uint16_t value) {
      dac::template Channel<channel>::Write(value);
    }
    
    static inline volatile void* dma_data() {
      return dac::template Channel<channel>::dma_data();
    }
    
    typedef uint16_t Value;
  };
  
  static uint8_t writable() {
    uint8_t mask = DAC_CH0DRE_bm | DAC_CH1DRE_bm;
    return dac::status() & mask == mask;
  }

  static inline void Wait() {
    while (!writable());
  }
  
  Channel<0> channel_0;
  Channel<1> channel_1;
};

}  // namespace avrlibx

#endif   // AVRLIBX_IO_DAC_H_
