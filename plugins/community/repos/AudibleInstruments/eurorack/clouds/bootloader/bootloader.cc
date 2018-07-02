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

#include "clouds/drivers/codec.h"
#include "clouds/drivers/leds.h"
#include "clouds/drivers/switches.h"
#include "clouds/drivers/system.h"
#include "clouds/drivers/version.h"
#include "clouds/meter.h"

#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"

#include <cstring>

using namespace clouds;
using namespace stmlib;
using namespace stm_audio_bootloader;

const double kSampleRate = 48000.0;
const double kModulationRate = 6000.0;
const double kBitRate = 12000.0;
const uint32_t kStartAddress = 0x08008000;

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

const int16_t lut_db[] = {
  -32768, -32768, -24576, -19783, -16384, -13746, -11591,  -9770,
   -8192,  -6799,  -5554,  -4428,  -3399,  -2453,  -1578,   -762,
       0,    716,   1392,   2031,   2637,   3213,   3763,   4289,
    4792,   5274,   5738,   6184,   6613,   7028,   7429,   7816,
    8192,   8555,   8908,   9251,   9584,   9907,  10223,  10530,
   10829,  11121,  11405,  11683,  11955,  12221,  12481,  12735,
   12984,  13227,  13466,  13700,  13930,  14155,  14376,  14592,
   14805,  15015,  15220,  15422,  15621,  15816,  16008,  16197,
   16384,  16567,  16747,  16925,  17100,  17273,  17443,  17610,
   17776,  17939,  18099,  18258,  18415,  18569,  18722,  18872,
   19021,  19168,  19313,  19456,  19597,  19737,  19875,  20012,
   20147,  20281,  20413,  20543,  20673,  20800,  20927,  21052,
   21176,  21298,  21419,  21539,  21658,  21776,  21892,  22007,
   22122,  22235,  22347,  22458,  22568,  22676,  22784,  22891,
   22997,  23102,  23207,  23310,  23412,  23514,  23614,  23714,
   23813,  23911,  24008,  24105,  24200,  24295,  24389,  24483,
   24576,  24667,  24759,  24849,  24939,  25028,  25117,  25205,
   25292,  25379,  25465,  25550,  25635,  25719,  25802,  25885,
   25968,  26049,  26131,  26211,  26291,  26371,  26450,  26529,
   26607,  26684,  26761,  26838,  26914,  26989,  27064,  27139,
   27213,  27286,  27360,  27432,  27505,  27576,  27648,  27719,
   27789,  27860,  27929,  27999,  28067,  28136,  28204,  28272,
   28339,  28406,  28473,  28539,  28605,  28670,  28735,  28800,
   28865,  28929,  28992,  29056,  29119,  29181,  29244,  29306,
   29368,  29429,  29490,  29551,  29611,  29671,  29731,  29791,
   29850,  29909,  29968,  30026,  30084,  30142,  30199,  30257,
   30314,  30370,  30427,  30483,  30539,  30594,  30650,  30705,
   30760,  30814,  30868,  30923,  30976,  31030,  31083,  31136,
   31189,  31242,  31294,  31347,  31399,  31450,  31502,  31553,
   31604,  31655,  31706,  31756,  31806,  31856,  31906,  31955,
   32005,  32054,  32103,  32152,  32200,  32248,  32297,  32345,
   32392,  32440,  32487,  32534,  32581,  32628,  32675,  32721,
   32721,
};

enum UiState {
  UI_STATE_WAITING,
  UI_STATE_RECEIVING,
  UI_STATE_ERROR,
  UI_STATE_WRITING
};

volatile bool switch_released = false;
volatile UiState ui_state;

void UpdateLeds() {
  leds.Clear();
  leds.set_freeze(true);
  switch (ui_state) {
    case UI_STATE_WAITING:
      leds.set_freeze(system_clock.milliseconds() & 128);
      break;

    case UI_STATE_RECEIVING:
      leds.set_freeze(system_clock.milliseconds() & 32);
      leds.PaintBar(lut_db[meter.peak() >> 7]);
      break;
    
    case UI_STATE_ERROR:
      {
        bool on = system_clock.milliseconds() & 256;
        for (uint8_t i = 0; i < 4; ++i) {
          leds.set_status(i, on ? 255 : 0, 0);
        }
      }
      break;
      
    case UI_STATE_WRITING:
      {
        for (uint8_t i = 0; i < 4; ++i) {
          leds.set_status(i, 0, 255);
        }
      }
      break;
  }
  leds.Write();
}

void SysTick_Handler() {
  system_clock.Tick();
  switches.Debounce();
  if (switches.released(2)) {
    switch_released = true;
  }
  UpdateLeds();
}

}

size_t discard_samples = 8000;
void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t n) {
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
  switches.Init();
  version.Init();
  if (!codec.Init(!version.revised(), 48000)) { }
  if (!codec.Start(32, &FillBuffer)) { }
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

  bool exit_updater = !switches.pressed_immediate(2);
  
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
  Uninitialize();
  JumpTo(kStartAddress);
  while (1) { }
}
