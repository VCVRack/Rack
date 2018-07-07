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

#ifndef AVRLIBX_IO_USART_SPI_H_
#define AVRLIBX_IO_USART_SPI_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/io/gpio.h"
#include "avrlibx/io/spi.h"
#include "avrlibx/io/usart.h"

namespace avrlibx {

template<typename Port, uint8_t index>
struct UsartSPIPorts { };

template<typename Port>
struct UsartSPIPorts<Port, 0> {
  typedef Gpio<Port, 1> XCK;
  typedef Gpio<Port, 2> RX;
  typedef Gpio<Port, 3> TX;
};

template<typename Port>
struct UsartSPIPorts<Port, 1> {
  typedef Gpio<Port, 5> XCK;
  typedef Gpio<Port, 6> RX;
  typedef Gpio<Port, 7> TX;
};

template<
    typename Port,
    uint8_t index,
    typename SlaveSelect,
    DataOrder order = MSB_FIRST,
    uint8_t speed = SPI_PRESCALER_CLK_4>
struct UsartSPIMaster {
  typedef USARTWrapper<Port, index> USART;
  typedef typename UsartSPIPorts<Port, index>::XCK XCK;
  typedef typename UsartSPIPorts<Port, index>::RX RX;
  typedef typename UsartSPIPorts<Port, index>::TX TX;
  
  static inline void Init() {
    XCK::set_direction(OUTPUT);
    TX::set_direction(OUTPUT);
    RX::set_direction(INPUT);
    SlaveSelect::set_direction(OUTPUT);
    SlaveSelect::High();
    
    uint16_t baud_rate_register_value = speed / 2 - 1;
    USART::usart().BAUDCTRLA = baud_rate_register_value & 0xff;
    USART::usart().BAUDCTRLB = (baud_rate_register_value >> 8) & 0xf;
    
    USART::usart().CTRLC = USART_CMODE_MSPI_gc;
    if (order == LSB_FIRST) {
      USART::usart().CTRLC |= 4;
    }
    USART::usart().CTRLB = USART_RXEN_bm | USART_TXEN_bm;
  }
  
  static inline void PullUpRX() {
    RX::set_mode(PORT_MODE_PULL_UP);
  }
  
  static inline void Begin() {
    SlaveSelect::Low();
  }

  static inline void End() {
    SlaveSelect::High();
  }
  
  static inline void Strobe() {
    SlaveSelect::High();
    SlaveSelect::Low();
  }

  static inline void Write(uint8_t v) {
    Begin();
    Send(v);
    End();
  }
  
  static inline uint8_t Read() {
    Begin();
    uint8_t result = Receive();
    End();
    return result;
  }
  
  static inline void Send(uint8_t v) {
    Overwrite(v);
    Wait();
  }
  
  static inline uint8_t Receive() {
    Send(0xff);
    return ImmediateRead();
  }
  
  static inline uint8_t ImmediateRead() {
    return USART::data();
  }
  
  static inline void Wait() {
    while (!USART::writable());
  }
  
  static inline void OptimisticWait() {
    while (!USART::writable_buffer());
  }
  
  static inline void Overwrite(uint8_t v) {
    USART::set_data(v);
  }

  static inline void WriteWord(uint8_t a, uint8_t b) {
    Begin();
    Send(a);
    Send(b);
    End();
  }
  
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

#endif   // AVRLIBX_IO_USART_SPI_H_
