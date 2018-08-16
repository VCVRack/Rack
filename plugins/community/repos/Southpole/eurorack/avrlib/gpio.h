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
// An alternative gpio library based on templates.
//
// Examples of use:
//
// NumberedGpio<3>::set_mode(DIGITAL_INPUT)
// NumberedGpio<4>::set_mode(DIGITAL_OUTPUT)
// NumberedGpio<3>::value()
// NumberedGpio<4>::High()
// NumberedGpio<4>::Low()
// NumberedGpio<4>::set_value(1)
// NumberedGpio<4>::set_value(0)

#ifndef AVRLIB_GPIO_H_
#define AVRLIB_GPIO_H_

#include <avr/io.h>

#include "avrlib/avrlib.h"
#include "avrlib/timer.h"

namespace avrlib {

enum PinMode {
  DIGITAL_INPUT = 0,
  DIGITAL_OUTPUT = 1,
  PWM_OUTPUT = 2
};

// All the registers used in the following definitions are wrapped here.
IORegister(DDRB);
IORegister(DDRC);
IORegister(DDRD);

IORegister(PORTB);
IORegister(PORTC);
IORegister(PORTD);

IORegister(PINB);
IORegister(PINC);
IORegister(PIND);

// Represents a i/o port, which has input, output and mode registers.
template<typename InputRegister, typename OutputRegister,
         typename ModeRegister>
struct Port {
  typedef InputRegister Input;
  typedef OutputRegister Output;
  typedef ModeRegister Mode;
};

// Definition of I/O ports.
typedef Port<PINBRegister, PORTBRegister, DDRBRegister> PortB;
typedef Port<PINCRegister, PORTCRegister, DDRCRegister> PortC;
typedef Port<PINDRegister, PORTDRegister, DDRDRegister> PortD;

#if defined(ATMEGA164P) || defined(ATMEGA324P) || defined(ATMEGA644P) || defined(ATMEGA1284P) || defined(ATMEGA2560)

IORegister(DDRA);
IORegister(PORTA);
IORegister(PINA);
typedef Port<PINARegister, PORTARegister, DDRARegister> PortA;

#endif

#if defined (ATMEGA640) || defined (ATMEGA1280) || defined(ATMEGA2560)
IORegister(DDRE);
IORegister(DDRF);
IORegister(DDRG);
IORegister(DDRH);
IORegister(DDRJ);
IORegister(DDRK);
IORegister(DDRL);

IORegister(PORTE);
IORegister(PORTF);
IORegister(PORTG);
IORegister(PORTH);
IORegister(PORTJ);
IORegister(PORTK);
IORegister(PORTL);

IORegister(PINE);
IORegister(PINF);
IORegister(PING);
IORegister(PINH);
IORegister(PINJ);
IORegister(PINK);
IORegister(PINL);

typedef Port<PINERegister, PORTERegister, DDRERegister> PortE;
typedef Port<PINFRegister, PORTFRegister, DDRFRegister> PortF;
typedef Port<PINGRegister, PORTGRegister, DDRGRegister> PortG;
typedef Port<PINHRegister, PORTHRegister, DDRHRegister> PortH;
typedef Port<PINJRegister, PORTJRegister, DDRJRegister> PortJ;
typedef Port<PINKRegister, PORTKRegister, DDRKRegister> PortK;
typedef Port<PINLRegister, PORTLRegister, DDRLRegister> PortL;

#endif


// The actual implementation of a pin, not very convenient to use because it
// requires the actual parameters of the pin to be passed as template
// arguments.
template<typename Port, typename PwmChannel, uint8_t bit>
struct GpioImpl {
  typedef BitInRegister<typename Port::Mode, bit> ModeBit;
  typedef BitInRegister<typename Port::Output, bit> OutputBit;
  typedef BitInRegister<typename Port::Input, bit> InputBit;
  typedef PwmChannel Pwm;

