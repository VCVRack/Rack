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
// Clock configuration.

#ifndef AVRLIBX_SYSTEM_EVENT_SYSTEM_H_
#define AVRLIBX_SYSTEM_EVENT_SYSTEM_H_

#include <avr/io.h>

namespace avrlibx {

template<uint8_t channel>
struct EventSystemChannel {
  static inline void set_source(uint8_t source) { }
};

#define SPECIALIZE_EVENT_SYSTEM_CHANNEL(channel) \
template<> \
struct EventSystemChannel<channel> { \
  static inline void set_source(uint8_t source) { \
    EVSYS_CH ## channel ## MUX = source; \
  } \
};

SPECIALIZE_EVENT_SYSTEM_CHANNEL(0)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(1)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(2)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(3)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(4)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(5)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(6)
SPECIALIZE_EVENT_SYSTEM_CHANNEL(7)

}  // avrlibx

#endif  // AVRLIBX_SYSTEM_EVENT_SYSTEM_H_
