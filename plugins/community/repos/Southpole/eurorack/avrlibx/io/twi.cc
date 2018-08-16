// Copyright 2012 Olivier Gillet.
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
// Interrupt handlers for TWI.

#include "avrlibx/io/twi.h"

#include <avr/interrupt.h>

using namespace avrlibx;

#ifdef TWIC_TWIM_vect

void (*TwiWrapper<PortC>::handler_)() = 0;

ISR(TWIC_TWIM_vect) {
  TwiWrapper<PortC>::Handler();
}

#endif

#ifdef TWIE_TWIM_vect

void (*TwiWrapper<PortE>::handler_)() = 0;

ISR(TWIE_TWIM_vect) {
  TwiWrapper<PortE>::Handler();
}

#endif