  static inline void set_mode(uint8_t mode) {
    if (mode == DIGITAL_INPUT) {
      ModeBit::clear();
    } else if (mode == DIGITAL_OUTPUT || mode == PWM_OUTPUT) {
      ModeBit::set();
    }
    if (mode == PWM_OUTPUT) {
      PwmChannel::Start();
    } else {
      PwmChannel::Stop();
    }
  }

  static inline void High() {
    OutputBit::set();
  }
  static inline void Low() {
    OutputBit::clear();
  }
  static inline void Toggle() {
    OutputBit::toggle();
  }
  static inline void set_value(uint8_t value) {
    if (value == 0) {
      Low();
    } else {
      High();
    }
  }
  
  static inline void set_pwm_value(uint8_t value) {
    if (PwmChannel::has_pwm) {
      PwmChannel::Write(value);
    } else {
      set_value(value);
    }
  }

  static inline uint8_t value() {
    return InputBit::value();
  }
  static inline uint8_t is_high() {
    return InputBit::value();
  }
  static inline uint8_t is_low() {
    return InputBit::value() == 0;
  }
};


template<typename port, uint8_t bit>
struct Gpio {
  typedef GpioImpl<port, NoPwmChannel, bit> Impl;
  static void High() { Impl::High(); }
  static void Low() { Impl::Low(); }
  static void Toggle() { Impl::Toggle(); }
  static void set_mode(uint8_t mode) { Impl::set_mode(mode); }
  static void set_value(uint8_t value) { Impl::set_value(value); }
  static void set_pwm_value(uint8_t value) { Impl::set_pwm_value(value); }
  static uint8_t value() { return Impl::value(); }
  static uint8_t is_low() { return Impl::is_low(); }
  static uint8_t is_high() { return Impl::is_high(); }
};

struct DummyGpio {
  static void High() { }
  static void Low() { }
  static void set_mode(uint8_t mode) { }
  static void set_value(uint8_t value) { }
  static void set_pwm_value(uint8_t value) { }
  static uint8_t value() { return 0; }
  static uint8_t is_low() { return 0; }
  static uint8_t is_high() { return 0; }
};

template<typename Gpio>
struct Inverter {
  static void High() { Gpio::Low(); }
  static void Low() { Gpio::High(); }
  static void set_mode(uint8_t mode) { Gpio::set_mode(mode); }
  static void set_value(uint8_t value) { Gpio::set_value(!value); }
  static void set_pwm_value(uint8_t value) { Gpio::set_pwm_value(~value); }
  static uint8_t value() { return !Gpio::value(); }
  static uint8_t is_low() { return !Gpio::is_low(); }
  static uint8_t is_high() { return !Gpio::is_high(); }
};

template<typename gpio>
struct DigitalInput {
  enum {
    buffer_size = 0,
    data_size = 1,
  };
  static void Init() {
    gpio::set_mode(DIGITAL_INPUT);
  }
  static void EnablePullUpResistor() {
    gpio::High();
  }
  static void DisablePullUpResistor() {
    gpio::Low();
  }
  static uint8_t Read() {
    return gpio::value();
  }
};

// A template that will be specialized for each pin, allowing the pin number to
// be specified as a template parameter.
template<int n>
struct NumberedGpioInternal { };

// Macro to make the pin definitions (template specializations) easier to read.
#define SetupGpio(n, port, timer, bit) \
template<> struct NumberedGpioInternal<n> { \
  typedef GpioImpl<port, timer, bit> Impl; };

// Pin definitions for ATmega lineup

#if defined(ATMEGA48P) || defined(ATMEGA88P) || defined(ATMEGA168P) || defined(ATMEGA328P)

SetupGpio(0, PortD, NoPwmChannel, 0);
SetupGpio(1, PortD, NoPwmChannel, 1);
SetupGpio(2, PortD, NoPwmChannel, 2);
SetupGpio(3, PortD, PwmChannel2B, 3);
SetupGpio(4, PortD, NoPwmChannel, 4);
SetupGpio(5, PortD, PwmChannel0B, 5);
SetupGpio(6, PortD, PwmChannel0A, 6);
SetupGpio(7, PortD, NoPwmChannel, 7);
SetupGpio(8, PortB, NoPwmChannel, 0);
SetupGpio(9, PortB, PwmChannel1A, 1);
SetupGpio(10, PortB, PwmChannel1B, 2);
SetupGpio(11, PortB, PwmChannel2A, 3);
SetupGpio(12, PortB, NoPwmChannel, 4);
SetupGpio(13, PortB, NoPwmChannel, 5);
SetupGpio(14, PortC, NoPwmChannel, 0);
SetupGpio(15, PortC, NoPwmChannel, 1);
SetupGpio(16, PortC, NoPwmChannel, 2);
SetupGpio(17, PortC, NoPwmChannel, 3);
SetupGpio(18, PortC, NoPwmChannel, 4);
SetupGpio(19, PortC, NoPwmChannel, 5);

SetupGpio(255, PortB, NoPwmChannel, 0);

typedef Gpio<PortB, 5> SpiSCK;
typedef Gpio<PortB, 4> SpiMISO;
typedef Gpio<PortB, 3> SpiMOSI;
typedef Gpio<PortB, 2> SpiSS;

typedef Gpio<PortD, 4> UartSpi0XCK;
typedef Gpio<PortD, 1> UartSpi0TX;
typedef Gpio<PortD, 0> UartSpi0RX;

#define HAS_USART0

#elif defined(ATMEGA164P) || defined(ATMEGA324P) || defined(ATMEGA644P) || defined(ATMEGA1284P)

SetupGpio(0,  PortB, NoPwmChannel, 0);
SetupGpio(1,  PortB, NoPwmChannel, 1);
SetupGpio(2,  PortB, NoPwmChannel, 2);
SetupGpio(3,  PortB, PwmChannel0A, 3);
SetupGpio(4,  PortB, PwmChannel0B, 4);
SetupGpio(5,  PortB, NoPwmChannel, 5);
SetupGpio(6,  PortB, NoPwmChannel, 6);
SetupGpio(7,  PortB, NoPwmChannel, 7);

SetupGpio(8,  PortD, NoPwmChannel, 0);
SetupGpio(9,  PortD, NoPwmChannel, 1);
SetupGpio(10, PortD, NoPwmChannel, 2);
SetupGpio(11, PortD, NoPwmChannel, 3);
SetupGpio(12, PortD, PwmChannel1B, 4);
SetupGpio(13, PortD, PwmChannel1A, 5);
SetupGpio(14, PortD, PwmChannel2B, 6);
SetupGpio(15, PortD, PwmChannel2A, 7);

SetupGpio(16, PortC, NoPwmChannel, 0);
SetupGpio(17, PortC, NoPwmChannel, 1);
SetupGpio(18, PortC, NoPwmChannel, 2);
SetupGpio(19, PortC, NoPwmChannel, 3);
SetupGpio(20, PortC, NoPwmChannel, 4);
SetupGpio(21, PortC, NoPwmChannel, 5);
SetupGpio(22, PortC, NoPwmChannel, 6);
SetupGpio(23, PortC, NoPwmChannel, 7);

SetupGpio(255, PortB, NoPwmChannel, 0);

typedef Gpio<PortB, 7> SpiSCK;
typedef Gpio<PortB, 6> SpiMISO;
typedef Gpio<PortB, 5> SpiMOSI;
typedef Gpio<PortB, 4> SpiSS;

typedef Gpio<PortB, 0> UartSpi0XCK;
typedef Gpio<PortD, 1> UartSpi0TX;
typedef Gpio<PortD, 0> UartSpi0RX;

typedef Gpio<PortD, 4> UartSpi1XCK;
typedef Gpio<PortD, 3> UartSpi1TX;
typedef Gpio<PortD, 2> UartSpi1RX;

#define HAS_USART0
#define HAS_USART1

#if defined(ATMEGA1284P) || defined(ATMEGA640) || defined(ATMEGA1280)  \
	|| defined(ATMEGA2560)
#define HAS_TIMER3
#endif

#elif defined (ATMEGA640) || defined(ATMEGA1280) || defined(ATMEGA2560)

SetupGpio(0,  PortB, NoPwmChannel, 0);
SetupGpio(1,  PortB, NoPwmChannel, 1);
SetupGpio(2,  PortB, NoPwmChannel, 2);
SetupGpio(3,  PortB, NoPwmChannel, 3);
SetupGpio(4,  PortB, PwmChannel2A, 4);
SetupGpio(5,  PortB, PwmChannel1A, 5);
SetupGpio(6,  PortB, PwmChannel1B, 6);
SetupGpio(7,  PortB, PwmChannel0A, 7);

SetupGpio(8,  PortD, NoPwmChannel, 0);
SetupGpio(9,  PortD, NoPwmChannel, 1);
SetupGpio(10, PortD, NoPwmChannel, 2);
SetupGpio(11, PortD, NoPwmChannel, 3);
SetupGpio(12, PortD, NoPwmChannel, 4);
SetupGpio(13, PortD, NoPwmChannel, 5);
SetupGpio(14, PortD, NoPwmChannel, 6);
SetupGpio(15, PortD, NoPwmChannel, 7);

SetupGpio(16, PortC, NoPwmChannel, 0);
SetupGpio(17, PortC, NoPwmChannel, 1);
SetupGpio(18, PortC, NoPwmChannel, 2);
SetupGpio(19, PortC, NoPwmChannel, 3);
SetupGpio(20, PortC, NoPwmChannel, 4);
SetupGpio(21, PortC, NoPwmChannel, 5);
SetupGpio(22, PortC, NoPwmChannel, 6);
SetupGpio(23, PortC, NoPwmChannel, 7);

SetupGpio(24, PortE, NoPwmChannel, 0);
SetupGpio(25, PortE, NoPwmChannel, 1);
SetupGpio(26, PortE, NoPwmChannel, 2);
SetupGpio(27, PortE, NoPwmChannel, 3);
SetupGpio(28, PortE, NoPwmChannel, 4);
SetupGpio(29, PortE, NoPwmChannel, 5);
SetupGpio(30, PortE, NoPwmChannel, 6);
SetupGpio(31, PortE, NoPwmChannel, 7);

SetupGpio(32, PortF, NoPwmChannel, 0);
SetupGpio(33, PortF, NoPwmChannel, 1);
SetupGpio(34, PortF, NoPwmChannel, 2);
SetupGpio(35, PortF, NoPwmChannel, 3);
SetupGpio(36, PortF, NoPwmChannel, 4);
SetupGpio(37, PortF, NoPwmChannel, 5);
SetupGpio(38, PortF, NoPwmChannel, 6);
SetupGpio(39, PortF, NoPwmChannel, 7);

SetupGpio(40, PortG, NoPwmChannel, 0);
SetupGpio(41, PortG, NoPwmChannel, 1);
SetupGpio(42, PortG, NoPwmChannel, 2);
SetupGpio(43, PortG, NoPwmChannel, 3);
SetupGpio(44, PortG, NoPwmChannel, 4);
SetupGpio(45, PortG, NoPwmChannel, 5);
SetupGpio(46, PortG, NoPwmChannel, 6);
SetupGpio(47, PortG, NoPwmChannel, 7);

SetupGpio(48, PortH, NoPwmChannel, 0);
SetupGpio(49, PortH, NoPwmChannel, 1);
SetupGpio(50, PortH, NoPwmChannel, 2);
SetupGpio(51, PortH, NoPwmChannel, 3);
SetupGpio(52, PortH, NoPwmChannel, 4);
SetupGpio(53, PortH, NoPwmChannel, 5);
SetupGpio(54, PortH, NoPwmChannel, 6);
SetupGpio(55, PortH, NoPwmChannel, 7);

SetupGpio(56, PortJ, NoPwmChannel, 0);
SetupGpio(57, PortJ, NoPwmChannel, 1);
SetupGpio(58, PortJ, NoPwmChannel, 2);
SetupGpio(59, PortJ, NoPwmChannel, 3);
SetupGpio(60, PortJ, NoPwmChannel, 4);
SetupGpio(61, PortJ, NoPwmChannel, 5);
SetupGpio(62, PortJ, NoPwmChannel, 6);
SetupGpio(63, PortJ, NoPwmChannel, 7);

SetupGpio(64, PortK, NoPwmChannel, 0);
SetupGpio(65, PortK, NoPwmChannel, 1);
SetupGpio(66, PortK, NoPwmChannel, 2);
SetupGpio(67, PortK, NoPwmChannel, 3);
SetupGpio(68, PortK, NoPwmChannel, 4);
SetupGpio(69, PortK, NoPwmChannel, 5);
SetupGpio(70, PortK, NoPwmChannel, 6);
SetupGpio(71, PortK, NoPwmChannel, 7);

SetupGpio(72, PortL, NoPwmChannel, 0);
SetupGpio(73, PortL, NoPwmChannel, 1);
SetupGpio(74, PortL, NoPwmChannel, 2);
SetupGpio(75, PortL, NoPwmChannel, 3);
SetupGpio(76, PortL, NoPwmChannel, 4);
SetupGpio(77, PortL, NoPwmChannel, 5);
SetupGpio(78, PortL, NoPwmChannel, 6);
SetupGpio(79, PortL, NoPwmChannel, 7);

typedef Gpio<PortB, 0> SpiSS;
typedef Gpio<PortB, 1> SpiSCK;
typedef Gpio<PortB, 2> SpiMOSI;
typedef Gpio<PortB, 3> SpiMISO;

typedef Gpio<PortE, 2> UartSpi0XCK;
typedef Gpio<PortE, 1> UartSpi0TX;
typedef Gpio<PortE, 0> UartSpi0RX;

typedef Gpio<PortD, 5> UartSpi1XCK;
typedef Gpio<PortD, 3> UartSpi1TX;
typedef Gpio<PortD, 2> UartSpi1RX;

typedef Gpio<PortH, 2> UartSpi2XCK;
typedef Gpio<PortH, 1> UartSpi2TX;
typedef Gpio<PortH, 0> UartSpi2RX;

typedef Gpio<PortJ, 2> UartSpi3XCK;
typedef Gpio<PortJ, 1> UartSpi3TX;
typedef Gpio<PortJ, 0> UartSpi3RX;

#define HAS_USART0
#define HAS_USART1
#define HAS_USART2
#define HAS_USART3

#else

#error Unsupported MCU type

#endif

// Two specializations of the numbered pin template, one which clears the timer
// for each access to the PWM pins, as does the original Arduino wire lib,
// the other that does not (use with care!).
template<int n>
struct NumberedGpio {
  typedef typename NumberedGpioInternal<n>::Impl Impl;
  static void High() { Impl::High(); }
  static void Low() { Impl::Low(); }
  static void set_mode(uint8_t mode) { Impl::set_mode(mode); }
  static void set_value(uint8_t value) { Impl::set_value(value); }
  static void set_pwm_value(uint8_t value) { Impl::set_pwm_value(value); }
  static uint8_t value() { return Impl::value(); }
};

template<int n>
struct PwmOutput {
  enum {
    buffer_size = 0,
    data_size = 8,
  };
  static void Init() {
    NumberedGpio<n>::set_mode(PWM_OUTPUT);
  }
  static void Write(uint8_t value) {
    return NumberedGpio<n>::set_pwm_value(value);
  }
  static void Stop() {
    NumberedGpio<n>::Impl::Pwm::Stop();
  }
  static void Start() {
    NumberedGpio<n>::Impl::Pwm::Start();
  }
};

}  // namespace avrlib

#endif   // AVRLIB_GPIO_H_
