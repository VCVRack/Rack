// Copyright 2010 Olivier Gillet.
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
// Implementation of the I2C protocol (Master mode only for now).
//
// Note that this file is not in the hal directory directly because I don't want
// the interrupt handler code for TWI to be linked with every project.

#ifndef AVRLIB_I2C_I2C_H_
#define AVRLIB_I2C_I2C_H_

#include <util/twi.h>

#include "avrlib/gpio.h"
#include "avrlib/avrlib.h"
#include "avrlib/ring_buffer.h"

namespace avrlib {

IORegister(TWCR);
IORegister(TWSR);
typedef BitInRegister<TWSRRegister, TWPS0> Prescaler0;
typedef BitInRegister<TWSRRegister, TWPS1> Prescaler1;
typedef BitInRegister<TWCRRegister, TWEN> I2cEnable;
typedef BitInRegister<TWCRRegister, TWIE> I2cInterrupt;
typedef BitInRegister<TWCRRegister, TWEA> I2cAck;
typedef BitInRegister<TWCRRegister, TWSTA> I2cStart;
typedef BitInRegister<TWCRRegister, TWSTO> I2cStop;

enum I2cState {
  I2C_STATE_READY,
  I2C_STATE_TRANSMITTING,
  I2C_STATE_RECEIVING,
};

enum I2cError {
  I2C_ERROR_NONE = 0xff,
  I2C_ERROR_NO_ACK_FOR_ADDRESS = 0x01,
  I2C_ERROR_NO_ACK_FOR_DATA = TW_MT_SLA_NACK,
  I2C_ERROR_ARBITRATION_LOST = TW_MT_DATA_NACK,
  I2C_ERROR_BUS_ERROR = 0xfe,
  I2C_ERROR_TIMEOUT = 0x02,
};

template<uint8_t output_buffer_size = 4>
class I2cOutput {
 public:
  I2cOutput() { }
  enum {
    buffer_size = output_buffer_size,
    data_size = 8
  };
  typedef typename DataTypeForSize<data_size>::Type Value;
};

template<uint8_t input_buffer_size = 4>
class I2cInput {
 public:
  I2cInput() { }
  enum {
    buffer_size = input_buffer_size,
    data_size = 8
  };
  typedef typename DataTypeForSize<data_size>::Type Value;
};

// I2C Handler.
extern void (*i2c_handler_)();

template<uint8_t input_buffer_size = 16,
         uint8_t output_buffer_size = 16,
         uint32_t frequency = 100000 /* Hz */>
class I2cMaster {
 public:
  I2cMaster() { }

  typedef typename DataTypeForSize<I2cInput<0>::data_size>::Type Value;

  static void Init() {
    // Prescaler is set to a factor of 1.
    Prescaler0::clear();
    Prescaler1::clear();
    TWBR = (F_CPU / frequency - 16) / 2;

    I2cEnable::set();
    I2cInterrupt::set();
    I2cAck::set();

    state_ = I2C_STATE_READY;
    i2c_handler_ = &Handler;
  }
  
  static void Done() {
    I2cInterrupt::clear();
    I2cEnable::clear();
    I2cAck::clear();
    i2c_handler_ = NULL;
  }

  static uint8_t Wait() {
    while (state_ != I2C_STATE_READY) { }
    return error_;
  }
  
  static uint8_t Wait(uint16_t num_cycles) {
    while (state_ != I2C_STATE_READY && num_cycles) {
      --num_cycles;
    }
    if (!num_cycles) {
      error_ = I2C_ERROR_TIMEOUT;
    }
    return error_;
  }

  static uint8_t Send(uint8_t address) {
    // The output buffer is empty, no need to do anything.
    if (!Output::readable()) {
      return 0;
    }

    // Sorry, data can be sent only when the line is not busy.
    if (state_ != I2C_STATE_READY) {
      return 0;
    }

    error_ = I2C_ERROR_NONE;
    state_ = I2C_STATE_RECEIVING;
    slarw_ = (address << 1) | TW_WRITE;

    uint8_t size = Output::readable();
    I2cStart::set();

    return size;
  }

