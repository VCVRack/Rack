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
// Real time clock. Based on the code in the arduino core library
// by David A. Mellis.

#include <avr/interrupt.h>
#include <avr/io.h>

#include "avrlibx/system/time.h"

namespace avrlibx {

volatile LongWord milliseconds_count = { 0 };

void Delay(uint32_t delay) {
  uint32_t t = milliseconds() + delay;
  while (milliseconds() < t);
}

uint32_t milliseconds() {
  uint32_t m;
  uint8_t oldSREG = SREG;
  cli();
  m = milliseconds_count.value;
  SREG = oldSREG;
  return m;
}

}  // namespace avrlibx