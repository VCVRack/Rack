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

#ifndef AVRLIBX_IO_ADC_H_
#define AVRLIBX_IO_ADC_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/io/gpio.h"

namespace avrlibx {

template<typename Port, uint8_t channel>
struct ADCChannelWrapper { };

#define WRAP_ADC_CHANNEL(port, index) \
template<> \
struct ADCChannelWrapper<Port ## port, index> { \
  static inline ADC_t& adc() { return ADC ## port; } \
  static inline ADC_CH_t& channel() { return ADC ## port.CH ## index; } \
};

WRAP_ADC_CHANNEL(A, 0)
WRAP_ADC_CHANNEL(A, 1)
WRAP_ADC_CHANNEL(A, 2)
WRAP_ADC_CHANNEL(A, 3)

#ifdef ADCB
  WRAP_ADC_CHANNEL(B, 0)
  WRAP_ADC_CHANNEL(B, 1)
  WRAP_ADC_CHANNEL(B, 2)
  WRAP_ADC_CHANNEL(B, 3)
#endif

template<
    typename Port,
    uint8_t channel,
    uint8_t prescaler = 7>
struct ADC {
  typedef ADCChannelWrapper<Port, channel> adc;

  static inline void Init() {
    adc::adc().CTRLA |= ADC_ENABLE_bm;
    adc::adc().CTRLB = ADC_CONMODE_bm | 0x06;  // Signed mode + left aligned.
    adc::adc().REFCTRL = ADC_REFSEL1_bm;  // External reference on port A.
    adc::adc().PRESCALER = prescaler;
    adc::channel().CTRL = ADC_CH_INPUTMODE1_bm;  // Differential input.
  }
  
  static inline void StartConversion(uint8_t pin) {
    adc::channel().MUXCTRL = (pin << ADC_CH_MUXPOS_gp) | 1;
    adc::channel().CTRL |= ADC_CH_START_bm;
  }
  
  static inline uint8_t readable() { return adc::channel().INTFLAGS; }
  
  static inline void Wait() {
    while (!readable());
    adc::channel().INTFLAGS = 1;
  }
  static inline int16_t Read() {
    Wait();
    return ReadOut();
  }
  static inline int16_t ReadOut() {
    return adc::channel().RES;
  }
  static inline int8_t ReadOut8() {
    return adc::channel().RESH;
  }
};

}  // namespace avrlibx

#endif   // AVRLIBX_IO_ADC_H_