  static uint8_t Request(uint8_t address, uint8_t requested) {
    // Make sure that we don't request more than the buffer can hold.
    if (requested >= Input::writable()) {
      requested = Input::writable() - 1;
    }
    // Sorry, data can be requested only when the line is not busy.
    if (state_ != I2C_STATE_READY) {
      return 0;
    }

    error_ = I2C_ERROR_NONE;
    state_ = I2C_STATE_RECEIVING;
    slarw_ = (address << 1) | TW_READ;
    received_ = 0;
    requested_ = requested;
    I2cStart::set();
    
    return requested;
  }

  // All the read/write operations are done on the buffer, so they do not
  // block.
  static inline void Write(Value v) { Output::Write(v); }
  static inline uint8_t writable() { return Output::writable(); }
  static inline uint8_t NonBlockingWrite(Value v) {
    return Output::NonBlockingWrite(v);
  }
  static inline void Overwrite(Value v) { Output::Overwrite(v); }
  static inline Value Read() { return Input::Read(); }
  static inline uint8_t readable() { return Input::readable(); }
  static inline int16_t NonBlockingRead() { return Input::NonBlockingRead(); }
  static inline Value ImmediateRead() { return Input::ImmediateRead(); }

  static inline void FlushInputBuffer() { Input::Flush(); }
  static inline void FlushOutputBuffer() { Output::Flush(); }

 private:
  static inline void Continue(uint8_t ack) {
    if (ack) {
      TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
    } else {
      TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    }
  }

  static inline void Stop() {
    I2cStop::set();
    while (I2cStop::value()) { }
    state_ = I2C_STATE_READY;
  }

  static inline void Abort() {
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);
    state_ = I2C_STATE_READY;
  }

  static void Handler() {
    switch (TW_STATUS) {
      case TW_START:
      case TW_REP_START:
        TWDR = slarw_;
        Continue(1);
        break;

      case TW_MT_DATA_ACK:
      case TW_MT_SLA_ACK:
        if (Output::readable()) {
          TWDR = Output::ImmediateRead();
          Continue(1);
        } else {
          Stop();
        }
        break;

      case TW_MT_SLA_NACK:
      case TW_MT_DATA_NACK:
        error_ = TW_STATUS;
        Stop();
        break;

      case TW_MT_ARB_LOST:
        error_ = I2C_ERROR_ARBITRATION_LOST;
        Abort();
        break;

      case TW_MR_DATA_ACK:
        Input::Overwrite(TWDR);
        ++received_;
      case TW_MR_SLA_ACK:
        if (received_ < requested_) {
          Continue(1);
        } else {
          Continue(0);
        }
        break;

      case TW_MR_DATA_NACK:
        Input::Overwrite(TWDR);
        ++received_;
      case TW_MR_SLA_NACK:
        Stop();
        break;

      case TW_NO_INFO:
        break;

      case TW_BUS_ERROR:
        error_ = I2C_ERROR_BUS_ERROR;
        Stop();
        break;
    }
  }

public:
  typedef RingBuffer<I2cInput<input_buffer_size> > Input;
  typedef RingBuffer<I2cOutput<output_buffer_size> > Output;

private:
  static volatile uint8_t state_;
  static volatile uint8_t error_;
  static volatile uint8_t slarw_;
  static volatile uint8_t received_;
  static uint8_t requested_;

  DISALLOW_COPY_AND_ASSIGN(I2cMaster);
};

/* static */
template<uint8_t input_buffer_size, uint8_t output_buffer_size,
         uint32_t frequency>
volatile uint8_t I2cMaster<input_buffer_size, output_buffer_size,
                           frequency>::state_;

/* static */
template<uint8_t input_buffer_size, uint8_t output_buffer_size,
         uint32_t frequency>
volatile uint8_t I2cMaster<input_buffer_size, output_buffer_size,
                           frequency>::error_;

/* static */
template<uint8_t input_buffer_size, uint8_t output_buffer_size,
         uint32_t frequency>
volatile uint8_t I2cMaster<input_buffer_size, output_buffer_size,
                           frequency>::slarw_;

/* static */
template<uint8_t input_buffer_size, uint8_t output_buffer_size,
         uint32_t frequency>
volatile uint8_t I2cMaster<input_buffer_size, output_buffer_size, frequency>::received_;

/* static */
template<uint8_t input_buffer_size, uint8_t output_buffer_size,
         uint32_t frequency>
uint8_t I2cMaster<input_buffer_size, output_buffer_size, frequency>::requested_;

}  // namespace avrlib

#endif   // AVRLIB_I2C_I2C_H_
