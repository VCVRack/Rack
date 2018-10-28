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

#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/system_clock.h"

#include "marbles/drivers/adc.h"
#include "marbles/drivers/dac.h"
#include "marbles/drivers/leds.h"
#include "marbles/drivers/switches.h"
#include "marbles/drivers/system.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include <cstring>

using namespace marbles;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;

const uint32_t kStartAddress = 0x08008000;

Adc adc;
Dac dac;
Leds leds;
Switches switches;
PacketDecoder decoder;
Demodulator demodulator;

int __errno;

void UpdateLeds();
volatile bool switch_released = false;

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

void SysTick_Handler() {
  IWDG_ReloadCounter();
  system_clock.Tick();
  switches.Debounce();
  if (switches.released(SWITCH_T_DEJA_VU)) {
    switch_released = true;
  }
  UpdateLeds();
}

}

enum UiState {
  UI_STATE_WAITING,
  UI_STATE_RECEIVING,
  UI_STATE_ERROR,
  UI_STATE_WRITING
};

volatile UiState ui_state;
volatile int32_t peak;

void UpdateLeds() {
  leds.Clear();
  switch (ui_state) {
    case UI_STATE_WAITING:
      leds.set(
          LED_T_DEJA_VU,
          system_clock.milliseconds() & 128 ? LED_COLOR_GREEN : 0);
      leds.set(
          LED_X_DEJA_VU,
          system_clock.milliseconds() & 128 ? 0 : LED_COLOR_GREEN);
      break;

    case UI_STATE_RECEIVING:
      leds.set(
          LED_T_DEJA_VU,
          system_clock.milliseconds() & 32 ? LED_COLOR_GREEN : 0);
      leds.set(
          LED_X_DEJA_VU,
          system_clock.milliseconds() & 32 ? 0 : LED_COLOR_GREEN);
      break;

    case UI_STATE_ERROR:
      {
        bool on = system_clock.milliseconds() & 256;
        for (int i = 0; i < LED_LAST; ++i) {
          leds.set(Led(i), on ? LED_COLOR_RED : 0);
        }
      }
      break;

    case UI_STATE_WRITING:
      {
        for (int i = 0; i < LED_LAST; ++i) {
          leds.set(Led(i), LED_COLOR_GREEN);
        }
      }
      break;
  }
  
  if (ui_state != UI_STATE_WRITING) {
    uint8_t pwm = system_clock.milliseconds() & 15;
    if (peak < 8192) {
      leds.set(
          LED_T_RANGE,
          (peak >> 9) > pwm ? LED_COLOR_GREEN : 0);
    } else if (peak < 16384) {
      leds.set(
          LED_T_RANGE,
          ((peak - 8192) >> 9) >= pwm ? LED_COLOR_YELLOW : LED_COLOR_GREEN);
    } else if (peak < 16384 + 8192) {
      leds.set(
          LED_T_RANGE,
          ((peak - 16384 - 8192) >> 9) >= pwm ?
              LED_COLOR_RED : LED_COLOR_YELLOW);
    } else {
      leds.set(LED_T_RANGE, LED_COLOR_RED);
    }
  }
  
  leds.Write();
}

int32_t dc_offset = 0;
int32_t gain_pot = 16;
size_t discard_samples = 8000;

IOBuffer::Block block;

IOBuffer::Slice FillBuffer(size_t size) {
  adc.Convert();
  if (!discard_samples) {
    // Scan gain pot.
    gain_pot = (adc.value(ADC_GROUP_POT) + 4095 * gain_pot) >> 12;
    int32_t gain = ((gain_pot >> 1) * gain_pot >> 21) + 128;
    // Extract sample. Note: there's a DC offset :/
    int32_t sample = 32768 - static_cast<int32_t>(adc.value(ADC_GROUP_CV));
    dc_offset += (sample - (dc_offset >> 15));
    sample = (sample - (dc_offset >> 15)) * gain >> 8;  // 0.5x to 4x
    CONSTRAIN(sample, -32768, 32767);
    
    // Update peak-meter
    int32_t rect = sample > 0 ? sample : -sample;
    peak = rect > peak ? rect : (rect + 32767 * peak) >> 15;

    // Write to DAC for monitoring
    block.cv_output[0][0] = 32767 - sample;
    block.cv_output[1][0] = 32767 - sample;
    block.cv_output[2][0] = 32767 - sample;
    block.cv_output[3][0] = 32767 - sample;
    demodulator.PushSample(2048 + (sample >> 4));
  } else {
    --discard_samples;
  }
  
  IOBuffer::Slice s;
  s.block = &block;
  s.frame_index = 0;
  return s;
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
const uint16_t kPacketsPerBlock = ::kBlockSize / kPacketSize;
uint8_t rx_buffer[::kBlockSize];

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

void Init() {
  System sys;
  switches.Init();
  sys.Init(false);
  system_clock.Init();
  adc.Init(true);
  dac.Init(48000, 1);
  leds.Init();
  sys.StartTimers();
  dac.Start(&FillBuffer);
}

int main(void) {
  Init();
  InitializeReception();

  bool exit_updater = !switches.pressed_immediate(SWITCH_T_DEJA_VU);
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
              ProgramPage(rx_buffer, ::kBlockSize);
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
  adc.DeInit();
  Uninitialize();
  JumpTo(kStartAddress);
  while (1) { }
}
