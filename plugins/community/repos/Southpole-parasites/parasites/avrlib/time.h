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
// Real time clock.

#ifndef AVRLIB_TIME_H_
#define AVRLIB_TIME_H_

#include <avr/delay.h>

#include "avrlib/base.h"

namespace avrlib {

uint32_t milliseconds();
uint32_t Delay(uint32_t delay);

#define ConstantDelay(x) _delay_ms((x))

void InitClock();

const uint32_t microseconds_per_timer0_overflow =
    (64 * 256) / (F_CPU / 1000000L);
const uint32_t milliseconds_increment =
    microseconds_per_timer0_overflow / 1000;

const uint32_t fractional_increment = (
    microseconds_per_timer0_overflow % 1000) >> 3;

const uint8_t fractional_max = 1000 >> 3;

// The timer count is stored as an union instead of a mere uint32_t because we
// need access to the individual 16-bit parts of the value.
extern volatile LongWord timer0_milliseconds;

extern uint8_t timer0_fractional;

inline void TickSystemClock() {
  // Compile-time optimization: with a 20Mhz clock rate, milliseconds_increment
  // is always null, so we have to increment it only when there's a
  // fractional overflow!
  if (milliseconds_increment) {
    timer0_milliseconds.value += milliseconds_increment;
  }
  timer0_fractional += fractional_increment;
  if (timer0_fractional >= fractional_max) {
    timer0_fractional -= fractional_max;
    // The next lines are equivalent to: ++timer0_fractional. Why am I not
    // using ++timer0_fractional? The reason is in the way gcc compiles this.
    // 32-bits values are always loaded into contiguous registers. This code is
    // called from an ISR, so this means 4 contiguous registers are going to
    // be pushed/popped in the ISR. This costs 4 pairs of push/pops (16 cycles).
    // On the other hand, this weird implementation only requires 2 adjacent
    // registers, and they are probably already used for something else in the
    // ISR. There's no free lunch, though: this code is less efficient than
    // a++. However, when it is called every 16th or 32th entry in an ISR, the
    // time saved by avoiding the extra push/pops makes it a better choice.
    //
    // Rule: when you *occasionnally* do something complicated from within an
    // ISR, the code doing the complicated thing should really try to minimize
    // the number of registers it uses, even if it takes more cycles to do
    // the work.
    ++timer0_milliseconds.words[0];
    if (timer0_milliseconds.words[0] == 0) {
      ++timer0_milliseconds.words[1];
    }
  }
}

}  // namespace avrlib

#endif  // AVRLIB_TIME_H_
