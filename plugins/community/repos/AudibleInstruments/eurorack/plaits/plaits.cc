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

#include "plaits/drivers/audio_dac.h"
#include "plaits/drivers/debug_pin.h"
#include "plaits/drivers/debug_port.h"

#include "plaits/dsp/dsp.h"
#include "plaits/dsp/voice.h"
#include "plaits/settings.h"
#include "plaits/ui.h"

using namespace plaits;
using namespace stmlib;

// #define PROFILE_INTERRUPT 1

const bool test_adc_noise = false;

AudioDac audio_dac;
DebugPort debug_port;
Modulations modulations;
Patch patch;
Settings settings;
Ui ui;
Voice voice;

char shared_buffer[16384];
uint32_t test_ramp;

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
void __cxa_pure_virtual() { while (1); }

}

void FillBuffer(AudioDac::Frame* output, size_t size) {
#ifdef PROFILE_INTERRUPT
  TIC
#endif  // PROFILE_INTERRUPT

  IWDG_ReloadCounter();
  
  ui.Poll();
  
  if (test_adc_noise) {
    static float note_lp = 0.0f;
    float note = modulations.note;
    ONE_POLE(note_lp, note, 0.0001f);
    float cents = (note - note_lp) * 100.0f;
    CONSTRAIN(cents, -8.0f, +8.0f);
    while (size--) {
      output->r = output->l = static_cast<int16_t>(cents * 4040.0f);
      ++output;
    }
  } else if (ui.test_mode()) {
    // 100 Hz ascending and descending ramps.
    while (size--) {
      output->l = ~test_ramp >> 16;
      output->r = test_ramp >> 16;
      test_ramp += 8947848;
      ++output;
    }
  } else {
    voice.Render(patch, modulations, (Voice::Frame*)(output), size);
    ui.set_active_engine(voice.active_engine());
  }
  
  if (debug_port.readable()) {
    uint8_t command = debug_port.Read();
    uint8_t response = ui.HandleFactoryTestingRequest(command);
    debug_port.Write(response);
  }
  
#ifdef PROFILE_INTERRUPT
  TOC
#endif  // PROFILE_INTERRUPT
}

void Init() {
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_16);
  
  BufferAllocator allocator(shared_buffer, 16384);
  voice.Init(&allocator);
  
  volatile size_t counter = 1000000;
  while (counter--);
  
  bool freshly_baked = !settings.Init();

  if (freshly_baked) {
#ifdef PROFILE_INTERRUPT
    DebugPin::Init();
#else
    debug_port.Init();
#endif  // PROFILE_INTERRUPT
  }

  ui.Init(&patch, &modulations, &settings);
  
  audio_dac.Init(48000, kBlockSize);

  audio_dac.Start(&FillBuffer);
  IWDG_Enable();
}

int main(void) {
  Init();
  while (1) { }
}
