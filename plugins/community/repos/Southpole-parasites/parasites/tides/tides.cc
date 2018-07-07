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

#include "tides/drivers/adc.h"
#include "tides/drivers/dac.h"
#include "tides/drivers/gate_input.h"
#include "tides/drivers/gate_output.h"
#include "tides/drivers/system.h"
#include "tides/cv_scaler.h"
#include "tides/generator.h"
#include "tides/ui.h"

using namespace tides_parasites;
using namespace stmlib;

Adc adc;
CvScaler cv_scaler;
Dac dac;
GateOutput gate_output;
GateInput gate_input;
Generator generator;
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

// These counters are used to divide the 96kHz DAC update clock into:
// * a 48kHz or 6kHz clock for refreshing the DAC values.
// * a 6kHz clock for reading and smoothing the ADC values (at the exception
//   of the LEVEL CV which is read at the sample rate).
static uint32_t dac_divider = 0;
static uint32_t adc_divider = 0;
static const bool debug_rendering = false;

extern "C" {

void SysTick_Handler() {
  if (ui.mode() == UI_MODE_FACTORY_TESTING) {
    ui.UpdateFactoryTestingFlags(gate_input.Read(), adc.values());
  }
  ui.Poll();
}

static uint32_t saw_counter = 0;

void TIM1_UP_IRQHandler(void) {
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == RESET) {
    return;
  }
  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  
  dac.Update();
  if (ui.mode() == UI_MODE_FACTORY_TESTING) {
    if (dac.ready()) {
      dac.Write(saw_counter >> 16, saw_counter >> 16);
      saw_counter += 8947848;
      gate_output.Write(saw_counter & 0x80000000, saw_counter & 0x80000000);
    }
  } else {
    if (dac.ready()) {
      ++dac_divider;
      if (dac_divider >= generator.clock_divider()) {
        dac_divider = 0;
        const GeneratorSample& sample = generator.Process(gate_input.Read());
        uint32_t uni = sample.unipolar;
        int32_t bi = sample.bipolar;
        uint32_t level = cv_scaler.level();
        if (ui.mode() == UI_MODE_CALIBRATION_C2 ||
            ui.mode() == UI_MODE_CALIBRATION_C4 ||
            ui.mode() == UI_MODE_FACTORY_TESTING) {
          level = 65535;  // Bypass VCA in calibration mode!
        }
        uni = uni * level >> 16;
        bi = -bi * level >> 16;
        dac.Write(uni, bi + 32768);
        if (!debug_rendering) {
          gate_output.Write(
              sample.flags & FLAG_END_OF_ATTACK,
              sample.flags & FLAG_END_OF_RELEASE);
        }
        ui.set_led_color(uni, sample.flags & FLAG_END_OF_ATTACK);
      }
      cv_scaler.ProcessSampleRate(adc.values());
    }
    ++adc_divider;
    if ((adc_divider & 7) == 0) {
      cv_scaler.ProcessControlRate(adc.values());
    }
  }
}

}

void Init() {
  sys.Init(F_CPU / (48000 * 2) - 1, true);
  adc.Init(false);
  cv_scaler.Init();
  dac.Init();
  gate_output.Init();
  gate_input.Init();
  generator.Init();
  ui.Init(&generator, &cv_scaler);
  sys.StartTimers();
}

int main(void) {
  Init();
  while (1) {
    if (generator.writable_block()) {
      if (debug_rendering) {
        gate_output.Write(true, true);
      }
      /* in harmonic mode, the range button sets the waveform, not the pitch */
      if (generator.feature_mode_ == Generator::FEAT_MODE_HARMONIC) {
        generator.set_pitch_high_range(cv_scaler.pitch(), cv_scaler.fm());
      } else {
        generator.set_pitch(cv_scaler.pitch(), cv_scaler.fm());
      }
      if (generator.feature_mode_ == Generator::FEAT_MODE_RANDOM) {
        generator.set_pulse_width(cv_scaler.raw_attenuverter());
      }

      generator.set_shape(cv_scaler.shape());
      generator.set_slope(cv_scaler.slope());
      generator.set_smoothness(cv_scaler.smoothness());
      generator.FillBuffer();
      if (debug_rendering) {
        gate_output.Write(false, false);
      }
    }
    ui.DoEvents();
  }
}
