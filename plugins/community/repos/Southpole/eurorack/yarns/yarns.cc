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

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/system_clock.h"
#include "stmlib/system/uid.h"

#include "yarns/drivers/dac.h"
#include "yarns/drivers/gate_output.h"
#include "yarns/drivers/midi_io.h"
#include "yarns/drivers/system.h"
#include "yarns/midi_handler.h"
#include "yarns/multi.h"
#include "yarns/settings.h"
#include "yarns/storage_manager.h"
#include "yarns/ui.h"

using namespace yarns;
using namespace stmlib;

Dac dac;
GateOutput gate_output;
Ui ui;
MidiIO midi_io;
System sys;

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

uint16_t cv[4];
bool gate[4];
bool has_audio_sources;
uint8_t audio_source[4];
uint16_t factory_testing_counter;

void SysTick_Handler() {
  // MIDI I/O, and CV/Gate refresh at 8kHz.
  // UI polling and LED refresh at 1kHz.
  static uint8_t counter;
  if ((++counter & 7) == 0) {
    ui.Poll();
    system_clock.Tick();
  }
  // When there is audio sources, lower the display refresh rate to 8kHz.
  if (has_audio_sources) {
    ui.PollFast();
  }
  
  // Try to read some MIDI input if available.
  if (midi_io.readable()) {
    midi_handler.PushByte(midi_io.ImmediateRead());
  }
  
  // Try to push some MIDI data out.
  if (midi_handler.mutable_high_priority_output_buffer()->readable()) {
    if (midi_io.writable()) {
      midi_io.Overwrite(
          midi_handler.mutable_high_priority_output_buffer()->ImmediateRead());
    }
  }

  if (midi_handler.mutable_output_buffer()->readable()) {
    if (midi_io.writable()) {
      midi_io.Overwrite(midi_handler.mutable_output_buffer()->ImmediateRead());
    }
  }

  // Observe that the gate output is written with a systick (0.125 ms) delay
  // compared to the CV output. This ensures that the CV output will have been
  // refreshed to the right value when the trigger/gate is sent.
  gate_output.Write(gate);
  multi.Refresh();
  multi.GetCvGate(cv, gate);
  has_audio_sources = multi.GetAudioSource(audio_source);
  
  // In calibration mode, overrides the DAC outputs with the raw calibration
  // table values.
  if (ui.calibrating()) {
    const Voice& voice = multi.voice(ui.calibration_voice());
    cv[ui.calibration_voice()] = voice.calibration_dac_code(
        ui.calibration_note());
  } else if (midi_handler.calibrating()) {
    const Voice& voice = multi.voice(midi_handler.calibration_voice());
    cv[midi_handler.calibration_voice()] = voice.calibration_dac_code(
        midi_handler.calibration_note());
  }
  
  // In UI testing mode, overrides the GATE values with timers
  if (ui.factory_testing()) {
    gate[0] = (factory_testing_counter % 800) < 400;
    gate[1] = (factory_testing_counter % 400) < 200;
    gate[2] = (factory_testing_counter % 266) < 133;
    gate[3] = (factory_testing_counter % 200) < 100;
    ++factory_testing_counter;
  }
  
  dac.Write(cv);
}

void TIM1_UP_IRQHandler(void) {
  // DAC refresh at 4x 48kHz.
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == RESET) {
    return;
  }
  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

  dac.Cycle();
  uint8_t source_voice = audio_source[dac.channel()];
  if (source_voice != 0xff) {
    uint16_t audio_sample = multi.mutable_voice(source_voice)->ReadSample();
    dac.Write(audio_sample);
  } else {
    // Use value written there during previous CV refresh.
    dac.Write();
  }
  
  if (dac.channel() == 0) {
    // Internal clock refresh at 48kHz
    multi.RefreshInternalClock();
  } else if (dac.channel() == 1) {
    if (!has_audio_sources) {
      ui.PollFast();
    }
  }
}

}

void Init() {
  sys.Init();
  
  settings.Init();
  multi.Init();
  ui.Init();

  // Load multi 0 on boot.
  storage_manager.LoadMulti(0);
  storage_manager.LoadCalibration();
  
  system_clock.Init();
  gate_output.Init();
  dac.Init();
  midi_io.Init();
  midi_handler.Init();
  sys.StartTimers();
}

int main(void) {
  Init();
  while (1) {
    ui.DoEvents();
    midi_handler.ProcessInput();
    multi.ProcessInternalClockEvents();
    multi.RenderAudio();
    if (midi_handler.factory_testing_requested()) {
      midi_handler.AcknowledgeFactoryTestingRequest();
      ui.StartFactoryTesting();
    }
  }
}
