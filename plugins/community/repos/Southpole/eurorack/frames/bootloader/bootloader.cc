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

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/flash_programming.h"
#include "stmlib/system/system_clock.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include "frames/drivers/adc.h"
#include "frames/drivers/switches.h"
#include "frames/drivers/channel_leds.h"
#include "frames/drivers/rgb_led.h"
#include "frames/drivers/system.h"

using namespace frames;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;

const uint32_t kStartAddress = 0x08004000;

Adc adc;
System sys;
ChannelLeds leds;
RgbLed rgb;
Switches switches;
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

enum UiState {
  UI_STATE_WAITING,
  UI_STATE_RECEIVING,
  UI_STATE_ERROR,
  UI_STATE_PACKET_OK
};

volatile bool switch_released = false;
volatile UiState ui_state;

inline void UpdateLeds() {
  switch (ui_state) {
    case UI_STATE_WAITING:
      {
        bool on = system_clock.milliseconds() & 256;
        rgb.set_color(255, 128, 0);
        leds.set_channel(0, on ? 255 : 0);
        leds.set_channel(1, on ? 255 : 0);
        leds.set_channel(2, on ? 255 : 0);
        leds.set_channel(3, on ? 255 : 0);
      }
      break;

    case UI_STATE_RECEIVING:
      {
        uint8_t stage = (system_clock.milliseconds() >> 7) & 3;
        rgb.set_color(0, 255, 0);
        leds.set_channel(0, stage == 0 ? 255 : 0);
        leds.set_channel(1, stage == 1 ? 255 : 0);
        leds.set_channel(2, stage == 2 ? 255 : 0);
        leds.set_channel(3, stage == 3 ? 255 : 0);
      }
      break;

    case UI_STATE_ERROR:
      {
        bool on = system_clock.milliseconds() & 128;
        rgb.set_color(255, 0, 0);
        leds.set_channel(0, on ? 255 : 0);
        leds.set_channel(1, on ? 255 : 0);
        leds.set_channel(2, on ? 255 : 0);
        leds.set_channel(3, on ? 255 : 0);
      }
      break;

    case UI_STATE_PACKET_OK:
      {
        rgb.set_color(64, 255, 128);
        leds.set_channel(0, 255);
        leds.set_channel(1, 255);
        leds.set_channel(2, 255);
        leds.set_channel(3, 255);
      }
      break;
  }
  rgb.Write();
  leds.Write();
}

void SysTick_Handler() {
  system_clock.Tick();  // Tick global ms counter.
  switches.Debounce();
  if (switches.released(0)) {
    switch_released = true;
  }
  UpdateLeds();
}

uint16_t discard_samples = 8000;

void TIM1_UP_IRQHandler(void) {
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == RESET) {
    return;
  }
  
  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  
  if (!discard_samples) {
    int16_t sample = adc.value(0) >> 4;
    demodulator.PushSample(sample);
  } else {
    --discard_samples;
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

void Init() {
  sys.Init(F_CPU / kSampleRate - 1, false);
  system_clock.Init();
  adc.Init(true);
  switches.Init();
  leds.Init();
  rgb.Init();
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
  ui_state = UI_STATE_WAITING;
}

uint8_t rx_buffer[PAGE_SIZE];
const uint16_t kPacketsPerPage = PAGE_SIZE / kPacketSize;

int main(void) {
  Init();
  InitializeReception();

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
                rx_buffer + (packet_index % kPacketsPerPage) * kPacketSize,
                decoder.packet_data(),
                kPacketSize);
            ++packet_index;
            if ((packet_index % kPacketsPerPage) == 0) {
              ui_state = UI_STATE_PACKET_OK;
              ProgramPage(rx_buffer, PAGE_SIZE);
              decoder.Reset();
              demodulator.SyncCarrier(false);
              ui_state = UI_STATE_RECEIVING;
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
