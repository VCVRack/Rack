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
// Driver for a HD44780 LCD display.

#ifndef AVRLIB_DEVICES_HD44780_LCD_H_
#define AVRLIB_DEVICES_HD44780_LCD_H_

#include "avrlib/base.h"
#include "avrlib/log2.h"
#include "avrlib/software_serial.h"
#include "avrlib/time.h"
#include "avrlib/resources_manager.h"

using avrlib::SimpleResourcesManager;

namespace avrlib {

enum LCD_FLAGS {
  LCD_COMMAND = 0x00,
  LCD_DATA = 0x10,

  LCD_CLEAR = 0x01,
  LCD_HOME = 0x02,
  LCD_ENTRY_MODE = 0x04,
  LCD_DISPLAY_STATUS = 0x08,
  LCD_CURSOR = 0x10,
  LCD_FUNCTION_SET = 0x20,
  LCD_SET_CGRAM_ADDRESS = 0x40,
  LCD_SET_DDRAM_ADDRESS = 0x80,

  LCD_SHIFT = 0x01,
  LCD_NO_SHIFT = 0x00,
  LCD_CURSOR_INCREMENT = 0x02,
  LCD_CURSOR_NO_INCREMENT = 0x00,
  LCD_DISPLAY_ON = 0x04,
  LCD_DISPLAY_OFF = 0x00,
  LCD_CURSOR_ON = 0x02,
  LCD_CURSOR_OFF = 0x00,
  LCD_BLINKING_ON = 0x01,
  LCD_BLINKING_OFF = 0x00,

  LCD_8_BITS = 0x10,
  LCD_4_BITS = 0x00,

  LCD_2_LINE = 0x08,
  LCD_1_LINE = 0x00,

  LCD_LARGE_FONT = 0x04,
  LCD_SMALL_FONT = 0x00,
};

template<typename RsPin,
         typename EnablePin,
         typename ParallelPort,
         uint8_t width = 16,
         uint8_t height = 2>
class Hd44780Lcd {
 public:
  enum {
    buffer_size = 64,
    data_size = 8,
  };
  enum {
    lcd_width = width,
    lcd_height = height,
  };
  typedef Hd44780Lcd<RsPin, EnablePin, ParallelPort, width, height> Me;
  typedef typename DataTypeForSize<data_size>::Type Value;
  typedef RingBuffer<Me> OutputBuffer;

  Hd44780Lcd() { }

  static inline void Init() {
    RsPin::set_mode(DIGITAL_OUTPUT);
    EnablePin::set_mode(DIGITAL_OUTPUT);
    ParallelPort::set_mode(DIGITAL_OUTPUT);

    RsPin::Low();
    EnablePin::Low();

    ConstantDelay(100);  // Wait for warm up

    // Set to 4 bit operation.
    for (uint8_t i = 0; i < 3; ++i) {
      SlowWrite((LCD_FUNCTION_SET | LCD_8_BITS) >> 4);
      ConstantDelay(2);
    }
    SlowWrite((LCD_FUNCTION_SET | LCD_4_BITS) >> 4);

    // Set number of lines and bus size.
    if (height == 2) {
      SlowCommand(LCD_FUNCTION_SET | LCD_4_BITS | LCD_2_LINE | LCD_SMALL_FONT);
    } else {
      SlowCommand(LCD_FUNCTION_SET | LCD_4_BITS | LCD_SMALL_FONT);
    }
    SlowCommand(LCD_DISPLAY_STATUS | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINKING_OFF);
    SlowCommand(LCD_ENTRY_MODE | LCD_CURSOR_INCREMENT | LCD_NO_SHIFT);
    SlowCommand(LCD_CLEAR);
    SlowCommand(LCD_HOME);
    transmitting_ = 0;
  }

