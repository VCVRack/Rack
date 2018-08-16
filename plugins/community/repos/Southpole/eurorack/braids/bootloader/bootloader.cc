// Copyright 2012 Olivier Gillet.
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

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/flash_programming.h"
#include "stmlib/system/system_clock.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include "braids/drivers/adc.h"
#include "braids/drivers/display.h"
#include "braids/drivers/encoder.h"
#include "braids/drivers/system.h"

using namespace braids;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;

const uint32_t kStartAddress = 0x08004000;

Adc adc;
System sys;
Display display;
Encoder encoder;
PacketDecoder decoder;
Demodulator demodulator;

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

volatile uint8_t packet_inspector_byte = 0;
volatile bool encoder_released = false;

void SysTick_Handler() {
  system_clock.Tick();  // Tick global ms counter.
  encoder.Debounce();
  encoder_released = encoder_released | encoder.released();
  packet_inspector_byte += encoder.increment();
  uint32_t ms_clock = system_clock.milliseconds();
  if ((ms_clock & 0x3f) == 0 && display.mutable_buffer()[0] >= '\x98') {
    display.mutable_buffer()[0] = '\x98' + ((ms_clock >> 6) & 7);
  }
  display.Refresh();
}

uint16_t discard_samples = 8000;

void TIM1_UP_IRQHandler(void) {
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == RESET) {
    return;
  }
  
  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  if (adc.PipelinedRead(3)) {
    if (!discard_samples) {
      int16_t sample = adc.channel(3);
      demodulator.PushSample(sample);
    } else {
      --discard_samples;
    }
  }
}

}

static uint32_t current_address;
static uint16_t packet_index;

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

void PrintPageNumber(uint16_t page_number, bool error) {
  char string[5];
  string[0] = '\x98';
  string[1] = error ? 'X' : '0' + page_number / 100;
  string[2] = '0' + (page_number / 10) % 10;
  string[3] = '0' + page_number % 10;
  string[4] = '\0';
  display.Print(string);
}

const char kHexChar[] = "0123456789ABCDEF";
void PacketInspector() {
  while (1) {
    char string[5];
    string[0] = kHexChar[packet_inspector_byte >> 4];
    string[1] = kHexChar[packet_inspector_byte & 0xf];
    string[2] = kHexChar[decoder.packet_data()[packet_inspector_byte] >> 4];
    string[3] = kHexChar[decoder.packet_data()[packet_inspector_byte] & 0xf];
    string[4] = '\0';
    display.Print(string);
  }
}

void Init() {
  sys.Init(F_CPU / (3 * kSampleRate) - 1, false);
  system_clock.Init();
  adc.Init(3 * kSampleRate > 96000);
  encoder.Init();
  display.Init();
  sys.StartTimers();
}

void InitializeReception() {
  decoder.Init();
  demodulator.Init(
      kModulationRate / kSampleRate * 4294967296.0,
      kSampleRate / kModulationRate,
      2.0 * kSampleRate / kBitRate);
  demodulator.SyncCarrier(true);
  decoder.Reset();
  current_address = kStartAddress;
  packet_index = 0;
  display.Print("\x98RDY");
}

uint8_t rx_buffer[PAGE_SIZE];
const uint16_t kPacketsPerPage = PAGE_SIZE / kPacketSize;
const char* kErrorStrings[2] = { "@SYN", "@CRC", };

int main(void) {
  Init();
  InitializeReception();

  bool exit_updater = !encoder.pressed_immediate();
  while (!exit_updater) {
    bool error = false;

    if (demodulator.state() == DEMODULATOR_STATE_OVERFLOW) {
      display.Print("@OVF");
      error = true;
    } else {
      demodulator.ProcessAtLeast(32);
    }
    
    while (demodulator.available() && !error && !exit_updater) {
      uint8_t symbol = demodulator.NextSymbol();
      PacketDecoderState state = decoder.ProcessSymbol(symbol);
      switch (state) {
        case PACKET_DECODER_STATE_OK:
          {
            memcpy(
                rx_buffer + (packet_index % kPacketsPerPage) * kPacketSize,
                decoder.packet_data(),
                kPacketSize);
            ++packet_index;
            if ((packet_index % kPacketsPerPage) == 0) {
              PrintPageNumber(packet_index / kPacketsPerPage, false);
              ProgramPage(rx_buffer, PAGE_SIZE);
              decoder.Reset();
              demodulator.SyncCarrier(false);
            } else {
              decoder.Reset();
              demodulator.SyncDecision();
            }
          }
          break;
        case PACKET_DECODER_STATE_ERROR_SYNC:
        case PACKET_DECODER_STATE_ERROR_CRC:
          display.Print(kErrorStrings[state - PACKET_DECODER_STATE_ERROR_SYNC]);
          error = true;
          break;
        case PACKET_DECODER_STATE_END_OF_TRANSMISSION:
          exit_updater = true;
          break;
        default:
          break;
      }
    }
    if (error) {
      encoder_released = false;
      while (!encoder_released);  // Polled in ISR
      InitializeReception();
    }
  }
  
  Uninitialize();
  JumpTo(kStartAddress);
  while (1) { }
}
