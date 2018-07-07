// Copyright 2015 Olivier Gillet.
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

#include "stmlib/dsp/dsp.h"
#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/system_clock.h"

#include "rings/drivers/adc.h"
#include "rings/drivers/codec.h"
#include "rings/drivers/leds.h"
#include "rings/drivers/switches.h"
#include "rings/drivers/system.h"
#include "rings/drivers/version.h"

#include "rings/meter.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include <cstring>

using namespace rings;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;
const uint32_t kStartAddress = 0x08008000;

Adc adc;
Codec codec;
Demodulator demodulator;
Leds leds;
Meter meter;
PacketDecoder decoder;
Switches switches;

int __errno;

// Default interrupt handlers.
extern "C" {

void NMI_Handler() { }
void HardFault_Handler() { while (1); }
void MemManage_Handler() { while (1); }
void BusFault_Handler() { while (1); }
void UsageFault_Handler() { while (1); }
void SVC_Handler() { }
void DebugMon_Handler() { }
void PendSV_Handler() { }

}

extern "C" {

enum UiState {
  UI_STATE_WAITING,
  UI_STATE_RECEIVING,
  UI_STATE_ERROR,
  UI_STATE_WRITING
};

volatile bool switch_released = false;
volatile UiState ui_state;
volatile int32_t gain = 4096;
volatile uint8_t pot_index = 0;

void UpdateLeds() {
  switch (ui_state) {
    case UI_STATE_WAITING:
      {
        // Alternate orange.
        bool on = system_clock.milliseconds() & 128;
        leds.set(0, on, on);
        leds.set(1, !on, !on);
      }
      break;

    case UI_STATE_RECEIVING:
      {
        // First LED: fast green. Second LED: peak-meter.
        int32_t peak = meter.peak();
        bool on = system_clock.milliseconds() & 32;
        uint8_t pwm = system_clock.milliseconds() & 15;
        leds.set(0, false, on);
        if (peak < 8192) {
          leds.set(1, false, (peak >> 9) >= pwm);
        } else if (peak < 16384) {
          leds.set(1, ((peak - 8192) >> 9) >= pwm, true);
        } else if (peak < 16384 + 8192) {
          leds.set(1, true, ((peak - 16384 - 8192) >> 9) < pwm);
        } else {
          leds.set(1, true, false);
        }
      }
      break;

    case UI_STATE_ERROR:
      {
        // Alternate red.
        bool on = system_clock.milliseconds() & 64;
        leds.set(0, on, 0);
        leds.set(1, !on, 0);
      }
      break;

    case UI_STATE_WRITING:
      {
        // Full orange.
        leds.set(0, true, true);
        leds.set(1, true, true);
      }
      break;
  }
  leds.Write();
}

void SysTick_Handler() {
  system_clock.Tick();
  adc.Convert();
  int32_t gain_raw = adc.value(ADC_CHANNEL_POT_FREQUENCY) >> 1;
  gain = gain_raw * gain_raw >> 16;
  switches.Debounce();
  if (switches.released(0)) {
    switch_released = true;
  }
  UpdateLeds();
}

}

size_t discard_samples = 8000;

void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    input[i].r = Clip16(static_cast<int32_t>(input[i].r) * gain >> 12);
  }
  meter.Process(input, n);
  while (n--) {
    if (!discard_samples) {
      demodulator.PushSample(2048 + (input->r >> 4));
    } else {
      --discard_samples;
    }
    *output = *input;
    ++output;
    ++input;
  }
}

static size_t current_address;
static uint16_t packet_index;
static uint32_t kSectorBaseAddress[] = {
  0x08000000,
  0x08004000,
  0x08008000,
  0x0800C000,
  0x08010000,
  0x08020000,
  0x08040000,
  0x08060000,
  0x08080000,
  0x080A0000,
  0x080C0000,
  0x080E0000
};
const uint32_t kBlockSize = 16384;
const uint16_t kPacketsPerBlock = kBlockSize / kPacketSize;
uint8_t rx_buffer[kBlockSize];

void ProgramPage(const uint8_t* data, size_t size) {
  FLASH_Unlock();
  FLASH_ClearFlag(
      FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
      FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR); 
  for (int32_t i = 0; i < 12; ++i) {
    if (current_address == kSectorBaseAddress[i]) {
      FLASH_EraseSector(i * 8, VoltageRange_3);
    }
  }
  const uint32_t* words = static_cast<const uint32_t*>(
      static_cast<const void*>(data));
  for (size_t written = 0; written < size; written += 4) {
    FLASH_ProgramWord(current_address, *words++);
    current_address += 4;
  }
}

void Init() {
  System sys;
  Version version;
  sys.Init(false);
  leds.Init();
  meter.Init(48000);
  switches.Init();
  version.Init();
  if (!codec.Init(!version.revised(), 48000)) { }
  if (!codec.Start(24, &FillBuffer)) { }
  sys.StartTimers();
  adc.Init();
}

void InitializeReception() {
  decoder.Init(20000);
  demodulator.Init(
      kModulationRate / kSampleRate * 4294967296.0,
      kSampleRate / kModulationRate,
      2.0 * kSampleRate / kBitRate);
  demodulator.SyncCarrier(true);
  decoder.Reset();
  current_address = kStartAddress;
  packet_index = 0;
  ui_state = UI_STATE_WAITING;
}

int main(void) {
  InitializeReception();
  Init();

  bool exit_updater = !switches.pressed_immediate(0);
  
  while (!exit_updater) {
    bool error = false;

    if (demodulator.state() == DEMODULATOR_STATE_OVERFLOW) {
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
            ui_state = UI_STATE_RECEIVING;
            memcpy(
                rx_buffer + (packet_index % kPacketsPerBlock) * kPacketSize,
                decoder.packet_data(),
                kPacketSize);
            ++packet_index;
            if ((packet_index % kPacketsPerBlock) == 0) {
              ui_state = UI_STATE_WRITING;
              ProgramPage(rx_buffer, kBlockSize);
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
      ui_state = UI_STATE_ERROR;
      switch_released = false;
      while (!switch_released);  // Polled in ISR
      InitializeReception();
    }
  }
  codec.Stop();
  adc.DeInit();
  Uninitialize();
  JumpTo(kStartAddress);
  while (1) { }
}
