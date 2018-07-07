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
// Real time clock. Based on the code in the arduino core library
// by David A. Mellis.

#include "avrlib/time.h"

#include "avrlib/timer.h"

namespace avrlib {

volatile LongWord timer0_milliseconds = { 0 };
uint8_t timer0_fractional = 0;

uint32_t Delay(uint32_t delay) {
  uint32_t t = milliseconds() + delay;
  while (milliseconds() < t);
}

uint32_t milliseconds() {
  uint32_t m;
  uint8_t oldSREG = SREG;
  cli();
  m = timer0_milliseconds.value;
  SREG = oldSREG;
  return m;
}

void InitClock() {
  Timer<0>::set_prescaler(3);
  Timer<0>::set_mode(TIMER_FAST_PWM);
  Timer<0>::Start();
}

}  // namespace avrlib
