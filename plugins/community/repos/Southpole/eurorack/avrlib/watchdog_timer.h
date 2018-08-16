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

#ifndef AVRLIB_WATCHDOG_TIMER_H_
#define AVRLIB_WATCHDOG_TIMER_H_

#include <avr/wdt.h>

#include "avrlib/base.h"

namespace avrlib {

// Note: this requires the bootloader to clear the Watchdog timer flags just
// after start-up.
inline void SystemReset(uint8_t interval) {
  wdt_enable(interval);
}

inline void ResetWatchdog() {
  uint8_t watchdog_status = MCUSR;
  MCUSR = 0;
  WDTCSR |= _BV(WDCE) | _BV(WDE);
  WDTCSR = 0;
}

}  // namespace avrlib

#endif  // AVRLIB_WATCHDOG_TIMER_H_