  static inline void Tick() {
    ++status_counter_;
    if (transmitting_) {
      EndWrite();
      transmitting_ = 0;
    } else {
      if (OutputBuffer::readable()) {
        transmitting_ = 1;
        StartWrite(OutputBuffer::ImmediateRead());
      }
    }
  }
  
  static uint8_t WriteData(uint8_t c) {
    if (OutputBuffer::writable() < 2) {
      return 0;
    }
    OutputBuffer::Overwrite2(LCD_DATA | (c >> 4), LCD_DATA | (c & 0xf));
  }

  static uint8_t WriteCommand(uint8_t c) {
    if (OutputBuffer::writable() < 2) {
      return 0;
    }
    OutputBuffer::Overwrite2(LCD_COMMAND | (c >> 4), LCD_COMMAND | (c & 0x0f));
  }
  
  static inline uint8_t Write(uint8_t character) {
    WriteData(character);
  }
  
  static inline uint8_t Write(const char* s) {
    while (*s) {
      WriteData(*s);
      ++s;
    }
  }

  static inline void MoveCursor(uint8_t row, uint8_t col) {
    WriteCommand(LCD_SET_DDRAM_ADDRESS | col | (row << 6));
  }
 
  static inline void SetCustomCharMap(
      const uint8_t* data,
      uint8_t num_characters,
      uint8_t first_character) {
    SlowCommand(LCD_SET_CGRAM_ADDRESS | (first_character << 3));
    for (uint8_t i = 0; i < num_characters * 8; ++i) {
      SlowData(*data++);
    }
  }

  static inline void SetCustomCharMapRes(
      const uint8_t* data,
      uint8_t num_characters,
      uint8_t first_character) {
    SlowCommand(LCD_SET_CGRAM_ADDRESS | (first_character << 3));
    for (uint8_t i = 0; i < num_characters * 8; ++i) {
      SlowData(SimpleResourcesManager::Lookup<uint8_t, uint8_t>(data, i));
    }
  }
  
  static inline void Flush() {
    while (OutputBuffer::readable() || busy()) {
      Tick();
      ConstantDelay(1);
    }
  }
  
  static inline uint8_t writable() { return OutputBuffer::writable(); }
  static inline uint8_t readable() { return OutputBuffer::readable(); }
  static inline uint8_t busy() { return transmitting_; }
  static inline uint8_t status_counter() { return status_counter_; }
  
  static inline void ResetStatusCounter() { status_counter_ = 0; }

 private:
  static inline void StartWrite(uint8_t nibble) {
    if (nibble & LCD_DATA) {
      RsPin::High();
    }
    ParallelPort::Write(nibble & 0x0f);
    EnablePin::High();
  }

  static inline void EndWrite() {
    EnablePin::Low();
    RsPin::Low();
  }

  static void SlowWrite(uint8_t nibble) {
    StartWrite(nibble);
    ConstantDelay(1);
    EndWrite();
    ConstantDelay(3);
  }
  
  static void SlowCommand(uint8_t value) {
    SlowWrite(LCD_COMMAND | (value >> 4));
    SlowWrite(LCD_COMMAND | (value & 0x0f));
  }
  
  static void SlowData(uint8_t value) {
    SlowWrite(LCD_DATA | (value >> 4));
    SlowWrite(LCD_DATA | (value & 0x0f));
  }

  static volatile uint8_t transmitting_;
  static volatile uint8_t status_counter_;

  DISALLOW_COPY_AND_ASSIGN(Hd44780Lcd);
};

/* static */
template<typename RsPin, typename EnablePin, typename ParallelPort,
         uint8_t width, uint8_t height>
volatile uint8_t Hd44780Lcd<RsPin, EnablePin, ParallelPort, width,
                            height>::transmitting_;

/* static */
template<typename RsPin, typename EnablePin, typename ParallelPort,
         uint8_t width, uint8_t height>
volatile uint8_t Hd44780Lcd<RsPin, EnablePin, ParallelPort, width,
                            height>::status_counter_;

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_HD44780_LCD_H_
