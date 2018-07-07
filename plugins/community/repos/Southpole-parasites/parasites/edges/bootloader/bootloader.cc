// Copyright 2011 Olivier Gillet.
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
// Bootloader supporting MIDI SysEx update.
//
// Caveat: assumes the firmware flashing is always done from first to last
// block, in increasing order. Random access flashing is not supported!

#include "avrlibx/io/gpio.h"
#include "avrlibx/io/usart.h"
#include "avrlibx/system/init.h"
#include "avrlibx/system/time.h"

#include "avrlibx/third_party/sp_driver/sp_driver.h"

#include "edges/hardware_config.h"

using namespace avrlibx;
using namespace edges;

MidiIO midi;
Leds leds;
Switches switches;

uint16_t page = 0;
uint8_t rx_buffer[APP_SECTION_PAGE_SIZE + 1];

typedef void (*MainEntryPoint)(void) __attribute__ ((noreturn));
MainEntryPoint main_entry_point = (MainEntryPoint)0x0000;

inline void Init() {
  SysInit();
  leds.set_direction(OUTPUT);
  switches.set_direction(INPUT);
  switches.set_mode(PORT_MODE_PULL_UP);
  midi.Init();
}

void WriteBufferToFlash() {
  SP_LoadFlashPage(rx_buffer);
  SP_EraseWriteApplicationPage(page);
  SP_WaitForSPM();
  
  NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}

void FlashLedsOk() {
  for (uint8_t i = 0; i < 8; ++i) {
    leds.Write(0xf);
    ConstantDelay(50);
    leds.Write(8);
    ConstantDelay(50);
  }
}

void FlashLedsError() {
  leds.Write(0);
  for (uint8_t i = 0; i < 5; ++i) {
    ConstantDelay(100);
    leds.Toggle();
  }
}

static const uint8_t sysex_header[] = {
  0xf0,  // <SysEx>
  0x00, 0x21, 0x02,  // Mutable instruments manufacturer id.
  0x00, 0x0a,  // Product ID for "any product".
};

enum SysExReceptionState {
  MATCHING_HEADER = 0,
  READING_COMMAND = 1,
  READING_DATA = 2,
};

inline void MidiLoop() {
  uint8_t byte;
  uint16_t bytes_read = 0;
  uint16_t rx_buffer_index;
  uint8_t state = MATCHING_HEADER;
  uint8_t checksum;
  uint8_t sysex_commands[2];
  uint8_t page_byte = 0;

  midi.Init();
  page = 0;
  
  while (1) {
    leds.Write(8 | (4 >> ((page_byte + 3) & 0x3)));
    byte = midi.Read();
    // In case we see a realtime message in the stream, safely ignore it.
    if (byte > 0xf0 && byte != 0xf7) {
      continue;
    }
    switch (state) {
      case MATCHING_HEADER:
        if (byte == sysex_header[bytes_read]) {
          ++bytes_read;
          if (bytes_read == sizeof(sysex_header)) {
            bytes_read = 0;
            state = READING_COMMAND;
          }
        } else {
          bytes_read = 0;
        }
        break;

      case READING_COMMAND:
        if (byte < 0x80) {
          sysex_commands[bytes_read++] = byte;
          if (bytes_read == 2) {
            bytes_read = 0;
            rx_buffer_index = 0;
            checksum = 0;
            state = READING_DATA;
          }
        } else {
          state = MATCHING_HEADER;
          bytes_read = 0;
        }
        break;

      case READING_DATA:
        if (byte < 0x80) {
          if (bytes_read & 1) {
            rx_buffer[rx_buffer_index] |= byte & 0xf;
            if (rx_buffer_index < APP_SECTION_PAGE_SIZE) {
              checksum += rx_buffer[rx_buffer_index];
            }
            ++rx_buffer_index;
          } else {
            rx_buffer[rx_buffer_index] = (byte << 4);
          }
          ++bytes_read;
        } else if (byte == 0xf7) {
          if (sysex_commands[0] == 0x7f &&
              sysex_commands[1] == 0x00 &&
              bytes_read == 0) {
            // Reset.
            return;
          } else if (rx_buffer_index == APP_SECTION_PAGE_SIZE + 1 &&
                     sysex_commands[0] == 0x7e &&
                     sysex_commands[1] == 0x00 &&
                     rx_buffer[rx_buffer_index - 1] == checksum) {
            // Block write.
            WriteBufferToFlash();
            page += APP_SECTION_PAGE_SIZE;
            ++page_byte;
          } else {
            FlashLedsError();
          }
          state = MATCHING_HEADER;
          bytes_read = 0;
        }
        break;
    }
  }
}

int main(void) {
  Init();
  ConstantDelay(25);
  if (!(switches.Read() & 1)) {
    FlashLedsOk();
    MidiLoop();
    FlashLedsOk();
  }
  SP_LockSPM();
  EIND = 0x00;
  main_entry_point();
}
