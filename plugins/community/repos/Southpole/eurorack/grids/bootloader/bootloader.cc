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

#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>

#include "avrlib/gpio.h"
#include "avrlib/serial.h"
#include "avrlib/watchdog_timer.h"

#include "avr_audio_bootloader/crc32.h"
#include "avr_audio_bootloader/fsk/decoder.h"

#include "grids/hardware_config.h"

using namespace avrlib;
using namespace grids;
using namespace avr_audio_bootloader;

MidiInput midi;
Inputs inputs;
Leds leds;
Decoder decoder;

uint16_t page = 0;
uint8_t rx_buffer[SPM_PAGESIZE + 4];

void (*main_entry_point)(void) = 0x0000;

inline void Init() {
  cli();
  leds.set_mode(DIGITAL_OUTPUT);
  inputs.set_mode(DIGITAL_INPUT);
  inputs.EnablePullUpResistors();
}

void WriteBufferToFlash() {
  uint16_t i;
  const uint8_t* p = rx_buffer;
  eeprom_busy_wait();

  boot_page_erase(page);
  boot_spm_busy_wait();

  for (i = 0; i < SPM_PAGESIZE; i += 2) {
    uint16_t w = *p++;
    w |= (*p++) << 8;
    boot_page_fill(page + i, w);
  }

  boot_page_write(page);
  boot_spm_busy_wait();
  boot_rww_enable();
}

void FlashLedsOk() {
  for (uint8_t i = 0; i < 8; ++i) {
    _delay_ms(50);
    leds.Write(0xf);
    _delay_ms(50);
    leds.Write(LED_CLOCK);
  }
}

void FlashLedsError() {
  for (uint8_t i = 0; i < 5; ++i) {
    _delay_ms(120);
    leds.Write(0xf);
    _delay_ms(120);
    leds.Write(0x0);
  }
}

// MIDI loader -----------------------------------------------------------------

static const uint8_t sysex_header[] = {
  0xf0,  // <SysEx>
  0x00, 0x21, 0x02,  // Mutable instruments manufacturer id.
  0x00, 0x09,  // Product ID for Grids.
};

enum SysExReceptionState {
  MATCHING_HEADER = 0,
  READING_COMMAND = 1,
  READING_DATA = 2,
};

inline void LoaderLoop() {
  uint8_t byte;
  uint16_t bytes_read = 0;
  uint16_t rx_buffer_index;
  uint8_t state = MATCHING_HEADER;
  uint8_t checksum;
  uint8_t sysex_commands[2];
  uint8_t status = 0;
  uint8_t page_byte = 0;

  midi.Init();
  decoder.Init();
  decoder.Sync();
  decoder.set_packet_destination(rx_buffer);
  page = 0;

  TCCR2A = 0;
  TCCR2B = 2;

  while (1) {
    leds.Write(LED_CLOCK | (LED_BD >> ((page_byte + 3) & 0x3)));

    // Sample the clock input at 20kHz and feed to the FSK decoder.
    if (TCNT2 >= (F_CPU / 8 / 40000 - 1)) {
      TCNT2 = 0;
      
      switch (decoder.PushSample(inputs.Read() & INPUT_CLOCK)) {
        case DECODER_STATE_ERROR_SYNC:
          FlashLedsError();
          decoder.Sync();
          break;

        case DECODER_STATE_END_OF_TRANSMISSION:
          return;
          break;

        case DECODER_STATE_PACKET_RECEIVED:
          {
            uint32_t crc = crc32(0, rx_buffer, SPM_PAGESIZE);
            uint32_t expected_crc = \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 0]) << 24) | \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 1]) << 16) | \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 2]) <<  8) | \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 3]) <<  0);
            if (crc == expected_crc) {
              WriteBufferToFlash();
              page += SPM_PAGESIZE;
              ++page_byte;
            } else {
              FlashLedsError();
            }
          }
          decoder.Sync();
          break;

        default:
          break;
      }
    }
    
    // Poll the MIDI input.
    if (midi.readable()) {
      byte = midi.ImmediateRead();
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
            status = 0;
            bytes_read = 0;
          }
          break;

        case READING_DATA:
          if (byte < 0x80) {
            if (bytes_read & 1) {
              rx_buffer[rx_buffer_index] |= byte & 0xf;
              if (rx_buffer_index < SPM_PAGESIZE) {
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
            } else if (rx_buffer_index == SPM_PAGESIZE + 1 &&
                       sysex_commands[0] == 0x7e &&
                       sysex_commands[1] == 0x00 &&
                       rx_buffer[rx_buffer_index - 1] == checksum) {
              // Block write.
              WriteBufferToFlash();
              page += SPM_PAGESIZE;
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
}

int main(void) {
  ResetWatchdog();
  Init();
  _delay_ms(40);
  if (!(inputs.Read() & INPUT_SW_RESET)) {
    FlashLedsOk();
    LoaderLoop();
    FlashLedsOk();
    FlashLedsOk();
  }
  main_entry_point();
}
