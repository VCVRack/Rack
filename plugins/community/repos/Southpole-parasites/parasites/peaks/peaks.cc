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

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/system_clock.h"

#include "peaks/drivers/dac.h"
#include "peaks/drivers/gate_input.h"
#include "peaks/drivers/system.h"

#include "peaks/processors.h"
#include "peaks/ui.h"

using namespace peaks;
using namespace stmlib;

Adc adc;
Dac dac;
GateInput gate_input;
Leds leds;
Switches switches;
System sys;
Ui ui;

extern "C" {
  
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void NMI_Handler(void) { }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }
void __cxa_pure_virtual() { while (1); }

}

extern "C" {

uint16_t counter = 0;

void SysTick_Handler() {
  ui.Poll();
}

void TIM1_UP_IRQHandler(void) {
  static int16_t cv_1;
  static int16_t cv_2;
  static uint8_t control;
  
  // DAC refresh at 48kHz
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == RESET) {
    return;
  }

  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  if (dac.ready()) {
    dac.Write(32767 - cv_1, 32767 - cv_2);
    ui.set_leds_brightness(cv_1, cv_2);
    
    control = gate_input.Read() | ui.ReadPanelGateState();
    cv_1 = processors[0].Process(control);
  } else {
    cv_2 = processors[1].Process(control >> 4);
  }
  dac.Update();
}

}

void Init() {
  sys.Init(F_CPU / 96000 - 1, true);
  
  system_clock.Init();
  gate_input.Init();
  dac.Init();
  
  processors[0].Init(0);
  processors[1].Init(1);
  ui.Init();
  
  sys.StartTimers();
}

int main(void) {
  Init();
  while (1) {
    // Faster rate than 1kHz, but no need to be faster than the buffer
    // fill rate.
    ui.PollPots(); 
    ui.DoEvents();
    processors[0].Buffer();
    processors[1].Buffer();
  }
}
