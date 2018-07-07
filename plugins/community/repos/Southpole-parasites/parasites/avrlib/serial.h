// Copyright 2009 Olivier Gillet.
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
// Fast serial (for the onboard UART), using compile time optimizations.
//
// Can be used in buffered mode, or in polled mode (to save space or avoid
// interruptions if there is an even higher priority task than the serial I/O).
//
// Usage:
//
// Set-up:
// typedef Serial<SerialPort0, 31250, BUFFERED, POLLED> Serial;
// then, in the setup() hook: Serial::Init()
//
// Write:
// Serial::Write(40)  // Will block until data is written.
// write_has_succeeded = Serial::NonBlockingWrite(40)  // Will not block.
//
// Buffer manipulation (for buffered I/O):
// Serial::available()  // Number of bytes ready to be read. For polled read too
// my_value = Serial::Read()  // Will wait until data arrives.
// my_value = Serial::NonBlockingRead()  // Will return -1 if no data is there.
// my_value = Serial::ImmediateRead()  // Assumes you are sure about what you
//   are doing and you know that data is here.
//
// Flushing a buffer:
// Serial::InputBuffer::Flush()
//
// TODO(pichenettes): Buffered writes not supported for now (should look up
// the right interrupt handler).

#ifndef AVRLIB_SERIAL_H_
#define AVRLIB_SERIAL_H_

#include "avrlib/avrlib.h"
#include "avrlib/gpio.h"
#include "avrlib/ring_buffer.h"

namespace avrlib {

const uint8_t kSerialOutputBufferSize = 32;
const uint8_t kSerialInputBufferSize = 32;

// Low-level interface to the low-level UART registers. Several specializations
// may be declared for each serial port. This class could theoretically be used
// for non-blocking write or polling reads.
template<typename TxEnableBit, typename TxReadyBit,
         typename RxEnableBit, typename RxReadyBit,
         typename RxInterruptBit,
         typename TurboBit,
         typename PrescalerRegisterH, typename PrescalerRegisterL,
         typename DataRegister,
         uint8_t input_buffer_size_,
         uint8_t output_buffer_size_>
struct SerialPort {
  typedef TxEnableBit Tx;
  typedef RxEnableBit Rx;
  typedef RxInterruptBit RxInterrupt;
  typedef TurboBit Turbo;
  enum {
    input_buffer_size = input_buffer_size_,
    output_buffer_size = output_buffer_size_
  };
  static inline void set_prescaler(uint16_t value) {
    *PrescalerRegisterH::ptr() = value >> 8;
    *PrescalerRegisterL::ptr() = value;
  }
  static inline uint8_t tx_ready() { return TxReadyBit::value(); }
  static inline uint8_t rx_ready() { return RxReadyBit::value(); }
  static inline uint8_t data() { return *DataRegister::ptr(); }
  static inline void set_data(uint8_t value) { *DataRegister::ptr() = value; }
};

template<typename SerialPort>
struct SerialInput : public Input {
  enum {
    buffer_size = SerialPort::input_buffer_size,
    data_size = 8
  };
  typedef uint8_t Value;

  // Blocking!
  static inline Value Read() { while (!readable()); return ImmediateRead(); }

  // Number of bytes available for read.
  static inline uint8_t readable() { return SerialPort::rx_ready(); }

  // A byte, or -1 if reading failed.
  static inline int16_t NonBlockingRead() { return readable() ? Read() : -1; }

  // No check for ready state.
  static inline Value ImmediateRead() { return SerialPort::data(); }

  // Called in data reception interrupt.
  static inline void Received() {
    if (!readable()) {
       return;
    }
    // This will discard data if the buffer is full.
    RingBuffer<SerialInput<SerialPort> >::NonBlockingWrite(ImmediateRead());
  }
};

template<typename SerialPort>
struct SerialOutput : public Output {
  enum {
    buffer_size = SerialPort::output_buffer_size,
    data_size = 8
  };
  typedef uint8_t Value;

  // Blocking!
  static inline void Write(Value v) { while (!writable()); Overwrite(v); }

