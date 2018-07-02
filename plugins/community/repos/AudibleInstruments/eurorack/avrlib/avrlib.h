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
// Important: All buffer sizes are expected to be less than 256! (fit in 8
// bits), and must be powers of 2.

#ifndef AVRLIB_AVRLIB_H_
#define AVRLIB_AVRLIB_H_

#include <avr/io.h>

#include "avrlib/base.h"
#include "avrlib/size_to_type.h"

namespace avrlib {

enum DataOrder {
  MSB_FIRST = 0,
  LSB_FIRST = 1
};

enum DigitalValue {
  LOW = 0,
  HIGH = 1
};

// <avr/io.h> is full of useful defines, but they cannot be used as template
// arguments because they are of the form: (*(volatile uint8_t *)(0x80))
// The following define wraps this reference into a class to make it easier to
// pass it as a template argument.
#define IORegister(reg) struct reg##Register { \
  static volatile uint8_t* ptr() { return &reg; } \
  reg##Register& operator=(const uint8_t& value) { *ptr() = value; } \
  uint8_t operator()(const uint8_t& value) { return *ptr(); } \
};

#define IORegister16(reg) struct reg##Register { \
  static volatile uint16_t* ptr() { return &reg; } \
  reg##Register& operator=(const uint16_t& value) { *ptr() = value; } \
  uint16_t operator()(const uint16_t& value) { return *ptr(); } \
};

#define SpecialFunctionRegister(reg) struct reg##Register { \
  static volatile uint8_t* ptr() { return &_SFR_BYTE(reg); } \
  reg##Register& operator=(const uint8_t& value) { *ptr() = value; } \
  uint8_t operator()(const uint8_t& value) { return *ptr(); } \
};

#define SpecialFunctionRegister16(reg) struct reg##Register { \
  static volatile uint16_t* ptr() { return &_SFR_WORD(reg); } \
  reg##Register& operator=(const uint16_t& value) { *ptr() = value; } \
  uint16_t operator()(const uint16_t& value) { return *ptr(); } \
};


// Represents a bit in an i/o port register.
template<typename Register, uint8_t bit>
struct BitInRegister {
  static void clear() {
    *Register::ptr() &= ~_BV(bit);
  }
  static void set() {
    *Register::ptr() |= _BV(bit);
  }
  static void toggle() {
    *Register::ptr() ^= _BV(bit);
  }
  static uint8_t value() {
    return *Register::ptr() & _BV(bit) ? 1 : 0;
  }
};

// These classes implement/define the basic input/output interface. The default
// implementation is that of an infinite stream of incoming/outgoing 0s.
struct Input {
  enum {
    buffer_size = 0,  // Recommended buffer size, when using buffered input.
    data_size = 0,  // 0 for disabled port, 1 for digital, 8 for byte.
  };
  typedef uint8_t Value;

  // Blocking!
  static inline Value Read() { while (!readable()); return ImmediateRead(); }

  // Number of bytes available for read.
  static inline uint8_t readable() { return 1; }

  // A byte, or -1 if reading failed.
  static inline int16_t NonBlockingRead() { return readable() ? Read() : -1; }

  // No check for ready state.
  static inline Value ImmediateRead() { return 0; }

  // Called in data reception interrupt.
  static inline void Received() { }
};

struct Output {
  enum {
    buffer_size = 0,  // Recommended buffer size, when using buffered output.
    data_size = 0  // 0 for disabled port, 1 for digital, 8 for byte.
  };
  typedef uint8_t Value;

  // Blocking!
  static inline void Write(Value v) { while (!writable()); Overwrite(v); }

  // Number of bytes that can be fed.
  static inline uint8_t writable() { return 1; }

  // 1 if success.
  static inline uint8_t NonBlockingWrite(Value v) {
    if (!writable()) {
      return 0;
    }
    Overwrite(v);
    return 1;
  } 

  // No check for ready state.
  static inline void Overwrite(Value) { return; }

  // Called in data emission interrupt.
  static inline Value Requested() { return 0; }
};

// An object capable both of input and output, composed from an Input and an
// Output implementation.
template<typename I, typename O>
struct InputOutput {
  typedef I Input;
  typedef O Output;

  static inline void Write(typename O::Value v) { O::Write(v); }
  static inline uint8_t writable() { return O::writable(); }
  static inline uint8_t NonBlockingWrite(typename O::Value v ) {
    return O::NonBlockingWrite(v);
  }
  static inline void Overwrite(typename O::Value v) { O::Overwrite(v); }
  static inline typename O::Value Requested() { return O::Requested(); }
  static inline typename I::Value Read() { return I::Read(); }
  static inline uint8_t readable() { return I::readable(); }
  static inline int16_t NonBlockingRead() { return I::NonBlockingRead(); }
  static inline typename I::Value ImmediateRead() { return I::ImmediateRead(); }
  static inline void Received() { I::Received(); }
};


// Dummy class that can be passed whenever we expect Input/Output types, and
// which do not perform any IO.
typedef Input DisabledInput;
typedef Output DisabledOutput;
typedef InputOutput<DisabledInput, DisabledOutput> DisabledInputOutput;

enum PortMode {
  DISABLED = 0,
  POLLED = 1,
  BUFFERED = 2
};

// Some classes (SPI, shift register) have a notion of communication session -
// Begin is called, several R/W are done, and then End is called to pull high
// a chip select or latch line. This template ensures that any path leaving a
// block of code will release the resource.
template<typename T>
class scoped_resource {
 public:
  scoped_resource() {
    T::Begin();
  }
  
  ~scoped_resource() {
    T::End();
  }
};

}  // namespace avrlib

#endif   // AVRLIB_AVRLIB_H_
