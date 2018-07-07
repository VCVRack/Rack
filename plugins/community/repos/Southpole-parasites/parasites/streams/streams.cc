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

#include "streams/drivers/adc.h"
#include "streams/drivers/dac.h"
#include "streams/drivers/pwm.h"
#include "streams/drivers/system.h"

#include "streams/cv_scaler.h"
#include "streams/processor.h"
#include "streams/ui.h"

using namespace streams;
using namespace stmlib;

Adc adc;
CvScaler cv_scaler;
Dac dac;
Processor processor[2];
Pwm pwm;
System sys;
Ui ui;

// Default interrupt handlers.
extern "C" {

void HardFault_Handler() { while (1); }
void MemManage_Handler() { while (1); }
void BusFault_Handler() { while (1); }
void UsageFault_Handler() { while (1); }
void NMI_Handler() { }
void SVC_Handler() { }
void DebugMon_Handler() { }
void PendSV_Handler() { }

}

extern "C" {

void SysTick_Handler() {
  // SysTick is at 4kHz to get a fast bargraph refresh.
  ui.Poll();
}

}

void Process(uint16_t* cv) {
  for (uint8_t i = 0; i < 2; ++i) {
    uint16_t gain = 0;
    uint16_t frequency = 65535;
    processor[i].Process(
        cv_scaler.audio_sample(i),
        cv_scaler.excite_sample(i),
        &gain,
        &frequency);
    if (ui.calibrating()) {
      gain = 0;
    }
    dac.Write(i, cv_scaler.ScaleGain(i, gain));
    pwm.Write(i, 65535 - frequency);
  }
}

void Init() {
  sys.Init(true);
  adc.Init(false, &Process);
  cv_scaler.Init(&adc);
  dac.Init();
  pwm.Init();
  processor[0].Init(0);
  processor[1].Init(1);
  ui.Init(&adc, &cv_scaler, &processor[0]);
  sys.StartTimers();
  adc.Start();
}

int main(void) {
  Init();
  while (1) {
    ui.DoEvents();
  }
}
