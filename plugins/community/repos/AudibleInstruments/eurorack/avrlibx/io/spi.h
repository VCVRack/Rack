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

#ifndef AVRLIBX_IO_SPI_H_
#define AVRLIBX_IO_SPI_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/io/gpio.h"

namespace avrlibx {

enum SPIPrescaler {
  SPI_PRESCALER_CLK_2 = 2,
  SPI_PRESCALER_CLK_4 = 4,
  SPI_PRESCALER_CLK_8 = 8,
  SPI_PRESCALER_CLK_16 = 16,
  SPI_PRESCALER_CLK_32 = 32,
  SPI_PRESCALER_CLK_64 = 64,
  SPI_PRESCALER_CLK_128 = 128,
};

template<typename Port> struct SPIWrapper { };

#define WRAP_SPI(port) \
template<> \
struct SPIWrapper<Port ## port> { \
  static inline SPI_t& spi() { \
    return SPI ## port; \
  } \
  static volatile inline uint8_t data() { \
    return SPI ## port ## _DATA; \
  } \
  static inline void set_data(uint16_t value) { \
    SPI ## port ## _DATA = value; \
  } \
  static inline uint8_t readable() { \
    return SPI ## port ## _STATUS & SPI_IF_bm; \
  } \
  static inline uint8_t writable() { \
    return SPI ## port ## _STATUS & SPI_IF_bm; \
  } \
  static inline uint8_t dma_rx_trigger() { \
    return DMA_CH_TRIGSRC_SPI ## port ## _gc; \
  } \
  static inline uint8_t dma_tx_trigger() { \
    return DMA_CH_TRIGSRC_SPI ## port ## _gc; \
  } \
  static inline volatile void* dma_data() { \
    return &(SPI ## port ## _DATA); \
  } \
};

WRAP_SPI(C)
WRAP_SPI(D)
#ifdef SPIE
  WRAP_SPI(E)
#endif
#ifdef SPIF
  WRAP_SPI(F)
#endif

template<
    typename Port,
    typename SlaveSelect,
    DataOrder order = MSB_FIRST,
    SPIPrescaler speed = SPI_PRESCALER_CLK_4,
    bool pull_up_miso=false>
struct SPIMaster {
  typedef SPIWrapper<Port> SPI;
  
  typedef Gpio<Port, 4> SS;
  typedef Gpio<Port, 5> MOSI;
  typedef Gpio<Port, 6> MISO;
  typedef Gpio<Port, 7> SCK;
  
  static inline void Init() {
    SCK::set_direction(OUTPUT);
    MOSI::set_direction(OUTPUT);
    MISO::set_direction(INPUT);
    if (pull_up_miso) {
      PullUpMISO();
    }
    SS::set_direction(OUTPUT);
    SS::High();
    SlaveSelect::set_direction(OUTPUT);
    SlaveSelect::High();
    
    uint8_t control = SPI_ENABLE_bm | SPI_MASTER_bm;
    if (order == LSB_FIRST) {
      control |= SPI_DORD_bm;
    }
    if (speed == SPI_PRESCALER_CLK_2) {
      control |= SPI_CLK2X_bm;
    } else if (speed == SPI_PRESCALER_CLK_4) {
      
    } else if (speed == SPI_PRESCALER_CLK_8) {
      control |= SPI_CLK2X_bm | 0x1;
    } else if (speed == SPI_PRESCALER_CLK_16) {
      control |= 0x1;
    } else if (speed == SPI_PRESCALER_CLK_32) {
      control |= SPI_CLK2X_bm | 0x2;
    } else if (speed == SPI_PRESCALER_CLK_64) {
      control |= 0x2;
    } else if (speed == SPI_PRESCALER_CLK_128) {
      control |= 0x3;
    }
    SPI::spi().CTRL = control;
  }
  
  static inline void PullUpMISO() {
    MISO::set_mode(PORT_MODE_PULL_UP);
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
    return SPI::data();
  }
  
  static inline void Wait() {
    while (!SPI::writable());
  }
  
  static inline void OptimisticWait() {
    Wait();
  }
  
  static inline void Overwrite(uint8_t v) {
    SPI::set_data(v);
  }

  static inline void WriteWord(uint8_t a, uint8_t b) {
    Begin();
    Send(a);
    Send(b);
    End();
  }
  
  static inline uint8_t dma_rx_trigger() {
    return SPI::dma_rx_trigger();
  }
  static inline uint8_t dma_tx_trigger() {
    return SPI::dma_rx_trigger();
  }
  static inline volatile void* dma_data() {
    return SPI::dma_data();
  }
  
  typedef uint8_t Value;
};



template<typename Port, DataOrder order = MSB_FIRST>
struct SPISlave {
  typedef SPIWrapper<Port> SPI;

  typedef Gpio<Port, 4> SS;
  typedef Gpio<Port, 5> MOSI;
  typedef Gpio<Port, 6> MISO;
  typedef Gpio<Port, 7> SCK;

  static inline void Init() {
    SCK::set_direction(INPUT);
    MOSI::set_direction(INPUT);
    MISO::set_direction(OUTPUT);
    SS::set_direction(INPUT);
    SS::High();

    uint8_t control = SPI_ENABLE_bm;
    if (order == LSB_FIRST) {
      control |= SPI_DORD_bm;
    }
    SPI::spi().CTRL = control;
  }

  static inline void Reply(uint8_t value) {
    SPI::set_data(value);
  }

  static inline uint8_t readable() {
    return SPI::readable();
  }

  static inline uint8_t ImmediateRead() {
    return SPI::data();
  }

  static inline uint8_t Read() {
    while (!readable());
    return ImmediateRead();
  }
  
  static inline uint8_t dma_rx_trigger() {
    return SPI::dma_rx_trigger();
  }
  static inline uint8_t dma_tx_trigger() {
    return SPI::dma_rx_trigger();
  }
  static inline volatile void* dma_data() {
    return SPI::dma_data();
  }
  
  typedef uint8_t Value;
};

}  // namespace avrlibx

#endif   // AVRLIBX_IO_SPI_H_
