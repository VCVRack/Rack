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

#ifndef AVRLIBX_IO_DMA_H_
#define AVRLIBX_IO_DMA_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"

namespace avrlibx {

template<uint8_t index>
struct DMAChannel { };

#define WRAP_DMA_CHANNEL(channel_index) \
template<> \
struct DMAChannel<channel_index> { \
  static inline DMA_CH_t& channel() { \
    return DMA.CH ## channel_index; \
  } \
};

WRAP_DMA_CHANNEL(0)
WRAP_DMA_CHANNEL(1)
WRAP_DMA_CHANNEL(2)
WRAP_DMA_CHANNEL(3)

// Uses DMA to automatically store data from a peripheral to a ring buffer.
template<
    typename Peripheral,
    uint8_t channel_index,
    uint16_t buffer_size>
class DMARxBuffer {
 private:
  typedef DMAChannel<channel_index> Channel;
  typedef typename Peripheral::Value Value;

 public:
  DMARxBuffer() { }
   
  static inline void Init() {
    DMA_CTRL = 0x80;
    uint8_t address_control = 0;
    address_control |= DMA_CH_SRCRELOAD_BURST_gc;
    address_control |= DMA_CH_DESTRELOAD_BLOCK_gc;
    address_control |= DMA_CH_SRCDIR_INC_gc;
    address_control |= DMA_CH_DESTDIR_INC_gc;
    
    Channel::channel().ADDRCTRL = address_control;
    Channel::channel().REPCNT = 0;
    Channel::channel().TRFCNT = buffer_size * sizeof(Value);
    
    uint32_t source = (uint32_t)(Peripheral::dma_data());
    uint32_t destination = (uint32_t)(&buffer_);
    Channel::channel().SRCADDR0 = (source >> 0) & 0xff;
    Channel::channel().SRCADDR1 = (source >> 8) & 0xff;
    Channel::channel().SRCADDR2 = (source >> 16) & 0xff;
    Channel::channel().DESTADDR0 = (destination >> 0) & 0xff;
    Channel::channel().DESTADDR1 = (destination >> 8) & 0xff;
    Channel::channel().DESTADDR2 = (destination >> 16) & 0xff;
    Channel::channel().TRIGSRC = Peripheral::dma_rx_trigger();
    Channel::channel().CTRLA = DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm;
    if (sizeof(Value) == 2) {
      Channel::channel().CTRLA |= 0x01;
    } else if (sizeof(Value) == 4) {
      Channel::channel().CTRLA |= 0x02;
    } else if (sizeof(Value) == 8) {
      Channel::channel().CTRLA |= 0x03;
    }
  }
  
  static inline uint8_t readable() {
    uint8_t write_ptr = buffer_size - \
        Channel::channel().TRFCNTL / sizeof(Value);
    return (write_ptr - read_ptr_) & (buffer_size - 1);
  }
  
  static inline Value ImmediateRead() {
    Value result = buffer_[read_ptr_];
    read_ptr_ = (read_ptr_ + 1) & (buffer_size - 1);
    return result;
  }
  
  static inline Value Read() {
    while (!readable());
    return ImmediateRead();
  }
  
  static inline void Start() {
    Channel::channel().CTRLA |= DMA_CH_ENABLE_bm;
  }
  
  static inline void Stop() {
    Channel::channel().CTRLA &= ~DMA_CH_ENABLE_bm;
  }
  
 private:
  static uint8_t read_ptr_;
  static Value buffer_[buffer_size];
  
  DISALLOW_COPY_AND_ASSIGN(DMARxBuffer);
};

template<typename P, uint8_t c, uint16_t b> \
uint8_t DMARxBuffer<P, c, b>::read_ptr_ = 0;
template<typename P, uint8_t c, uint16_t b> \
typename P::Value DMARxBuffer<P, c, b>::buffer_[b];


// Uses DMA to automatically feed data to a peripheral.
template<
    uint8_t channel_index,
    uint16_t buffer_size,
    typename Destination,
    typename Trigger,
    bool one_shot = false>
class DMATxBuffer {
 private:
  typedef DMAChannel<channel_index> Channel;
  typedef typename Destination::Value Value;

 public:
  DMATxBuffer() { }
   
  static inline void Init() {
    DMA_CTRL = 0x80;
    uint8_t address_control = 0;
    address_control |= DMA_CH_SRCRELOAD_BLOCK_gc;
    address_control |= DMA_CH_DESTRELOAD_BURST_gc;
    address_control |= DMA_CH_SRCDIR_INC_gc;
    address_control |= DMA_CH_DESTDIR_INC_gc;
    
    Channel::channel().ADDRCTRL = address_control;
    Channel::channel().REPCNT = one_shot ? 1 : 0;
    Channel::channel().TRFCNT = buffer_size * sizeof(Value);
    
    uint32_t source = (uint32_t)(&buffer_);
    uint32_t destination = (uint32_t)(Destination::dma_data());
    Channel::channel().SRCADDR0 = (source >> 0) & 0xff;
    Channel::channel().SRCADDR1 = (source >> 8) & 0xff;
    Channel::channel().SRCADDR2 = (source >> 16) & 0xff;
    Channel::channel().DESTADDR0 = (destination >> 0) & 0xff;
    Channel::channel().DESTADDR1 = (destination >> 8) & 0xff;
    Channel::channel().DESTADDR2 = (destination >> 16) & 0xff;
    Channel::channel().TRIGSRC = Trigger::dma_tx_trigger();
    Channel::channel().CTRLA = DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm;
    if (sizeof(Value) == 2) {
      Channel::channel().CTRLA |= DMA_CH_BURSTLEN_2BYTE_gc;
    } else if (sizeof(Value) == 4) {
      Channel::channel().CTRLA |= DMA_CH_BURSTLEN_4BYTE_gc;
    } else if (sizeof(Value) == 8) {
      Channel::channel().CTRLA |= DMA_CH_BURSTLEN_8BYTE_gc;
    }
  }
  
  static inline uint8_t writable() {
    uint8_t read_ptr = buffer_size - Channel::channel().TRFCNTL / sizeof(Value);
    return (read_ptr - write_ptr_ - 1) & (buffer_size - 1);
  }
  
  static inline void Write(Value v) {
    while (!writable());
    Overwrite(v);
  }
  
  static inline void Overwrite(Value v) {
    uint8_t w = write_ptr_;
    buffer_[w] = v;
    write_ptr_ = (w + 1) & (buffer_size - 1);
  }
  
  static inline void Start() {
    Channel::channel().CTRLA |= DMA_CH_ENABLE_bm;
  }
  
  static inline void Stop() {
    Channel::channel().CTRLA &= ~DMA_CH_ENABLE_bm;
  }
  
 private:
  static uint8_t write_ptr_;
  static Value buffer_[buffer_size];
  
  DISALLOW_COPY_AND_ASSIGN(DMATxBuffer);
};

template<uint8_t c, uint16_t b, typename D, typename T, bool o> \
uint8_t DMATxBuffer<c, b, D, T, o>::write_ptr_ = 0;
template<uint8_t c, uint16_t b, typename D, typename T, bool o> \
typename D::Value DMATxBuffer<c, b, D, T, o>::buffer_[b];

}  // namespace avrlibx

#endif   // AVRLIBX_IO_DMA_H_
