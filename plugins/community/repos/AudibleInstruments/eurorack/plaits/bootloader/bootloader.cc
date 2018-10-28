// Copyright 2016 Olivier Gillet.
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

#include <stm32f37x_conf.h>
#include <cstring>

#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/flash_programming.h"
#include "stmlib/system/system_clock.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include "plaits/drivers/audio_dac.h"
#include "plaits/drivers/firmware_update_adc.h"
#include "plaits/drivers/leds.h"
#include "plaits/drivers/switches.h"

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

using namespace plaits;
using namespace std;
using namespace stm_audio_bootloader;
using namespace stmlib;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;
const uint32_t kStartAddress = 0x08008000;
const uint16_t kPacketsPerPage = PAGE_SIZE / kPacketSize;

enum UiState {
  UI_STATE_WAITING,
  UI_STATE_RECEIVING,
  UI_STATE_ERROR,
  UI_STATE_WRITING
};

AudioDac audio_dac;
FirmwareUpdateAdc adc;
Leds leds;
Switches switches;
PacketDecoder decoder;
Demodulator demodulator;

int discard_samples = 8000;
int32_t peak = 0;
int32_t gain_pot = 0;
uint32_t current_address;
uint16_t packet_index;
uint8_t rx_buffer[PAGE_SIZE];

volatile bool switch_released = false;
volatile UiState ui_state;

inline void UpdateLeds() {
  leds.Clear();
  
  // Show bargraph on the upper 4 LEDs.
  int32_t pwm = system_clock.milliseconds() & 15;
  leds.set(3, (peak >> 9) > pwm ? LED_COLOR_GREEN : 0);
  leds.set(2, ((peak - 8192) >> 9) >= pwm ? LED_COLOR_GREEN : 0);
  leds.set(1, ((peak - 16384) >> 9) >= pwm ? LED_COLOR_YELLOW : 0);
  leds.set(0, ((peak - 16384 - 8192) >> 9) >= pwm ? LED_COLOR_RED : 0);
  
  // Show status info on the lower 4 LEDs.
  switch (ui_state) {
    case UI_STATE_WAITING:
      {
        bool on = system_clock.milliseconds() & 128;
        for (int i = 4; i < 8; ++i) {
          leds.set(i, on ? LED_COLOR_YELLOW : LED_COLOR_OFF);
        }
      }
      break;

    case UI_STATE_RECEIVING:
      {
        int stage = (system_clock.milliseconds() >> 7) & 3;
        leds.set(stage + 4, LED_COLOR_GREEN);
      }
      break;

    case UI_STATE_ERROR:
      {
        bool on = system_clock.milliseconds() & 256;
        for (int i = 0; i < 8; ++i) {
          leds.set(i, on ? LED_COLOR_RED : LED_COLOR_OFF);
        }
      }
      break;

    case UI_STATE_WRITING:
      {
        for (uint8_t i = 4; i < 8; ++i) {
          leds.set(i, LED_COLOR_GREEN);
        }
      }
      break;
    }
  leds.Write();
}

extern "C" {
  
void SysTick_Handler() {
  system_clock.Tick();
  switches.Debounce();
  if (switches.released(Switch(0))) {
    switch_released = true;
  }
  UpdateLeds();
}
  
}

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

void FillBuffer(AudioDac::Frame* output, size_t size) {
  gain_pot = (adc.gain_pot() + 4095 * gain_pot) >> 12;
  int32_t sample = 32768 - static_cast<int32_t>(adc.sample());
  adc.Convert();

  int32_t gain = ((gain_pot >> 1) * gain_pot >> 21) + 128;
  sample = sample * gain >> 8;
  CONSTRAIN(sample, -32767, 32767)
  
  int32_t rect = abs(sample);
  peak = rect > peak ? rect : (rect + 32767 * peak) >> 15;

  if (!discard_samples) {
    demodulator.PushSample(2048 + (sample >> 4));
  } else {
    --discard_samples;
  }
  output->l = -sample;
  output->r = -sample;
}

void InitializeReception() {
  decoder.Init(1000, true);
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
  adc.Init();
  leds.Init();
  switches.Init();
  audio_dac.Init(48000, 1);
  audio_dac.Start(&FillBuffer);
  SysTick_Config(F_CPU / 1000);
}

int main(void) {
  Init();
  InitializeReception();
  
  bool exit_updater = !switches.pressed_immediate(Switch(0));
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
              ui_state = UI_STATE_WRITING;
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
        case PACKET_DECODER_STATE_ERROR_CRC:
        case PACKET_DECODER_STATE_ERROR_SYNC:
          error = true;
          break;
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
  audio_dac.Stop();
  Uninitialize();
  JumpTo(kStartAddress);
  while (1) { }
}
