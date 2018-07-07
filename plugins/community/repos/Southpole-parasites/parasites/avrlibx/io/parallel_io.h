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
//
// -----------------------------------------------------------------------------
//
// Templates for using a range of bits from a port for parallel IO.

#ifndef AVRLIBX_IO_PARALLEL_H_
#define AVRLIBX_IO_PARALLEL_H_

#include <avr/io.h>

#include "avrlibx/io/gpio.h"

namespace avrlibx {

template<typename Port, uint8_t first, uint8_t last>
struct ParallelPort {
  enum Masks {
    mask = ((1L << (last - first + 1)) - 1) << first,
    shift = first
  };
  
  static inline void set_direction(PortDirection direction) {
    if (direction == INPUT) {
      Port::dir_clr(mask);
    } else {
      Port::dir_set(mask);
    }
  }
  static inline void set_mode(PortMode mode) {
    PORTCFG.MPCMASK = mask;
    Gpio<Port, 0>::set_mode(mode);
  }
  
  static inline void High() {
    Port::out_set(mask);
  }
  static inline void Low() {
    Port::out_clr(mask);
  }
  static inline void Toggle() {
    Port::out_tgl(mask);
  }
  
  static inline void set_value(uint8_t value) {
    uint8_t preserve = Port::out() & ~mask;
    Port::out(preserve | (value << shift));
  }
  static inline uint8_t value() {
    return (Port::in() & mask) >> shift;
  }
  
  static inline void Write(uint8_t value) { set_value(value); }
  static inline uint8_t Read() { return value(); }
};

}  // namespace avrlibx

#endif   // AVRLIBX_IO_PARALLEL_H_
