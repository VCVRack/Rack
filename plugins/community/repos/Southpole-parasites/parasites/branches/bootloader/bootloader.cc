// Copyright 2013 Olivier Gillet.
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
// Simple FSK bootloader

#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>

#include "avrlib/gpio.h"
#include "avrlib/serial.h"
#include "avrlib/watchdog_timer.h"

#include "avr_audio_bootloader/fsk/decoder.h"

using namespace avrlib;
using namespace avr_audio_bootloader;

Gpio<PortD, 1> led_1_a;
Gpio<PortD, 2> led_1_k;
Gpio<PortB, 1> led_2_a;
Gpio<PortB, 0> led_2_k;
Gpio<PortD, 4> in_1;
Gpio<PortC, 3> switch_1;

Decoder decoder;

uint16_t page = 0;
uint8_t rx_buffer[SPM_PAGESIZE + 4];

int main(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));

inline void Init() {
  cli();  
  switch_1.set_mode(DIGITAL_INPUT);
  switch_1.High();
  led_1_a.set_mode(DIGITAL_OUTPUT);
  led_1_k.set_mode(DIGITAL_OUTPUT);
  led_2_a.set_mode(DIGITAL_OUTPUT);
  led_2_k.set_mode(DIGITAL_OUTPUT);
  in_1.set_mode(DIGITAL_INPUT);
  in_1.High();
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

void FlashLeds(bool error) {
  for (uint8_t i = 0; i < 8; ++i) {
    _delay_ms(20);
    if (error) {
      led_1_a.High();
      led_1_k.Low();
    } else {
      led_1_a.Low();
      led_1_k.High();
    }
    _delay_ms(20);
    led_1_a.Low();
    led_1_k.Low();
  }
}

uint32_t crc32 (uint32_t crc, uint8_t* buffer, uint8_t length) {
  crc = crc ^ ~0UL;
  for (uint8_t i = 0; i < length; ++i)  {
    crc = crc ^ *buffer++;
    for (uint8_t j = 0; j < 8; j++)  { 
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320; 
      } else {
        crc = crc >> 1; 
      } 
    }
  }
  crc = crc ^ ~0UL;
  return crc;
}

inline void LoaderLoop() {
  uint8_t page_byte = 0;

  decoder.Init();
  decoder.Sync();
  decoder.set_packet_destination(rx_buffer);
  page = 0;

  TCCR2A = 0;
  TCCR2B = 2;

  while (1) {
    // Sample the clock input at 15625 Hz and feed to the FSK decoder.
    if (TCNT2 >= (8000000 / 8 / 15625 - 1)) {
      TCNT2 = 0;
      led_2_a.set_value(page_byte & 1);
      led_2_k.set_value(!(page_byte & 1));
      switch (decoder.PushSample(in_1.value())) {
        case DECODER_STATE_ERROR_SYNC:
          FlashLeds(true);
          decoder.Sync();
          break;

        case DECODER_STATE_END_OF_TRANSMISSION:
          return;
          break;

        case DECODER_STATE_PACKET_RECEIVED:
          {
#ifdef DO_NOT_CHECK_CRC
            uint32_t crc = 0;
            uint32_t expected_crc = 0;    
#else            
            uint32_t crc = crc32(0, rx_buffer, SPM_PAGESIZE);
            uint32_t expected_crc = \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 0]) << 24) | \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 1]) << 16) | \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 2]) <<  8) | \
                (static_cast<uint32_t>(rx_buffer[SPM_PAGESIZE + 3]) <<  0);
#endif  // DO_NOT_CHECK_CRC               
            if (crc == expected_crc) {
              WriteBufferToFlash();
              page += SPM_PAGESIZE;
              ++page_byte;
            } else {
              FlashLeds(true);
            }
          }
          decoder.Sync();
          break;

        default:
          break;
      }
    }
  }
}

int main(void) {
  ResetWatchdog();
  Init();
  if (!switch_1.value()) {
    FlashLeds(false);
    LoaderLoop();
    FlashLeds(false);
  }
  void (*main_entry_point)(void) = 0x0000;
  main_entry_point();
}
