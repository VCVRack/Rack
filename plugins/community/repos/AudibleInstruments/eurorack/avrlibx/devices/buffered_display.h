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
// In-memory text page with on-demand copy to LCD display.
//
// Instead of queuing text drawing instruction in the LCD buffer (BAD), this
// uses 2 text buffers, one containing the current status of the LCD, and one
// containing the requested text page. The 2 buffer are compared in a background
// process and differences are sent to the LCD display. This also manages a
// software blinking cursor.

#ifndef AVRLIBX_DEVICES_BUFFERED_DISPLAY_H_
#define AVRLIBX_DEVICES_BUFFERED_DISPLAY_H_

#include "avrlibx/avrlibx.h"
#include "avrlibx/system/time.h"
#include "avrlibx/utils/log2.h"
#include "avrlibx/utils/op.h"

namespace avrlibx {

static const uint8_t kLcdNoCursor = 0xff;
static const uint8_t kLcdCursor = 0xff;
static const uint8_t kLcdEditCursor = '_';

template<typename Lcd>
class BufferedDisplay {
 public:
  enum {
    width = Lcd::lcd_width,
    height = Lcd::lcd_height,
    lcd_buffer_size = width * height,
  };

  BufferedDisplay() { }

  static void Init() {
    Clear();
    memset(remote_, '?', lcd_buffer_size);
    scan_position_ = 0;
    scan_row_ = 0;
    scan_column_ = 0;
    scan_position_last_write_ = 255;
    cursor_position_ = 255;
    blink_ = 0;
  }
  
  static char* line_buffer(uint8_t line) {
    return static_cast<char*>(
        static_cast<void*>(local_ + U8U8Mul(line, width)));
  }

  static void Print(uint8_t line, const char* text) {
    uint8_t row = width;
    char* destination = line_buffer(line);
    while (*text && row) {
      *destination++ = *text;
      ++text;
      --row;
    }
  }
  
  static void Clear() {
    memset(local_, ' ', lcd_buffer_size);
  }

  // Use kLcdNoCursor (255) or any other value outside of the screen to hide.
  static inline void set_cursor_position(uint8_t cursor) {
    cursor_position_ = cursor;
  }

  static inline void set_cursor_character(uint8_t character) {
    cursor_character_ = character;
  }
  
  static inline uint8_t cursor_position() {
    return cursor_position_;
  }

  static inline void set_status(uint8_t status) {
    status_ = status + 1;
  }
  
  static inline void ForceStatus(uint8_t status) {
    if (Lcd::writable() < 4) {
      return;
    }
    status_ = status + 1;
    scan_position_ = 0;
    scan_row_ = 0;
    scan_column_ = 0;
    Lcd::MoveCursor(scan_row_, scan_column_);
    Lcd::WriteData(status_ - 1);
    remote_[scan_position_] = status_ - 1;
  }

  static void Tick() {
    // The following code is likely to write 4 bytes at most. If there are less
    // than 4 bytes available for write in the output buffer, there's no reason
    // to take the risk to continue.
    if (Lcd::writable() < 4) {
      return;
    }
    // It is now safe to assume that all writes of 4 bytes to the display buffer
    // will not block.
    
    if (previous_blink_counter_ > Lcd::blink_counter()) {
      ++blink_;
      status_ = 0;
    }
    previous_blink_counter_ = Lcd::blink_counter();

    uint8_t character = 0;
    // Determine which character to show at the current position.
    // If the scan position is the cursor and it is shown (blinking), draw the
    // cursor.
    if (scan_position_ == cursor_position_ && (blink_ & 2)) {
      character = cursor_character_;
    } else {
      // Otherwise, check if there's a status indicator to display. It is
      // displayed either on the left or right of the first line, depending on
      // the available space.
      if (status_ && (scan_position_ == 0 || scan_position_ == (width - 1)) &&
          local_[scan_position_] == ' ') {
        character = status_ - 1;
      } else {
        character = local_[scan_position_];
      }
    }
    // Check whether the screen really has to be updated to show the character.
    if (character != remote_[scan_position_]) {
      // There is a character to transmit!
      // If the new character to transmit is just after the previous one, and on
      // the same line, we're good, we don't need to reposition the cursor.
      if ((scan_position_ == scan_position_last_write_ + 1) && scan_column_) {
        // We use overwrite because we have checked before that there is
        // enough room in the buffer.
        Lcd::WriteData(character);
      } else {
        // The character to transmit is at a different position, we need to move
        // the cursor, and determine the cursor move command argument.
        Lcd::MoveCursor(scan_row_, scan_column_);
        Lcd::WriteData(character);
      }
      // We can now assume that the remote display will be updated.
      remote_[scan_position_] = character;
      scan_position_last_write_ = scan_position_;
    }
    ++scan_column_;
    ++scan_position_;
    if (scan_column_ == width) {
      scan_column_ = 0;
      ++scan_row_;
      if (scan_row_ == height) {
        scan_row_ = 0;
        scan_position_ = 0;
      }
    }
  }

 private:
  // Character pages storing what the display currently shows (remote), and
  // what it ought to show (local).
  static uint8_t local_[width * height + 1];
  static uint8_t remote_[width * height];

  // Position of the last character being transmitted.
  static uint8_t scan_position_;
  static uint8_t scan_row_;
  static uint8_t scan_column_;
  static uint8_t scan_position_last_write_;
  static uint8_t blink_;
  static uint8_t previous_blink_counter_;
  static uint8_t cursor_position_;
  static uint8_t cursor_character_;
  static uint8_t status_;

  DISALLOW_COPY_AND_ASSIGN(BufferedDisplay);
};

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::local_[width * height + 1];

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::remote_[width * height];

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::scan_position_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::scan_row_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::scan_column_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::scan_position_last_write_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::blink_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::previous_blink_counter_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::cursor_position_;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::cursor_character_ = kLcdCursor;

/* static */
template<typename Lcd>
uint8_t BufferedDisplay<Lcd>::status_;

}  // namespace avrlibx

#endif   // AVRLIBX_DEVICES_BUFFERED_DISPLAY_H_
