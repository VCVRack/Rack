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
#include "peaks/drivers/debug_pin.h"
#include "peaks/drivers/gate_input.h"
#include "peaks/drivers/system.h"

#include "peaks/calibration_data.h"
#include "peaks/io_buffer.h"
#include "peaks/processors.h"
#include "peaks/ui.h"

using namespace peaks;
using namespace std;
using namespace stmlib;

Adc adc;
CalibrationData calibration_data;
Dac dac;
GateInput gate_input;
IOBuffer io_buffer;
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

}

extern "C" {

uint16_t counter = 0;

void SysTick_Handler() {
  ui.Poll();
}

GateFlags gate_flags[2];

void TIM1_UP_IRQHandler(void) {
  // DAC refresh at 48kHz
  if (!(TIM1->SR & TIM_IT_Update)) {
    return;
  }
  TIM1->SR = (uint16_t)~TIM_IT_Update;
  
  bool wrote_both_channels = dac.Update();
  if (!wrote_both_channels) {
    return;
  }
  
  uint32_t external_gate_inputs = gate_input.Read();
  uint32_t buttons = ui.ReadPanelGateState();
  uint32_t gate_inputs = external_gate_inputs | buttons;
  
  IOBuffer::Slice slice = io_buffer.NextSlice(1);
  for (size_t i = 0; i < kNumChannels; ++i) {
    gate_flags[i] = ExtractGateFlags(
        gate_flags[i],
        gate_inputs & (1 << i));
    dac.Write(i, slice.block->output[i][slice.frame_index]);
  }
  
  // A hack to make channel 1 aware of what's going on in channel 2. Used to
  // reset the sequencer.
  slice.block->input[0][slice.frame_index] = gate_flags[0] \
      | (gate_flags[1] << 4) \
      | (buttons & 1 ? GATE_FLAG_FROM_BUTTON : 0);
  
  slice.block->input[1][slice.frame_index] = gate_flags[1] \
      | (buttons & 2 ? GATE_FLAG_FROM_BUTTON : 0);
}

}

int16_t output_buffer[kBlockSize];

void Process(IOBuffer::Block* block, size_t size) {
  // DebugPin::High();
  ui.PollPots();
  for (size_t i = 0; i < kNumChannels; ++i) {
    if (ui.calibrating()) {
      fill(&output_buffer[0], &output_buffer[size], 0);
    } else {
      processors[i].Process(block->input[i], output_buffer, size);
    }
    ui.set_led_brightness(i, output_buffer[0]);
    for (size_t j = 0; j < size; ++j) {
      block->output[i][j] = calibration_data.DacCode(i, output_buffer[j]);
    }
  }
  // DebugPin::Low();
}

void Init() {
  sys.Init(F_CPU / 96000 - 1, true);
  
  system_clock.Init();
  gate_input.Init();
  io_buffer.Init();
  dac.Init();

  calibration_data.Init();
  processors[0].Init(0);
  processors[1].Init(1);
  ui.Init(&calibration_data);
  
  // DebugPin::Init();
  
  sys.StartTimers();
}

int main(void) {
  Init();
  
  while (1) {
    ui.DoEvents();
    io_buffer.Process(&Process);
  }
}