  // Number of bytes that can be fed.
  static inline uint8_t writable() { return SerialPort::tx_ready(); }

  // 1 if success.
  static inline uint8_t NonBlockingWrite(Value v) {
    if (!writable()) {
      return 0;
    }
    Overwrite(v);
    return 1;
  } 

  // No check for ready state.
  static inline void Overwrite(Value v) { SerialPort::set_data(v); }

  // Called in data emission interrupt.
  static inline Value Requested() {
    Value v = RingBuffer<SerialOutput<SerialPort> >::NonBlockingRead();
    if (v >= 0) {
      Overwrite(v);
    }
  }
};

template<typename SerialPort, PortMode input = POLLED, PortMode output = POLLED>
struct SerialImplementation { };

template<typename SerialPort>
struct SerialImplementation<SerialPort, DISABLED, DISABLED> {
  typedef InputOutput<DisabledInput, DisabledOutput> IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, DISABLED, POLLED> {
  typedef InputOutput<DisabledInput, SerialOutput<SerialPort> > IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, DISABLED, BUFFERED> {
  typedef RingBuffer<SerialOutput<SerialPort> > OutputBuffer;
  typedef InputOutput<DisabledInput, OutputBuffer > IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, POLLED, DISABLED> {
  typedef InputOutput<SerialInput<SerialPort>, DisabledOutput> IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, POLLED, POLLED> {
  typedef InputOutput<SerialInput<SerialPort>, SerialOutput<SerialPort> > IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, POLLED, BUFFERED> {
  typedef RingBuffer<SerialOutput<SerialPort> > OutputBuffer;
  typedef InputOutput<SerialInput<SerialPort>, OutputBuffer> IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, BUFFERED, DISABLED> {
  typedef RingBuffer<SerialInput<SerialPort> > InputBuffer;
  typedef InputOutput<InputBuffer, DisabledOutput> IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, BUFFERED, POLLED> {
  typedef RingBuffer<SerialInput<SerialPort> > InputBuffer;
  typedef InputOutput<InputBuffer, SerialOutput<SerialPort> > IO;
};
template<typename SerialPort>
struct SerialImplementation<SerialPort, BUFFERED, BUFFERED> {
  typedef RingBuffer<SerialInput<SerialPort> > InputBuffer;
  typedef RingBuffer<SerialOutput<SerialPort> > OutputBuffer;
  typedef InputOutput<InputBuffer, OutputBuffer> IO;
};

template<typename SerialPort, uint32_t baud_rate, PortMode input = POLLED,
         PortMode output = POLLED, bool turbo = false>
struct Serial {
  typedef SerialImplementation<SerialPort, input, output> Impl;
  typedef uint8_t Value;
  typedef typename Impl::IO::Input Input;
  typedef typename Impl::IO::Output Output;
  static inline void Init() {
    Init<baud_rate>();
  }
  template<uint32_t new_baud_rate>
  static inline void Init() {
    if (turbo) {
      SerialPort::Turbo::set();
      uint16_t prescaler = F_CPU / (8L * baud_rate) - 1;
      SerialPort::set_prescaler(prescaler);
    } else {
      SerialPort::Turbo::clear();
      uint16_t prescaler = F_CPU / (16 * baud_rate) - 1;
      SerialPort::set_prescaler(prescaler);
    }
    if (output != DISABLED) {
      SerialPort::Tx::set();
    }
    if (input != DISABLED) {
      SerialPort::Rx::set();
    }
    if (input == BUFFERED) {
      SerialPort::RxInterrupt::set();
    }
  }
  
  static inline void Disable() {
    SerialPort::Tx::clear();
    SerialPort::Rx::clear();
    SerialPort::RxInterrupt::clear();
  }
  
  static inline void Write(Value v) { Impl::IO::Write(v); }
  static inline uint8_t writable() { return Impl::IO::writable(); }
  static inline uint8_t NonBlockingWrite(Value v) {
    return Impl::IO::NonBlockingWrite(v);
  }
  static inline void Overwrite(Value v) { Impl::IO::Overwrite(v); }
  static inline Value Read() { return Impl::IO::Read(); }
  static inline uint8_t readable() { return Impl::IO::readable(); }
  static inline int16_t NonBlockingRead() {
    return Impl::IO::NonBlockingRead();
  }
  static inline Value ImmediateRead() { return Impl::IO::ImmediateRead(); }
};


#ifdef HAS_USART0

IORegister(UBRR0H);
IORegister(UBRR0L);
IORegister16(UBRR0);
IORegister(UCSR0A);
IORegister(UCSR0B);
IORegister(UCSR0C);
IORegister(UDR0);

typedef SerialPort<
    BitInRegister<UCSR0BRegister, TXEN0>,
    BitInRegister<UCSR0ARegister, UDRE0>,
    BitInRegister<UCSR0BRegister, RXEN0>,
    BitInRegister<UCSR0ARegister, RXC0>,
    BitInRegister<UCSR0BRegister, RXCIE0>,
    BitInRegister<UCSR0ARegister, U2X0>,
    UBRR0HRegister,
    UBRR0LRegister,
    UDR0Register,
    kSerialOutputBufferSize,
    kSerialInputBufferSize> SerialPort0;

#endif  // #ifdef HAS_USART0


#ifdef HAS_USART1

IORegister(UBRR1H);
IORegister(UBRR1L);
IORegister16(UBRR1);
IORegister(UCSR1A);
IORegister(UCSR1B);
IORegister(UCSR1C);
IORegister(UDR1);

typedef SerialPort<
    BitInRegister<UCSR1BRegister, TXEN1>,
    BitInRegister<UCSR1ARegister, UDRE1>,
    BitInRegister<UCSR1BRegister, RXEN1>,
    BitInRegister<UCSR1ARegister, RXC1>,
    BitInRegister<UCSR1BRegister, RXCIE1>,
    BitInRegister<UCSR1ARegister, U2X1>,
    UBRR1HRegister,
    UBRR1LRegister,
    UDR1Register,
    kSerialOutputBufferSize,
    kSerialInputBufferSize> SerialPort1;

#endif  // #ifdef HAS_USART1


#ifdef HAS_USART2

IORegister(UBRR2H);
IORegister(UBRR2L);
IORegister16(UBRR2);
IORegister(UCSR2A);
IORegister(UCSR2B);
IORegister(UCSR2C);
IORegister(UDR2);

typedef SerialPort<
    BitInRegister<UCSR2BRegister, TXEN2>,
    BitInRegister<UCSR2ARegister, UDRE2>,
    BitInRegister<UCSR2BRegister, RXEN2>,
    BitInRegister<UCSR2ARegister, RXC2>,
    BitInRegister<UCSR2BRegister, RXCIE2>,
    BitInRegister<UCSR2ARegister, U2X2>,
    UBRR2HRegister,
    UBRR2LRegister,
    UDR2Register,
    kSerialOutputBufferSize,
    kSerialInputBufferSize> SerialPort2;

#endif  // #ifdef HAS_USART2

#ifdef HAS_USART3

IORegister(UBRR3H);
IORegister(UBRR3L);
IORegister16(UBRR3);
IORegister(UCSR3A);
IORegister(UCSR3B);
IORegister(UCSR3C);
IORegister(UDR3);

typedef SerialPort<
    BitInRegister<UCSR3BRegister, TXEN3>,
    BitInRegister<UCSR3ARegister, UDRE3>,
    BitInRegister<UCSR3BRegister, RXEN3>,
    BitInRegister<UCSR3ARegister, RXC3>,
    BitInRegister<UCSR3BRegister, RXCIE3>,
    BitInRegister<UCSR3ARegister, U2X3>,
    UBRR3HRegister,
    UBRR3LRegister,
    UDR3Register,
    kSerialOutputBufferSize,
    kSerialInputBufferSize> SerialPort3;

#endif  // #ifdef HAS_USART3

}  // namespace avrlib


#endif  // AVRLIB_SERIAL_H_
