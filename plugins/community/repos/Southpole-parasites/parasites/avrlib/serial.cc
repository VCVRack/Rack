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
// Fast serial (for the onboard UART), using compile time optimizations.

#include "avrlib/serial.h"

#include <avr/interrupt.h>

#include "avrlib/gpio.h"

using namespace avrlib;

#ifndef DISABLE_DEFAULT_UART_RX_ISR

#ifdef SERIAL_RX_0

#if defined(HAS_USART0) && defined(HAS_USART1)

ISR(USART0_RX_vect) {
  SerialInput<SerialPort0>::Received();
}

#elif defined(HAS_USART0)

ISR(USART_RX_vect) {
  SerialInput<SerialPort0>::Received();
}

#endif  // HAS_USART

#endif  // SERIAL_RX_0


#ifdef SERIAL_RX_1

#ifdef HAS_USART1

ISR(USART1_RX_vect) {
  SerialInput<SerialPort1>::Received();
}

#endif  // HAS_USART1

#endif  // SERIAL_RX_1

#endif  // DISABLE_DEFAULT_UART_RX_ISR