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
// Driver for Wii Nunchuk.

#ifndef AVRLIB_DEVICES_WII_NUNCHUK_H_
#define AVRLIB_DEVICES_WII_NUNCHUK_H_

#include "avrlib/i2c/i2c.h"

namespace avrlib {

const uint8_t kNunchukAddress = 0x52;
const uint8_t kNunchukPacketSize = 6;
const uint16_t kNunchukTimeout = 65535;

class WiiNunchuk {
 public:
  WiiNunchuk() { }

  static void Done() {
    bus_.Done();
  }

  // Returns 0 in case of error.
  static uint8_t Init() {
    bus_.Init();
    bus_.FlushOutputBuffer();
    bus_.Write(0xf0);
    bus_.Write(0x55);
    bus_.Send(kNunchukAddress);
    if (bus_.Wait(kNunchukTimeout) != I2C_ERROR_NONE) {
      return 0;
    }
    bus_.Write(0xfb);
    bus_.Write(0x00);
    bus_.Send(kNunchukAddress);
    if (bus_.Wait(kNunchukTimeout) != I2C_ERROR_NONE) {
      return 0;
    }
    return 1;
  }
  
  // Returns 0 in case of error.
  static uint8_t Poll() {
    bus_.FlushInputBuffer();
    bus_.FlushOutputBuffer();
    bus_.Write(0x00);
    bus_.Send(kNunchukAddress);
    if (bus_.Wait(kNunchukTimeout) == I2C_ERROR_NONE) {
      if (bus_.Request(kNunchukAddress, kNunchukPacketSize) == \
          kNunchukPacketSize) {
        if (bus_.Wait(kNunchukTimeout) == I2C_ERROR_NONE) {
          for (uint8_t i = 0; i < kNunchukPacketSize; ++i) {
            data_[i] = bus_.ImmediateRead();
          }
          return 1;
        }
      }
    }
    return 0;
  }
  
  static inline uint8_t joystick_x() { return data_[0]; }
  static inline uint8_t joystick_y() { return data_[1]; }
  
  static inline uint8_t acc_x() { return data_[2]; }
  static inline uint8_t acc_y() { return data_[3]; }
  static inline uint8_t acc_z() { return data_[4]; }
  
  static inline uint16_t acc_x_16() { 
    return (static_cast<uint16_t>(data_[2]) << 2) | ((data_[5] & 0x0c) >> 2);
  }
  static inline uint16_t acc_y_16() { 
    return (static_cast<uint16_t>(data_[3]) << 2) | ((data_[5] & 0x30) >> 4);
  }
  static inline uint16_t acc_z_16() { 
    return (static_cast<uint16_t>(data_[4]) << 2) | ((data_[5] & 0xc0) >> 6);
  }
  
  static inline uint8_t z_pressed() { return !(data_[5] & 0x01); }
  static inline uint8_t c_pressed() { return !(data_[5] & 0x02); }
  static inline uint8_t alive() {
    for (uint8_t i = 0; i < 6; ++i) {
      if (data_[i] != 0xff) {
        return 1;
      }
    }
    return 0;
  }

 private:
  static uint8_t data_[6];
  static I2cMaster<8, 4, 400000> bus_;

  DISALLOW_COPY_AND_ASSIGN(WiiNunchuk);
};

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_WII_NUNCHUK_H_
