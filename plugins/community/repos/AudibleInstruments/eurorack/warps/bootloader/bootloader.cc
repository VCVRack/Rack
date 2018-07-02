// Copyright 2014 Olivier Gillet.
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

#include "warps/drivers/adc.h"
#include "warps/drivers/codec.h"
#include "warps/drivers/leds.h"
#include "warps/drivers/switches.h"
#include "warps/drivers/system.h"
#include "warps/drivers/version.h"
#include "warps/meter.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include <cstring>

using namespace warps;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;
const uint32_t kStartAddress = 0x08008000;

Adc adc;
Codec codec;
Meter meter;
Leds leds;
Switches switches;
PacketDecoder decoder;
Demodulator demodulator;

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
volatile int32_t gain = 4096;
volatile UiState ui_state;

void UpdateLeds() {
  leds.Clear();
  
  bool blink = system_clock.milliseconds() & 128;
  
  int32_t color = meter.peak();
  color = color < 8192 ? (color - 8192) << 2 : ((color - 8192) * 341 >> 8);
  if (color < 0) {
    leds.set_main(0, (color + 32768) >> 7, 0);
  } else if (color < 16384) {
    leds.set_main(color >> 6, 255, 0);
  } else {
    leds.set_main(255, 255 - ((color - 16384) >> 6), 0);
  }
  
  switch (ui_state) {
    case UI_STATE_WAITING:
      leds.set_osc(blink ? 255 : 0, blink ? 255 : 0);
      break;

    case UI_STATE_RECEIVING:
      leds.set_osc(0, system_clock.milliseconds() & 32 ? 255 : 0);
      break;
    
    case UI_STATE_ERROR:
      leds.set_osc(blink ? 255 : 0, 0);
      leds.set_main(255, 0, 0);
      break;
      
    case UI_STATE_WRITING:
      leds.set_osc(0, 255);
      leds.set_main(0, 255, 0);
      break;
  }
  leds.Write();
}

void SysTick_Handler() {
  system_clock.Tick();
  int32_t gain_raw = adc.value(ADC_LEVEL_1_POT) >> 1;
  gain = gain_raw * gain_raw >> 14;
  
  switches.Debounce();
  if (switches.released(0)) {
    switch_released = true;
  }
  UpdateLeds();
}

}

size_t discard_samples = 8000;

void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t n) {
  adc.Convert();
  for (size_t i = 0; i < n; ++i) {
    input[i].l = Clip16(static_cast<int32_t>(input[i].l) * gain >> 12);
  }
  meter.Process(input, n);
  while (n--) {
    int32_t sample = (input->l >> 4) + 2048;
    if (!discard_samples) {
      demodulator.PushSample(sample);
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
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
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
  version.Init();
  switches.Init();
  if (!codec.Init(!version.revised(), 48000)) { }
  if (!codec.Start(48, &FillBuffer)) { }
  adc.Init();
  sys.StartTimers();
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
