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
// Definitions of timer and related PWM registers.

#ifndef AVRLIB_TIMER_H_
#define AVRLIB_TIMER_H_

// interrupt.h is not strictly needed here, but .cc files including the timer
// classes are likely to also define interrupt handlers (and we have macros for
// that).
#include <avr/interrupt.h>
#include <avr/io.h>

#include "avrlib/avrlib.h"

namespace avrlib {

SpecialFunctionRegister(TCCR0A);
SpecialFunctionRegister(TCCR0B);
SpecialFunctionRegister(TCCR1A);
SpecialFunctionRegister(TCCR1B);
SpecialFunctionRegister(TCCR2A);
SpecialFunctionRegister(TCCR2B);
SpecialFunctionRegister(TIMSK0);
SpecialFunctionRegister(TIMSK1);
SpecialFunctionRegister(TIMSK2);
SpecialFunctionRegister(TCNT0);
SpecialFunctionRegister16(TCNT1);
SpecialFunctionRegister(TCNT2);
SpecialFunctionRegister(OCR0A);
SpecialFunctionRegister(OCR0B);
SpecialFunctionRegister(OCR1A);
SpecialFunctionRegister(OCR1B);
SpecialFunctionRegister(OCR2A);
SpecialFunctionRegister(OCR2B);

#ifdef HAS_TIMER3
SpecialFunctionRegister(TCCR3A);
SpecialFunctionRegister(TCCR3B);
SpecialFunctionRegister(TIMSK3);
SpecialFunctionRegister(TCNT3);
SpecialFunctionRegister(OCR3A);
SpecialFunctionRegister(OCR3B);
#endif  // HAS_TIMER3

enum TimerMode {
  TIMER_NORMAL = 0,
  TIMER_PWM_PHASE_CORRECT = 1,
  TIMER_CTC = 2,
  TIMER_FAST_PWM = 3,
};

template<typename ControlRegisterA,
         typename ControlRegisterB,
         typename InterruptRegister,
         typename ValueRegister>
struct TimerImpl {
  typedef ControlRegisterA A;
  typedef ControlRegisterB B;

  static inline uint8_t value() {
    return *ValueRegister::ptr();
  }

  static inline void Start() {
    *InterruptRegister::ptr() |= _BV(0);
  }
  static inline void Stop() {
    *InterruptRegister::ptr() &= ~(_BV(0));
  }
  static inline void StartInputCapture() {
    *InterruptRegister::ptr() |= _BV(5);
  }
  static inline void StopInputCapture() {
    *InterruptRegister::ptr() &= ~(_BV(5));
  }
  static inline void StartCompare() {
    *InterruptRegister::ptr() |= _BV(1);
  }
  static inline void StopCompare() {
    *InterruptRegister::ptr() &= ~(_BV(1));
  }
  
  static inline void set_mode(TimerMode mode) {
    // Sets the mode registers.
    *ControlRegisterA::ptr() = (*ControlRegisterA::ptr() & 0xfc) | mode;
  }
  
  static inline void set_mode(
      uint8_t wg_mode_1,
      uint8_t wg_mode_2,
      uint8_t prescaler) {
    // Sets the mode registers.
    *ControlRegisterA::ptr() = wg_mode_1;
    *ControlRegisterB::ptr() = wg_mode_2 | prescaler;
  }
  
  static inline void set_value(uint16_t value) {
    *ValueRegister::ptr() = value;
  }

