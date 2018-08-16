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

#include "stmlib/system/system_clock.h"

#include "frames/drivers/dac.h"
#include "frames/drivers/system.h"
#include "frames/drivers/trigger_output.h"
#include "frames/keyframer.h"
#include "frames/poly_lfo.h"
#include "frames/ui.h"

using namespace frames;
using namespace stmlib;

Dac dac;
Keyframer keyframer;
PolyLfo poly_lfo;
System sys;
TriggerOutput trigger_output;
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
  system_clock.Tick();  // Tick global ms counter.
  ui.Poll();
}

volatile uint16_t refresh = 0;
static uint16_t factory_testing_timer = 0;
static int16_t previous_position = -2;
static int16_t previous_nearest_keyframe = -2;
static uint16_t pulse_counter;
static bool can_fire_trigger = false;
static const uint16_t kPulseDuration = 128;

void TIM1_UP_IRQHandler(void) {
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == RESET) {
    return;
  }
  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  
  dac.Update();
  if (dac.ready()) {
    ++refresh;
  }
  
  int16_t position = keyframer.position();
  if (previous_position != position) {
    previous_position = position;
    if (can_fire_trigger) {
      pulse_counter = kPulseDuration;
      trigger_output.High();
      can_fire_trigger = false;
    }
  }
  int16_t nearest_keyframe = keyframer.nearest_keyframe();
  if (previous_nearest_keyframe != nearest_keyframe) {
    previous_nearest_keyframe = nearest_keyframe;
    can_fire_trigger = true;
  }

  if (ui.mode() == UI_MODE_FACTORY_TESTING) {
    ++factory_testing_timer;
    if (factory_testing_timer == 1280) {
      pulse_counter = kPulseDuration;
      trigger_output.High();
      factory_testing_timer = 0;
    }
  }
  
  if (pulse_counter) {
    --pulse_counter;
    if (!pulse_counter) {
      trigger_output.Low();
    }
  }
}

}

void Init() {
  sys.Init(F_CPU / 128000 - 1, true);
  dac.Init();
  trigger_output.Init();
  trigger_output.Low();
  keyframer.Init();
  poly_lfo.Init();
  ui.Init(&keyframer, &poly_lfo);
  sys.StartTimers();
}

int main(void) {
  Init();
  
  while (ui.mode() == UI_MODE_SPLASH) {
    ui.DoEvents();
  }
  
  ui.TryCalibration();
  
  bool trigger_detector_armed = false;
  uint8_t sequencer_step = 0;
  int32_t dc_offset_frame_modulation = keyframer.dc_offset_frame_modulation();

  while (1) {
    ui.DoEvents();
    
    if (refresh) {
      --refresh;
      int32_t frame = ui.frame();
      int32_t frame_modulation = \
          (ui.frame_modulation() - dc_offset_frame_modulation) << 1;
      frame += frame_modulation;
      if (ui.poly_lfo_mode()) {
        poly_lfo.Render(frame);
        dac.Write(0, poly_lfo.dac_code(0));
        dac.Write(1, poly_lfo.dac_code(1));
        dac.Write(2, poly_lfo.dac_code(2));
        dac.Write(3, poly_lfo.dac_code(3));
      } else {
        if (frame < 0) {
          frame = 0;
        } else if (frame > 65535) {
          frame = 65535;
        }

        if (ui.sequencer_mode()) {
          // Detect a trigger on the FRAME input.
          if (frame_modulation < 21845) {
            trigger_detector_armed = true;
          }
          if (frame_modulation > 43690 && trigger_detector_armed) {
            trigger_detector_armed = false;
            ++sequencer_step;
          }
          if (sequencer_step >= keyframer.num_keyframes()) {
            sequencer_step = 0;
          }
          frame = keyframer.keyframe(sequencer_step).timestamp;
        }
        
        keyframer.Evaluate(frame);
        dac.Write(0, keyframer.dac_code(0));
        dac.Write(1, keyframer.dac_code(1));
        dac.Write(2, keyframer.dac_code(2));
        dac.Write(3, keyframer.dac_code(3));
      }
    }
  }
}
