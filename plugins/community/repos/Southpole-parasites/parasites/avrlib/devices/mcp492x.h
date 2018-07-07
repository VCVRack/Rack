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
// Driver for a MCP492x DAC (SPI single/dual 12-bits DAC).

#ifndef AVRLIB_DEVICES_MCP492X_H_
#define AVRLIB_DEVICES_MCP492X_H_

#include "avrlib/spi.h"
#include "avrlib/op.h"

using namespace avrlib;

namespace avrlib {

static const uint8_t kDacSpeed = 2;

enum DacVoltageReference {
  BUFFERED_REFERENCE,
  UNBUFFERED_REFERENCE
};

template<typename Interface,
         DacVoltageReference voltage_reference = UNBUFFERED_REFERENCE,
         uint8_t gain = 1>
class Dac {
 public:
  enum {
    buffer_size = 0,
    data_size = 8,
  };
  Dac() { }

  static void Init() {
    Interface::Init();
  }

  static inline void Write(uint8_t value) {
    Write(value, 0);
  }

  static inline void Write(uint8_t value, uint8_t channel) {
    value = U8Swap4(value);
    uint8_t command;
    command = (value & 0x0f) | 0x10;
    if (channel) {
      command |= 0x80;
    }
    if (voltage_reference == BUFFERED) {
      command |= 0x40;
    }
    if (gain == 1) {
      command |= 0x20;
    }
    Interface::WriteWord(command, value & 0xf0);
  }
};

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_MCP492X_H_
