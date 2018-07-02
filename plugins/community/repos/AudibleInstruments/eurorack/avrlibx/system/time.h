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
// Millisecond clock. The RTC is not used here because we need a 32 bits
// timestamp.

#ifndef AVRLIBX_SYSTEM_TIME_H_
#define AVRLIBX_SYSTEM_TIME_H_

#include <avr/delay.h>
#include <avr/io.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/system/timer.h"

#define ConstantDelay(x) _delay_ms((x))

namespace avrlibx {

uint32_t milliseconds();
void Delay(uint32_t delay);

extern volatile LongWord milliseconds_count;

// Must be called every millisecond.
inline void Tick() {
  ++milliseconds_count.words[0];
  if (milliseconds_count.words[0] == 0) {
    ++milliseconds_count.words[1];
  }
}

#ifdef TCF0
  #define TICK_ISR ISR(TCF0_OVF_vect) { Tick(); }
  typedef Timer<PortF, 0> TickTimer;
#else
  #define TICK_ISR ISR(TCE0_OVF_vect) { Tick(); }
  typedef Timer<PortE, 0> TickTimer;
#endif  // TCF0

inline void SetupTickTimer(uint8_t priority = 1) {
  TickTimer timer;
  timer.set_prescaler(TIMER_PRESCALER_CLK);
  timer.set_period(F_CPU / 1000);
  timer.EnableOverflowInterrupt(priority);
  timer.set_mode(TIMER_MODE_NORMAL);
}

inline void DelayRTC(uint16_t delay) {
  RTC.CNT = 0;
  while (RTC.CNT < delay);
}

}  // namespace avrlibx

#endif  // AVRLIBX_SYSTEM_TIME_H_
