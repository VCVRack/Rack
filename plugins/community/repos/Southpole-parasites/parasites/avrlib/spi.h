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
// Fast SPI communication (using the hardware implementation). This will take
// ownership of the pins 11 (data output),  12 (data input) and 13 (clock), +
// a user-definable pin for slave selection. Pin 10 should be kept as an output
// pin, since the SPI master/slave mode is based upon the value of this pin.

#ifndef AVRLIB_SPI_H_
#define AVRLIB_SPI_H_

#include "avrlib/avrlib.h"
#include "avrlib/gpio.h"
#include "avrlib/serial.h"

namespace avrlib {

IORegister(SPSR);
typedef BitInRegister<SPSRRegister, SPI2X> DoubleSpeed;
typedef BitInRegister<SPSRRegister, SPIF> TransferComplete;

template<typename SlaveSelect,
         DataOrder order = MSB_FIRST,
         uint8_t speed = 4>
class SpiMaster {
 public:
  enum {
    buffer_size = 0,
    data_size = 8
  };

  static void Init() {
    SpiSCK::set_mode(DIGITAL_OUTPUT);
    SpiMOSI::set_mode(DIGITAL_OUTPUT);
    SpiMISO::set_mode(DIGITAL_INPUT);
    SpiSS::set_mode(DIGITAL_OUTPUT);  // I'm a master!
    SpiSS::High();
    SlaveSelect::set_mode(DIGITAL_OUTPUT);
    SlaveSelect::High();

    // SPI enabled, configured as master.
    uint8_t configuration = _BV(SPE) | _BV(MSTR);
    if (order == LSB_FIRST) {
      configuration |= _BV(DORD);
    }
    DoubleSpeed::clear();
    switch (speed) {
      case 2:
        DoubleSpeed::set();
      case 4:
        break;
      case 8:
        DoubleSpeed::set();
      case 16:
        configuration |= _BV(SPR0);
        break;
      case 32:
        DoubleSpeed::set();
      case 64:
        configuration |= _BV(SPR1);
        break;
      case 128:
        configuration |= _BV(SPR0);
        configuration |= _BV(SPR1);
        break;
    }
    SPCR = configuration;
  }
  
  static inline void PullUpMISO() {
    SpiMISO::High();
  }
  
  static inline void Begin() {
    SlaveSelect::Low();
  }

  static inline void End() {
    SlaveSelect::High();
  }
  
  static inline void Strobe() {
    SlaveSelect::High();
    SlaveSelect::Low();
  }

  static inline void Write(uint8_t v) {
    Begin();
    Send(v);
    End();
  }
  
  static inline uint8_t Read() {
    Begin();
    uint8_t result = Receive();
    End();
    return result;
  }
  
  static inline void Send(uint8_t v) {
    Overwrite(v);
    Wait();
  }
  
  static inline uint8_t Receive() {
    Send(0xff);
    return ImmediateRead();
  }
  
  static inline uint8_t ImmediateRead() {
    return SPDR;
  }
  
  static inline void Wait() {
    while (!TransferComplete::value());
  }
  
  static inline void OptimisticWait() {
    Wait();
  }
  
  static inline void Overwrite(uint8_t v) {
    SPDR = v;
  }

  static inline void WriteWord(uint8_t a, uint8_t b) {
    Begin();
    Send(a);
    Send(b);
    End();
  }
};

template<DataOrder order = MSB_FIRST,
         bool enable_interrupt = false>
class SpiSlave {
 public:
  enum {
    buffer_size = 128,
    data_size = 8
  };

  static void Init() {
    SpiSCK::set_mode(DIGITAL_INPUT);
    SpiMOSI::set_mode(DIGITAL_INPUT);
    SpiMISO::set_mode(DIGITAL_OUTPUT);
    SpiSS::set_mode(DIGITAL_INPUT);  // Ohhh mistress, ohhhh!

    // SPI enabled, configured as master.
    uint8_t configuration = _BV(SPE);
    if (order == LSB_FIRST) {
      configuration |= _BV(DORD);
    }
    if (enable_interrupt) {
      configuration |= _BV(SPIE);
    }
    SPCR = configuration;
  }
  
