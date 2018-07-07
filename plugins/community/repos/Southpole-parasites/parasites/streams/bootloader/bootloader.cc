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

#include <stm32f10x_conf.h>
#include <string.h>

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/bootloader_utils.h"
#include "stmlib/system/flash_programming.h"
#include "stmlib/system/system_clock.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include "streams/drivers/adc.h"
#include "streams/drivers/switches.h"
#include "streams/drivers/leds.h"
#include "streams/drivers/system.h"

using namespace streams;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;

const uint32_t kStartAddress = 0x08004000;

Adc adc;
System sys;
Leds leds;
Switches switches;
PacketDecoder decoder;
Demodulator demodulator;


const int16_t wav_db[] = {
  -32768, -32768, -24576, -19783,
  -16384, -13746, -11591,  -9770,
   -8192,  -6799,  -5554,  -4428,
   -3399,  -2453,  -1578,   -762,
       0,    716,   1392,   2031,
    2637,   3213,   3763,   4289,
    4792,   5274,   5738,   6184,
    6613,   7028,   7429,   7816,
    8192,   8555,   8908,   9251,
    9584,   9907,  10223,  10530,
   10829,  11121,  11405,  11683,
   11955,  12221,  12481,  12735,
   12984,  13227,  13466,  13700,
   13930,  14155,  14376,  14592,
   14805,  15015,  15220,  15422,
   15621,  15816,  16008,  16197,
   16384,  16567,  16747,  16925,
   17100,  17273,  17443,  17610,
   17776,  17939,  18099,  18258,
   18415,  18569,  18722,  18872,
   19021,  19168,  19313,  19456,
   19597,  19737,  19875,  20012,
   20147,  20281,  20413,  20543,
   20673,  20800,  20927,  21052,
   21176,  21298,  21419,  21539,
   21658,  21776,  21892,  22007,
   22122,  22235,  22347,  22458,
   22568,  22676,  22784,  22891,
   22997,  23102,  23207,  23310,
   23412,  23514,  23614,  23714,
   23813,  23911,  24008,  24105,
   24200,  24295,  24389,  24483,
   24576,  24667,  24759,  24849,
   24939,  25028,  25117,  25205,
   25292,  25379,  25465,  25550,
   25635,  25719,  25802,  25885,
   25968,  26049,  26131,  26211,
   26291,  26371,  26450,  26529,
   26607,  26684,  26761,  26838,
   26914,  26989,  27064,  27139,
   27213,  27286,  27360,  27432,
   27505,  27576,  27648,  27719,
   27789,  27860,  27929,  27999,
   28067,  28136,  28204,  28272,
   28339,  28406,  28473,  28539,
   28605,  28670,  28735,  28800,
   28865,  28929,  28992,  29056,
   29119,  29181,  29244,  29306,
   29368,  29429,  29490,  29551,
   29611,  29671,  29731,  29791,
   29850,  29909,  29968,  30026,
   30084,  30142,  30199,  30257,
   30314,  30370,  30427,  30483,
   30539,  30594,  30650,  30705,
   30760,  30814,  30868,  30923,
   30976,  31030,  31083,  31136,
   31189,  31242,  31294,  31347,
   31399,  31450,  31502,  31553,
   31604,  31655,  31706,  31756,
   31806,  31856,  31906,  31955,
   32005,  32054,  32103,  32152,
   32200,  32248,  32297,  32345,
   32392,  32440,  32487,  32534,
   32581,  32628,  32675,  32721,
   32721,
};

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
int32_t peak = 0;
int32_t gain = 0;

inline void UpdateLeds() {
  leds.Clear();
  
  // Show vu-meter on right side.
  int32_t rectified_sample = adc.cv(0) - 32768;
  rectified_sample = rectified_sample * gain >> 4;
  if (rectified_sample < 0) {
    rectified_sample = -rectified_sample;
  }
  if (rectified_sample > peak) {
    peak = rectified_sample;
  } else {
    peak += (rectified_sample - peak) * 130 >> 15;
  }
  leds.PaintPositiveBar(1, wav_db[peak >> 7] + 8192);
  
  // Show status info on left side (or whole display in case of error).
  switch (ui_state) {
    case UI_STATE_WAITING:
      {
        bool on = system_clock.milliseconds() & 128;
        if (on) {
          for (uint8_t i = 0; i < 4; ++i) {
            leds.set(i, 255, 255);
          }
        }
      }
      break;

    case UI_STATE_RECEIVING:
      {
        uint8_t stage = (system_clock.milliseconds() >> 7) & 3;
        leds.set(stage, 0, 255);
      }
      break;

    case UI_STATE_ERROR:
      {
        bool on = system_clock.milliseconds() & 256;
        for (uint8_t i = 0; i < 4; ++i) {
          leds.set(i, on ? 255 : 0, 0);
        }
      }
      break;

    case UI_STATE_PACKET_OK:
      {
        for (uint8_t i = 0; i < 4; ++i) {
          leds.set(i, 0, 255);
        }
      }
      break;
  }
  leds.Write();
}

void SysTick_Handler() {
  static uint8_t divider = 0;
  
  // SysTick is at 4kHz to get a fast bargraph refresh.
  ++divider;
  if ((divider & 3) == 0) {
    system_clock.Tick();
    system_clock.Tick();  // Tick global ms counter.
    switches.Debounce();
    if (switches.released(0)) {
      switch_released = true;
    }
    adc.ScanPots();
    gain = adc.pot(0) >> 11;
    UpdateLeds();
  }
}

uint16_t discard_samples = 8000;

void TIM2_IRQHandler(void) {
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == RESET) {
    return;
  }
  
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  
  if (!discard_samples) {
    int32_t sample = adc.cv(0);
    sample -= 32768;
    sample = sample * gain >> 4;
    sample = (sample >> 4) + 2048;
    if (sample < 0) {
      sample = 0;
    } else if (sample > 4095) {
      sample = 4095;
    }
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
  sys.Init(false);
  system_clock.Init();
  adc.Init(true, NULL);
  switches.Init();
  leds.Init();
  sys.StartTimers();
  adc.Start();
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
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