  // These are the values for MCUs clocked at 20 MHz
  // 
  // Timer speed
  // value | fast        | accurate
  // --------------------------------------
  // 1     | 78.125 kHz  | 39.215 kHz
  // 2     | 9.765 kHz   | 4.901 kHz
  // 3     | 1220.7 Hz   | 612.7 Hz
  // 4     | 305.2 Hz    | 153.2 Hz
  // 5     | 76.3 Hz     | 38.3 Hz
  static inline void set_prescaler(uint8_t prescaler) {
    *ControlRegisterB::ptr() = (*ControlRegisterB::ptr() & 0xf8) | prescaler;
  }
};

template<int n>
struct NumberedTimer { };

template<> struct NumberedTimer<0> {
  typedef TimerImpl<
      TCCR0ARegister,
      TCCR0BRegister,
      TIMSK0Register,
      TCNT0Register> Impl;
};

template<> struct NumberedTimer<1> {
  typedef TimerImpl<
      TCCR1ARegister,
      TCCR1BRegister,
      TIMSK1Register,
      TCNT1Register> Impl;
};

template<> struct NumberedTimer<2> {
  typedef TimerImpl<
      TCCR2ARegister,
      TCCR2BRegister,
      TIMSK2Register,
      TCNT2Register> Impl;
};

#ifdef HAS_TIMER3
template<> struct NumberedTimer<3> {
  typedef TimerImpl<
      TCCR3ARegister,
      TCCR3BRegister,
      TIMSK3Register,
      TCNT3Register> Impl;
};
#endif  // HAS_TIMER3

template<int n>
struct Timer {
  typedef typename NumberedTimer<n>::Impl Impl;
  static inline uint8_t value() { return Impl::value(); }
  static inline void Start() { Impl::Start(); }
  static inline void Stop() { Impl::Stop(); }
  static inline void StartInputCapture() { Impl::StartInputCapture(); }
  static inline void StopInputCapture() { Impl::StopInputCapture(); }
  static inline void StartCompare() { Impl::StartCompare(); }
  static inline void StopCompare() { Impl::StopCompare(); }
  static inline void set_mode(TimerMode mode) { Impl::set_mode(mode); }
  static inline void set_mode(uint8_t a, uint8_t b, uint8_t c) {
    Impl::set_mode(a, b, c);
  }
  static inline void set_prescaler(uint8_t prescaler) {
    Impl::set_prescaler(prescaler);
  }
};

template<typename Timer, uint8_t enabled_flag, typename PwmRegister>
struct PwmChannel {
  typedef BitInRegister<typename Timer::Impl::A, enabled_flag> EnabledBit;
  enum {
    has_pwm = 1
  };
  static inline void Start() {
    EnabledBit::set();
  }
  static inline void Stop() {
    EnabledBit::clear();
  }
  static inline void Write(uint8_t value) {
    *PwmRegister::ptr() = value;
  }
  static inline void set_frequency(uint16_t f) {
    OCR1A = f;
    OCR1B = f >> 1;
  }
  static inline void set_frequency_pulse(uint16_t f) {
    OCR1A = f;
    OCR1B = f - (f >> 2);
  }
};

struct NoPwmChannel {
  enum {
    has_pwm = 0
  };
  static inline void Start() { }
  static inline void Stop() { }
  static inline void Write(uint8_t value) { }
};

typedef PwmChannel<Timer<0>, COM0A1, OCR0ARegister> PwmChannel0A;
typedef PwmChannel<Timer<0>, COM0B1, OCR0BRegister> PwmChannel0B;
typedef PwmChannel<Timer<1>, COM1A1, OCR1ARegister> PwmChannel1A;
typedef PwmChannel<Timer<1>, COM1B1, OCR1BRegister> PwmChannel1B;
typedef PwmChannel<Timer<2>, COM2A1, OCR2ARegister> PwmChannel2A;
typedef PwmChannel<Timer<2>, COM2B1, OCR2BRegister> PwmChannel2B;

// Readable aliases for timer interrupts.
#define TIMER_0_TICK ISR(TIMER0_OVF_vect)
#define TIMER_1_TICK ISR(TIMER1_OVF_vect)
#define TIMER_2_TICK ISR(TIMER2_OVF_vect)

#ifdef HAS_TIMER3
#define TIMER_3_TICK ISR(TIMER3_OVF_vect)
#endif  // HAS_TIMER3

}  // namespace avrlib

#endif   // AVRLIB_TIMER_H_
