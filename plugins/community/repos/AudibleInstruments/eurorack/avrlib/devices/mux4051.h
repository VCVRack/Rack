// Copyright 2011 Peter Kvitek
//
// Author: Peter Kvitek (pete@kvitek.com)
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
// CD4051 mux/demux abstraction. 4051 must be connected to low or high
// nibble of a port.
//
// Based on parallel_io.cc code by Olivier Gillet (ol.gillet@gmail.com)

#ifndef AVRLIB_DEVICES_MUX4051_H_
#define AVRLIB_DEVICES_MUX4051_H_

#include <avr/io.h>

#include "avrlib/gpio.h"

namespace avrlib {

enum Mux4051PortMode {
  MUX4051_NIBBLE_HIGH,
  MUX4051_NIBBLE_LOW,
};

template<Mux4051PortMode mode>
struct ShiftMasks {
  enum Masks {
    maskD = 0,
    maskE = 0,
    shift = 0
  };
};

template<>
struct ShiftMasks<MUX4051_NIBBLE_HIGH> {
  enum Masks {
    maskD = 0x70,
    maskE = 0x80,
    shift = 4,
  };
};

template<>
struct ShiftMasks<MUX4051_NIBBLE_LOW> {
  enum Masks {
    maskD = 0x07,
    maskE = 0x08,
    shift = 0,
  };
};

template<typename Port, Mux4051PortMode mux_mode = MUX4051_NIBBLE_LOW>
struct Mux4051Port {
  typedef ShiftMasks<mux_mode> Masks;

  static inline void Init() {
    *Port::Mode::ptr()|= Masks::maskD | Masks::maskE;
  }

  static inline void Disable() {
    *Port::Output::ptr()|= (0x08 << Masks::shift);
  }

  static inline void Enable() {
    *Port::Output::ptr()&=~(0x08 << Masks::shift);
  }

  static inline void Write(uint8_t value) {
    uint8_t tmp = *Port::Output::ptr() & ~Masks::maskD;
    *Port::Output::ptr() = tmp | (value << Masks::shift);
  }

};

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_MUX4051_H_
