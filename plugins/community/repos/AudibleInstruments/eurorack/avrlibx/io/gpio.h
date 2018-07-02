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

#ifndef AVRLIBX_IO_GPIO_H_
#define AVRLIBX_IO_GPIO_H_

#include <avr/io.h>

#include "avrlibx/avrlibx.h"

namespace avrlibx {

enum PortDirection {
  INPUT,
  OUTPUT
};

enum PortMode {
  PORT_MODE_TOTEM_POLE,
  PORT_MODE_BUS_KEEPER,
  PORT_MODE_PULL_UP,
  PORT_MODE_PULL_DOWN
};

enum SenseMode {
  SENSE_MODE_BOTH_EDGES,
  SENSE_MODE_RISING,
  SENSE_MODE_FALLING,
  SENSE_MODE_LOW_LEVEL
};

#define WRAP_PORT(letter) \
struct Port ## letter { \
  static inline PORT_t& port_t() { return PORT ## letter; } \
  static inline void dir(uint8_t value) { PORT ## letter ## _DIR = value; } \
  static inline void dir_clr(uint8_t value) { PORT ## letter ## _DIRCLR = value; } \
  static inline void dir_set(uint8_t value) { PORT ## letter ## _DIRSET = value; } \
  static inline void out(uint8_t value) { PORT ## letter ## _OUT = value; } \
  static inline void out_set(uint8_t value) { PORT ## letter ## _OUTSET = value; } \
  static inline void out_clr(uint8_t value) { PORT ## letter ## _OUTCLR = value; } \
  static inline void out_tgl(uint8_t value) { PORT ## letter ## _OUTTGL = value; } \
  static inline uint8_t in() { return PORT ## letter ## _IN; } \
  static inline uint8_t dir() { return PORT ## letter ## _DIR; } \
  static inline uint8_t out() { return PORT ## letter ## _OUT; } \
  static inline uint8_t event_base() { return EVSYS_CHMUX_PORT ## letter ## _PIN0_gc; } \
};

WRAP_PORT(A)
WRAP_PORT(B)
WRAP_PORT(C)
WRAP_PORT(D)
WRAP_PORT(E)
#ifdef PORTF
  WRAP_PORT(F)
#endif

template<typename Port, uint8_t bit>
struct Pin {
  static inline void set_control(uint8_t value) { }
};

#define SPECIALIZE_PIN(id) \
template<typename Port> \
struct Pin<Port, id> { \
  static inline void set_control(uint8_t value) { \
    Port::port_t().PIN ## id ## CTRL = value; \
  } \
};

SPECIALIZE_PIN(0)
SPECIALIZE_PIN(1)
SPECIALIZE_PIN(2)
SPECIALIZE_PIN(3)
SPECIALIZE_PIN(4)
SPECIALIZE_PIN(5)
SPECIALIZE_PIN(6)
SPECIALIZE_PIN(7)

template<typename Port, uint8_t bit, bool slow = true>
struct Gpio {
  static inline void set_direction(PortDirection direction) {
    if (direction == INPUT) {
      Port::dir_clr(_BV(bit));
    } else {
      Port::dir_set(_BV(bit));
      if (slow) {
        Pin<Port, bit>::set_control(PORT_SRLEN_bm);
      }
    }
  }
  
  static inline void set_mode(PortMode mode) {
    if (mode == PORT_MODE_TOTEM_POLE) {
      Pin<Port, bit>::set_control(PORT_OPC_TOTEM_gc);
    } else if (mode == PORT_MODE_BUS_KEEPER) {
      Pin<Port, bit>::set_control(PORT_OPC_BUSKEEPER_gc);
    } else if (mode == PORT_MODE_PULL_UP) {
      Pin<Port, bit>::set_control(PORT_OPC_PULLUP_gc);
    } else if (mode == PORT_MODE_PULL_DOWN) {
      Pin<Port, bit>::set_control(PORT_OPC_PULLDOWN_gc);
    }
  }
  
  static inline void set_sense(SenseMode mode) {
    if (mode == SENSE_MODE_BOTH_EDGES) {
      Pin<Port, bit>::set_control(PORT_ISC_BOTHEDGES_gc);
    } else if (mode == SENSE_MODE_RISING) {
      Pin<Port, bit>::set_control(PORT_ISC_RISING_gc);
    } else if (mode == SENSE_MODE_FALLING) {
      Pin<Port, bit>::set_control(PORT_ISC_FALLING_gc);
    } else if (mode == SENSE_MODE_LOW_LEVEL) {
      Pin<Port, bit>::set_control(PORT_ISC_LEVEL_gc);
    }
  }
  
  static inline uint8_t event() {
    return Port::event_base() + bit;
  }
  
  static inline void High() {
    Port::out_set(_BV(bit));
  }
  static inline void Low() {
    Port::out_clr(_BV(bit));
  }
  static inline void Toggle() {
    Port::out_tgl(_BV(bit));
  }
  
  static inline void set_value(uint8_t value) {
    if (value) {
      Port::out_set(_BV(bit));
    } else {
      Port::out_clr(_BV(bit));
    }
  }
  
  static inline uint8_t value() { 
    return Port::in() & _BV(bit) ? HIGH : LOW; 
  }
  
  static inline void Write(uint8_t value) { set_value(value); }
  static inline uint8_t Read() { return value(); }
};

struct DummyGpio {
  static inline void set_direction(PortDirection direction) { }
  static inline void set_mode(PortDirection direction) { }
  
  static inline void High() { }
  static inline void Low() { }
  static inline void Toggle() { }
  
  static inline void set_value(uint8_t value) { }
  static inline uint8_t value() { return 0; }
  static inline void Write(uint8_t value) { set_value(value); }
  static inline uint8_t Read() { return value(); }
};

template<typename Gpio>
struct Inverter {
  static inline void set_direction(PortDirection direction) {
    Gpio::set_direction(direction);
  }
  static inline void set_mode(PortDirection direction) {
    Gpio::set_mode(direction);
  }
  
  static inline void High() { Gpio::Low(); }
  static inline void Low() { Gpio::High(); }
  static inline void Toggle() { Gpio::Toggle(); }
  
  static inline void set_value(uint8_t value) { Gpio::set_value(!value); }
  static inline uint8_t value() { return !Gpio::value(); }
  static inline void Write(uint8_t value) { set_value(value); }
  static inline uint8_t Read() { return value(); }
};

}  // namespace avrlibx

#endif   // AVRLIBX_IO_GPIO_H_
