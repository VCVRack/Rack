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

#ifndef AVRLIBX_SYSTEM_BOOT_H_
#define AVRLIBX_SYSTEM_BOOT_H_

#include <avr/interrupt.h>

#include "avrlibx/system/clock.h"

namespace avrlibx {

inline void SysInit() {
  sei();
  PMIC.CTRL |= 0x7;
  SetupClock<CLOCK_EXTERNAL_8M, 4>();
}

}  // avrlibx

#endif  // AVRLIBX_SYSTEM_CLOCK_H_