  static inline void Reply(uint8_t value) {
    SPDR = value;
  }
  
  static inline uint8_t readable() {
    return TransferComplete::value();
  }
  
  static inline uint8_t ImmediateRead() {
    return SPDR;
  }
  
  static inline uint8_t Read() {
    while (!readable());
    return ImmediateRead();
  }
};

template<typename XckPort,
         typename TxPort,
         typename RxPort,
         typename PrescalerRegister,
         typename ControlRegisterB,
         uint8_t BFlags,
         typename ControlRegisterC,
         uint8_t CFlags,
         typename TxReadyBit,
         typename DataRegister>
struct UartSpiPort {
  static inline uint8_t tx_ready() { return TxReadyBit::value(); }
  static inline uint8_t data() { return *DataRegister::ptr(); }
  static inline void set_data(uint8_t value) { *DataRegister::ptr() = value; }
  static inline void Setup(uint16_t rate) {
    *PrescalerRegister::ptr() = 0;
    XckPort::set_mode(DIGITAL_OUTPUT);
    TxPort::set_mode(DIGITAL_OUTPUT);
    RxPort::set_mode(DIGITAL_INPUT);
    *ControlRegisterC::ptr() = CFlags;
    *ControlRegisterB::ptr() = BFlags;
    *PrescalerRegister::ptr() = rate;
  }
};

#ifdef HAS_USART0

typedef UartSpiPort<
    UartSpi0XCK,
    UartSpi0TX,
    UartSpi0RX,
    UBRR0Register,
    UCSR0BRegister,
    _BV(RXEN0) | _BV(TXEN0),
    UCSR0CRegister,
    _BV(UMSEL01) | _BV(UMSEL00),
    BitInRegister<UCSR0ARegister, UDRE0>,
    UDR0Register> UartSpiPort0;

#endif  // HAS_USART0

#ifdef HAS_USART1

typedef UartSpiPort<
    UartSpi1XCK,
    UartSpi1TX,
    UartSpi1RX,
    UBRR1Register,
    UCSR1BRegister,
    _BV(RXEN1) | _BV(TXEN1),
    UCSR1CRegister,
    _BV(UMSEL11) | _BV(UMSEL10),
    BitInRegister<UCSR1ARegister, UDRE1>,
    UDR1Register> UartSpiPort1;

#endif  // HAS_USART1

template<typename Port, typename SlaveSelect, uint8_t speed = 2>
class UartSpiMaster {
 public:
  enum {
    buffer_size = 0,
    data_size = 8
  };

  static void Init() {
    SlaveSelect::set_mode(DIGITAL_OUTPUT);
    SlaveSelect::High();
    Port::Setup((speed / 2) - 1);
  }

  static inline void Begin() {
    SlaveSelect::Low();
  }

  static inline void End() {
    SlaveSelect::High();
  }
  
  static inline void Strobe() {
    SlaveSelect::High();
    SlaveSelect::Low();
  }

  static inline void Write(uint8_t v) {
    Begin();
    Send(v);
    End();
  }
  
  static inline void Send(uint8_t v) {
    Overwrite(v);
    Wait();
  }

  static inline void Wait() {
    while (!Port::tx_ready());
  }
  
  static inline void OptimisticWait() { }
  
  static inline void Overwrite(uint8_t v) {
    Port::set_data(v);
  }
  
  static inline void WriteWord(uint8_t a, uint8_t b) {
    Begin();
    Send(a);
    Send(b);
    End();
  }
};

#define SPI_RECEIVE ISR(SPI_STC_vect)

}  // namespace avrlib

#endif AVRLIB_SPI_H_
