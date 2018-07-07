// Copyright 2012 Olivier Gillet.
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

#ifndef AVRLIBX_IO_TWI_H_
#define AVRLIBX_IO_TWI_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/io/gpio.h"
#include "avrlibx/io/ring_buffer.h"

namespace avrlibx {

template<typename Port> struct TwiWrapper { };

#define WRAP_TWI(port) \
template<> \
struct TwiWrapper<Port ## port> { \
  static inline TWI_t& twi() { \
    return TWI ## port; \
  } \
  static inline TWI_MASTER_t& master() { \
    return twi().MASTER; \
  } \
  static inline TWI_SLAVE_t& slave() { \
    return twi().SLAVE; \
  } \
  static inline void Handler() { \
    if (handler_) { \
      (*handler_)(); \
    } \
  } \
  static inline void set_interrupt_handler(void (*handler)()) { \
    handler_ = handler; \
  } \
  static void (*handler_)(); \
};

enum TWIState {
  TWI_STATE_READY,
  TWI_STATE_TRANSMITTING,
  TWI_STATE_RECEIVING,
};

enum TWIError {
  TWI_ERROR_NONE = 0,
  TWI_ERROR_NO_ACK = 1,
  TWI_ERROR_ARBITRATION_LOST = 2,
  TWI_ERROR_BUS_ERROR = 3,
  TWI_ERROR_UNEXPECTED_STATE = 4
};

WRAP_TWI(C)
WRAP_TWI(E)

template<typename Port, uint8_t size = 16>
class TwiOutputBufferSpecs {
 public:
  TwiOutputBufferSpecs() { }
  enum {
    buffer_size = size,
    data_size = 8
  };
  typedef uint8_t Value;
};

template<typename Port, uint8_t size = 16>
class TwiInputBufferSpecs {
 public:
  TwiInputBufferSpecs() { }
  enum {
    buffer_size = size,
    data_size = 8
  };
  typedef uint8_t Value;
};

template<
    typename Port,
    uint32_t frequency = 100000, /* Hz */
    uint8_t input_buffer_size = 16,
    uint8_t output_buffer_size = 16,
    uint8_t int_level = 1>
struct TwiMaster {
  typedef TwiWrapper<Port> Twi;
  
  static inline void Init() {
    Twi::set_interrupt_handler(&InterruptHandler);
    Twi::twi().CTRL = 0;
    Twi::master().BAUD = F_CPU / (2 * frequency) - 5;
    Twi::master().CTRLA = (int_level << 6) \
      | TWI_MASTER_RIEN_bm \
      | TWI_MASTER_WIEN_bm \
      | TWI_MASTER_ENABLE_bm;
    Twi::master().STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
    state_ = TWI_STATE_READY;
    
  }
  
  static uint8_t Wait() {
    while (state_ != TWI_STATE_READY) { }
    return error_;
  }
  
  static void Done() {
    Twi::master().CTRLA = 0;
    Twi::set_interrupt_handler(NULL);
  }
  
  static uint8_t Request(uint8_t address, uint8_t requested) {
    // Make sure that we don't request more than the buffer can hold.
    if (requested >= Input::writable()) {
      return 0;
    }
    if (state_ != TWI_STATE_READY) {
      return 0;
    }
    
    error_ = TWI_ERROR_NONE;
    state_ = TWI_STATE_RECEIVING;
    slarw_ = (address << 1) | 1;
    received_ = 0;
    requested_ = requested;
    Twi::master().ADDR = slarw_;
    return requested;
  }
  
  static uint8_t Send(uint8_t address) {
    if (!Output::readable()) {
      return 0;
    }
    if (state_ != TWI_STATE_READY) {
      return 0;
    }
    error_ = TWI_ERROR_NONE;
    state_ = TWI_STATE_TRANSMITTING;
    slarw_ = (address << 1) | 0;

    uint8_t sent = Output::readable();
    Twi::master().ADDR = slarw_;
    return sent;
  }
  
