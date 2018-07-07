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
// Interface to the onboard ADC converter, and analog multiplexer.

#ifndef AVRLIB_ADC_H_
#define AVRLIB_ADC_H_

#include <avr/io.h>

#include "avrlib/avrlib.h"

namespace avrlib {

enum AdcReference {
  ADC_EXTERNAL = 0,
  ADC_DEFAULT = 1,
  ADC_INTERNAL = 3
};

enum AdcAlignment {
  ADC_RIGHT_ALIGNED = 0,
  ADC_LEFT_ALIGNED = 1
};

IORegister(ADCSRA);

typedef BitInRegister<ADCSRARegister, ADSC> AdcConvert;
typedef BitInRegister<ADCSRARegister, ADEN> AdcEnabled;

class Adc {
 public:
  Adc() { }
  
  static inline void Init() {
    set_prescaler(7);  // 128 -> 156kHz ADC clock.
    Enable();
  }
  static inline void set_prescaler(uint8_t factor) {
    ADCSRA = (ADCSRA & ~0x07) | (factor & 0x07);
  }
  static inline void set_reference(AdcReference reference) {
    admux_value_ = (admux_value_ & 0x3f) | (reference << 6);
  }
  static inline void set_alignment(AdcAlignment alignment) {
    admux_value_ &= 0xc0;
    if (alignment == ADC_LEFT_ALIGNED) {
      admux_value_ |= 0x20;
    }
  }
  static inline void Enable() {
    AdcEnabled::set();
  }
  static inline void Disable() {
    AdcEnabled::clear();
  }
  static inline int16_t Read(uint8_t pin) {
    StartConversion(pin);
    Wait();
    return ReadOut();
  }
  
  static inline void StartConversion(uint8_t pin) {
    ADMUX = admux_value_ | (pin & 0x07);
    AdcConvert::set();
  }
  static inline void Wait() {
    while (AdcConvert::value());
  }
  static bool ready() {
    return !AdcConvert::value();
  }
  static inline int16_t ReadOut() {
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    return (high << 8) | low;
  }
  // Only works when the ADC is left aligned
  static inline uint8_t ReadOut8() {
    return ADCH;
  }

 private:
  static uint8_t admux_value_;
 
  DISALLOW_COPY_AND_ASSIGN(Adc);
};

// Class that cycles through all the analog pins and read their value. Compared
// to Adc::Read(), this class is wait-free as it doesn't block on the
// ADSC bit value.
class AdcInputScanner {
 public:
  enum {
    buffer_size = 0,
    data_size = 16,
  };
   
  AdcInputScanner() { }
  
  static void Init() {
    Adc::Init();
    current_pin_ = 0;
    Adc::StartConversion(0);
  }
  
  static inline void set_num_inputs(uint8_t num_inputs) {
    num_inputs_ = num_inputs;
  }
  
  static inline int16_t Read(uint8_t pin) {
    return state_[pin];
  }
  
  static inline uint8_t Read8(uint8_t pin) {
    return static_cast<uint16_t>(state_[pin]) >> 8;
  }

  static uint8_t current_pin() {
    return current_pin_;
  }
  
  static int16_t Sync(uint8_t pin) {
    Adc::Wait();
    int16_t value = Adc::Read(pin);
    Adc::StartConversion(current_pin_);
    return value;
  }
  
  static void Scan() {
    Adc::Wait();
    state_[current_pin_] = Adc::ReadOut();
    ++current_pin_;
    if (current_pin_ >= num_inputs_) {
      current_pin_ = 0;
    }
    Adc::StartConversion(current_pin_);
  }
  
 private:
  static uint8_t current_pin_;
  static uint8_t num_inputs_;
  static int16_t state_[8];
  
  DISALLOW_COPY_AND_ASSIGN(AdcInputScanner);
};

template<int pin>
struct AnalogInput {
  enum {
    buffer_size = 0,
    data_size = 16,
  };
  static void Init() { }
  static int16_t Read() {
    return Adc::Read(pin);
  }
};

template<int id>
class MuxedAnalogInput {
 public:
  enum {
    buffer_size = 0,
    data_size = 16,
  };
  static void Init() { }
  static int16_t Read() {
    return Adc::Read(current_pin_);
  }
  static void set_pin(uint8_t current_pin) {
    current_pin_ = current_pin;
  }

 private:
  static uint8_t current_pin_;
};

/* static */
template<int id>
uint8_t MuxedAnalogInput<id>::current_pin_ = 0;

}  // namespace avrlib

#endif  // AVRLIB_ADC_H_
