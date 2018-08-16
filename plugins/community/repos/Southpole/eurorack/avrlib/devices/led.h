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
// Single pin LED device, sink or source current. Also a flashing LED device: 
// being set to on LED stays lit for specified about of tick cycles.

#ifndef AVRLIB_DEVICES_LED_H_
#define AVRLIB_DEVICES_LED_H_

#include <avr/io.h>

#include "avrlib/gpio.h"

namespace avrlib {

enum LedMode {
  LED_SINK_CURRENT,
  LED_SOURCE_CURRENT,
};

template<typename LedPort,
         LedMode led_mode = LED_SINK_CURRENT>
class Led {
 public:

  static void Init() {
    LedPort::set_mode(DIGITAL_OUTPUT);
    Off();
  }

  static inline void On(bool on) {
    if (on) {
      On();
    } else
      Off();
  }

  static inline void On() {
    if (led_mode == LED_SINK_CURRENT) {
      LedPort::Low();
    } else
      LedPort::High();
  }

  static inline void Off() {
    if (led_mode == LED_SINK_CURRENT) {
      LedPort::High();
    } else
      LedPort::Low();
  }
};


template<typename LedT,
         uint8_t on_count = 10>
class FlashLed {
 public:

  FlashLed() { }

  static void Init() {
    LedT::Init();
    on_count_ = 0;
  }

  static inline void On() {
    LedT::On();
    on_count_ = on_count;
  }

  static inline void Off() {
    LedT::Off();
    on_count_ = 0;
  }

  static inline void Tick() {
    if (on_count_) {
      if (--on_count_ == 0) {
        LedT::Off();
      }
    }
  }

 private:
  static volatile uint8_t on_count_;

  DISALLOW_COPY_AND_ASSIGN(FlashLed);
};

// Static variables created for each instance

template<typename LedT,
         uint8_t on_count>
  volatile uint8_t FlashLed<LedT, on_count>::on_count_ = 0;

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_LED_H_