  // All the read/write operations are done on the buffer, so they do not
  // block.
  static inline void Write(uint8_t v) { Output::Write(v); }
  static inline uint8_t writable() { return Output::writable(); }
  static inline uint8_t NonBlockingWrite(uint8_t v) {
    return Output::NonBlockingWrite(v);
  }
  static inline void Overwrite(uint8_t v) { Output::Overwrite(v); }
  static inline uint8_t Read() { return Input::Read(); }
  static inline uint8_t readable() { return Input::readable(); }
  static inline int16_t NonBlockingRead() { return Input::NonBlockingRead(); }
  static inline uint8_t ImmediateRead() { return Input::ImmediateRead(); }

  static inline void FlushInputBuffer() { Input::Flush(); }
  static inline void FlushOutputBuffer() { Output::Flush(); }

 private:
  static void InterruptHandler() {
    uint8_t current_status = Twi::master().STATUS;
    if ((current_status & TWI_MASTER_ARBLOST_bm) ||
        (current_status & TWI_MASTER_BUSERR_bm)) {
      // Arbitration lost or bus error.
      error_ = current_status & TWI_MASTER_ARBLOST_bm
          ? TWI_ERROR_ARBITRATION_LOST
          : TWI_ERROR_BUS_ERROR;
      state_ = TWI_STATE_READY;
    } else if (current_status & TWI_MASTER_WIF_bm) {
      // Ready for write.
      // Check if slave has NACK'ed.
      if (current_status & TWI_MASTER_RXACK_bm) {
        error_ = TWI_ERROR_NO_ACK;
        state_ = TWI_STATE_READY;
        Twi::master().CTRLC = TWI_MASTER_CMD_STOP_gc;
      } else if (Output::readable()) {
        Twi::master().DATA = Output::ImmediateRead();
      } else if (received_ < requested_) {
        Twi::master().ADDR = slarw_;
      } else {
        Twi::master().CTRLC = TWI_MASTER_CMD_STOP_gc;
        state_ = TWI_STATE_READY;
      }
    } else if (current_status & TWI_MASTER_RIF_bm) {
      Input::Overwrite(Twi::master().DATA);
      ++received_;
      if (received_ < requested_) {
        Twi::master().CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
      } else {
        Twi::master().CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
        state_ = TWI_STATE_READY;
      }
    } else {
      error_ = TWI_ERROR_UNEXPECTED_STATE;
      state_ = TWI_STATE_READY;
    }
  }
 
 public:
  typedef RingBuffer<TwiInputBufferSpecs<Port, input_buffer_size> > Input;
  typedef RingBuffer<TwiOutputBufferSpecs<Port, output_buffer_size> > Output;

 private:
  static volatile uint8_t state_;
  static volatile uint8_t error_;
  static volatile uint8_t slarw_;
  static volatile uint8_t received_;
  static uint8_t requested_;

  DISALLOW_COPY_AND_ASSIGN(TwiMaster);
};

/* static */

template<typename P, uint32_t f, uint8_t is, uint8_t os, uint8_t il>
volatile uint8_t TwiMaster<P, f, is, os, il>::state_;

/* static */
template<typename P, uint32_t f, uint8_t is, uint8_t os, uint8_t il>
volatile uint8_t TwiMaster<P, f, is, os, il>::error_;

/* static */
template<typename P, uint32_t f, uint8_t is, uint8_t os, uint8_t il>
volatile uint8_t TwiMaster<P, f, is, os, il>::slarw_;

/* static */
template<typename P, uint32_t f, uint8_t is, uint8_t os, uint8_t il>
volatile uint8_t TwiMaster<P, f, is, os, il>::received_;

/* static */
template<typename P, uint32_t f, uint8_t is, uint8_t os, uint8_t il>
uint8_t TwiMaster<P, f, is, os, il>::requested_;

}  // namespace avrlibx

#endif   // AVRLIBX_IO_TWI_H_
