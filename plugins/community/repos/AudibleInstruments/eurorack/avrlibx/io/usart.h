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

#ifndef AVRLIBX_IO_USART_H_
#define AVRLIBX_IO_USART_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"

namespace avrlibx {
  
enum RxMode {
  RX_DISABLED,
  RX_POLLED
};

enum TxMode {
  TX_DISABLED,
  TX_POLLED
};

template<typename Port, uint8_t index> struct USARTWrapper { };

#define WRAP_USART(port, index) \
template<> \
struct USARTWrapper<Port ## port, index> { \
  static inline USART_t& usart() { \
    return USART ## port ## index; \
  } \
  static volatile inline uint8_t data() { \
    return USART ## port ## index ## _DATA; \
  } \
  static inline void set_data(uint16_t value) { \
    USART ## port ## index ## _DATA = value; \
  } \
  static inline uint8_t readable() { \
    return USART ## port ## index ## _STATUS & USART_RXCIF_bm; \
  } \
  static inline uint8_t writable_buffer() { \
    return USART ## port ## index ## _STATUS & USART_DREIF_bm; \
  } \
  static inline uint8_t writable() { \
    return USART ## port ## index ## _STATUS & USART_TXCIF_bm; \
  } \
  static inline uint8_t dma_rx_trigger() { \
    return DMA_CH_TRIGSRC_USART ## port ## index ## _RXC_gc; \
  } \
  static inline uint8_t dma_tx_trigger() { \
    return DMA_CH_TRIGSRC_USART ## port ## index ## _DRE_gc; \
  } \
  static inline volatile void* dma_data() { \
    return &(USART ## port ## index ## _DATA); \
  } \
};

WRAP_USART(C, 0)
WRAP_USART(C, 1)
WRAP_USART(D, 0)
WRAP_USART(D, 1)
WRAP_USART(E, 0)
#ifdef USARTE1
  WRAP_USART(E, 1)
#endif
#ifdef USARTF0
  WRAP_USART(F, 0)
#endif
#ifdef USARTF1
  WRAP_USART(F, 1)
#endif

template<
    typename Port,
    uint8_t index,
    uint32_t baud_rate,
    RxMode rx_mode,
    TxMode tx_mode>
struct Usart {
  typedef USARTWrapper<Port, index> USART;
  
  static inline void Init() {
    Init<baud_rate>();
  }
  
  template<uint32_t new_baud_rate>
  static inline void Init() {
    uint16_t prescaler = F_CPU / (16 * baud_rate) - 1;
    USART::usart().BAUDCTRLA = prescaler & 0xff;
    USART::usart().BAUDCTRLB = (prescaler >> 8) & 0xf;
    uint8_t control;
    if (rx_mode != RX_DISABLED) {
      control |= USART_RXEN_bm;
    }
    if (tx_mode != TX_DISABLED) {
      if (index == 0) {
        Gpio<Port, 3>::set_direction(OUTPUT);
      } else {
        Gpio<Port, 7>::set_direction(OUTPUT);
      }
      control |= USART_TXEN_bm;
    }
    USART::usart().CTRLB = control;
  }
  
  static inline void Write(uint8_t v) { while (!writable()); Overwrite(v); }
  static inline uint8_t writable() { return USART::writable_buffer(); }
  static inline uint8_t NonBlockingWrite(uint8_t v) {
    if (!writable()) {
      return 0;
    }
    Overwrite(v);
    return 1;
  }
  static inline void Overwrite(uint8_t v) { USART::set_data(v); }
  
  static inline uint8_t Read() { while (!readable()); return ImmediateRead(); }
  static inline uint8_t readable() { return USART::readable(); }
  static inline uint16_t NonBlockingRead() { 
    return readable() ? ImmediateRead() : -1;
  }
  static inline uint8_t ImmediateRead() { return USART::data(); }
  
  static inline uint8_t dma_rx_trigger() {
    return USART::dma_rx_trigger();
  }
  static inline uint8_t dma_tx_trigger() {
    return USART::dma_rx_trigger();
  }
  static inline volatile void* dma_data() {
    return USART::dma_data();
  }
  
  typedef uint8_t Value;
};

}  // namespace avrlibx

#endif   // AVRLIBX_IO_USART_H_
