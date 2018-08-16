// Copyright 2013 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.

#include <stm32f10x_conf.h>
#include <string.h>

#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/flash_programming.h"
#include "stmlib/system/system_clock.h"
#include "stmlib/utils/ring_buffer.h"

#include "yarns/drivers/display.h"
#include "yarns/drivers/encoder.h"
#include "yarns/drivers/midi_io.h"
#include "yarns/drivers/system.h"

using namespace yarns;
using namespace stmlib;

System sys;
Display display;
Encoder encoder;
MidiIO midi_io;
RingBuffer<uint8_t, 128> midi_in_buffer;

extern "C" {
  
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void NMI_Handler(void) { }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }

}

extern "C" {

void SysTick_Handler() {
  static uint8_t counter;
  if ((++counter & 7) == 0) {
    system_clock.Tick();  // Tick global ms counter.
    uint32_t ms_clock = system_clock.milliseconds();
    if ((ms_clock & 0x3f) == 0 && display.mutable_buffer()[0] >= '\x98') {
      display.mutable_buffer()[0] = '\x98' + ((ms_clock >> 6) & 7);
    }
    display.RefreshSlow();
    encoder.Debounce();
  }
  display.RefreshFast();
  
  // Try to read some MIDI input.
  if (midi_io.readable()) {
    midi_in_buffer.Overwrite(midi_io.ImmediateRead());
  }
}

}

void PrintByte(uint8_t byte) {
  char str[] = "00";
  str[0] += byte / 10;
  str[1] += byte % 10;
  display.Print(str);
}

static uint32_t current_address;

void ProgramPage(const uint8_t* data, size_t size) {
  FLASH_Unlock();
  FLASH_ErasePage(current_address);
  const uint32_t* words = static_cast<const uint32_t*>(
      static_cast<const void*>(data));
  for (size_t written = 0; written < size; written += 4) {
    FLASH_ProgramWord(current_address, *words++);
    current_address += 4;
  }
}

enum SysExReceptionState {
  MATCHING_HEADER = 0,
  READING_COMMAND = 1,
  READING_DATA = 2,
};

enum SysExReturnCode {
  SYSEX_RESULT_ERROR = 0,
  SYSEX_RESULT_OK = 1,
  SYSEX_RESULT_PAGE_WRITTEN = 2,
  SYSEX_RESULT_DONE = 3
};

static const uint8_t sysex_header[] = {
  0xf0,  // <SysEx>
  0x00, 0x21, 0x02,  // Mutable instruments manufacturer id.
  0x00, 0x0b,  // Product ID for Yarns.
};

static uint32_t bytes_read = 0;
static uint16_t rx_buffer_index = 0;
static SysExReceptionState state = MATCHING_HEADER;
static uint8_t checksum;
static uint8_t sysex_commands[2];
static uint8_t page_index = 0;
static uint8_t rx_buffer[PAGE_SIZE + 1];
static uint8_t total_bytes = 0;

const uint32_t kStartAddress = 0x08001000;

void PrepareReception() {
  bytes_read = 0;
  rx_buffer_index = 0;
  state = MATCHING_HEADER;
  page_index = 0;
  current_address = kStartAddress;
  midi_in_buffer.Init();
  
  display.Print("\x98 ");
}

SysExReturnCode ProcessSysExByte(uint8_t byte) {
  if (byte > 0xf0 && byte != 0xf7) {
    return SYSEX_RESULT_OK;
  }
  ++total_bytes;
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
        return SYSEX_RESULT_ERROR;
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
        return SYSEX_RESULT_ERROR;
      }
      break;

    case READING_DATA:
      if (byte < 0x80) {
        if (bytes_read & 1) {
          rx_buffer[rx_buffer_index] |= byte & 0xf;
          if (rx_buffer_index < PAGE_SIZE) {
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
          return SYSEX_RESULT_DONE;
        } else if (rx_buffer_index == PAGE_SIZE + 1 &&
                   sysex_commands[0] == 0x7e &&
                   sysex_commands[1] == 0x00 &&
                   rx_buffer[rx_buffer_index - 1] == checksum) {
          // Block write.
          ProgramPage(rx_buffer, PAGE_SIZE);
          ++page_index;
        }
        state = MATCHING_HEADER;
        bytes_read = 0;
        return SYSEX_RESULT_PAGE_WRITTEN;
      }
      break;
  }
  return SYSEX_RESULT_OK;
}

void Init() {
  SystemInit();
  
  RCC_APB2PeriphClockCmd(
      RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
      RCC_APB2Periph_TIM1 | RCC_APB2Periph_USART1, ENABLE);
  
  system_clock.Init();
  encoder.Init();
  display.Init();
  midi_io.Init();
  
  SysTick_Config(F_CPU / 8000);
}

int main(void) {
  Init();
  PrepareReception();
  bool exit_updater = !encoder.pressed_immediate();
  while (!exit_updater) {
    while (midi_in_buffer.readable() && !exit_updater) {
      SysExReturnCode result = ProcessSysExByte(midi_in_buffer.ImmediateRead());
      switch (result) {
        case SYSEX_RESULT_DONE:
          {
            display.Print("OK");
            system_clock.Delay(1000);
            exit_updater = true;
          }
          break;
          
        case SYSEX_RESULT_PAGE_WRITTEN:
          {
            PrintByte(page_index);
          }
          break;
          
        case SYSEX_RESULT_OK:
          break;
          
        case SYSEX_RESULT_ERROR:
          display.Print("Er");
          system_clock.Delay(1000);
          PrepareReception();
          break;
      }
    }
  }
  Uninitialize();
  JumpTo(kStartAddress);
  while (1) { }
}
