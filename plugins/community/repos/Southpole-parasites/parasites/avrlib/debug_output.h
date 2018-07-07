// Copyright 2011 Peter Kvitek
//
// Author: Peter Kvitek (pete@kvitek.com)
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
// Serial debug output. Supports USART and software serial, both vanilla and buffered.
//
// Overrides stdout so printf("Hello world"), putc() etc. all do the right thing.
//
// * USART output declaration:
//
//   DebugOutput< Serial<SerialPort0, 57600, DISABLED, POLLED> > dbg;
//
// * Software serial declaration: (see software_serial.h)
//
//   DebugOutput< SoftwareSerialOutput< Gpio<PortD, 1>, 57600> > dbg;
//
// * Buffered software serial declaration:
//
//   typedef BufferedSoftwareSerialOutput< Gpio<PortD, 1>, 31250, 2400, 32> DbgOutput;
//
//   DebugOutput<DbgOutput> dbg;
//
//   void InitTimer2() {
//     Timer<2>::set_prescaler(3);
//     Timer<2>::set_mode(TIMER_CTC);
//     TIMSK2|= (1 << OCIE2A);
//     OCR2A = 19;  // for prescaler 32 @ 20MHz
//     Timer<2>::Start();
//   }
//
//   ISR(TIMER2_COMPA_vect) 
//   {
//     DbgOutput::Tick(); // called at 31250 kHz
//   }

#ifndef AVRLIB_DEBUG_OUTPUT_H_
#define AVRLIB_DEBUG_OUTPUT_H_

#include <stdio.h>
#include <avr/io.h>
#include "avrlib/serial.h"

namespace avrlib {

template<typename SerialT, bool ensureCRLF = false>
class DebugOutput {
 public:

  DebugOutput() { }

  static void Init() {
    SerialT::Init();
    stdout = &dbg_stdout_;
  }

  static inline void Write(char c) {
    if (ensureCRLF && c == '\n') {
      SerialT::Write('\r');
    }
    SerialT::Write(c);
  }

  static inline void Write(char* s) {
    while (*s) {
      Write(*s++);
    }
  }

 private:
  static FILE dbg_stdout_;

  static int dbg_putchar(char c, FILE* stream) {
    Write(c);
    return 0;
  }

  DISALLOW_COPY_AND_ASSIGN(DebugOutput);
};

// Static variables created for each instance

template<typename SerialT, bool ensureCRLF>
  FILE DebugOutput<SerialT, ensureCRLF>::dbg_stdout_ = 
    { 0, 0, _FDEV_SETUP_WRITE, 0, 0, DebugOutput<SerialT, ensureCRLF>::dbg_putchar, NULL, 0 };

}  // namespace avrlib

#endif   // AVRLIB_DEBUG_OUTPUT_H_
