// Copyright 2015 Olivier Gillet.
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

#include <stm32f4xx_conf.h>

#include "marbles/drivers/clock_inputs.h"
#include "marbles/drivers/dac.h"
#include "marbles/drivers/debug_pin.h"
#include "marbles/drivers/debug_port.h"
#include "marbles/drivers/gate_outputs.h"
#include "marbles/drivers/rng.h"
#include "marbles/drivers/system.h"

#include "marbles/ramp/ramp_extractor.h"
#include "marbles/random/random_generator.h"
#include "marbles/random/random_stream.h"
#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"

#include "marbles/clock_self_patching_detector.h"
#include "marbles/cv_reader.h"
#include "marbles/io_buffer.h"
#include "marbles/note_filter.h"
#include "marbles/resources.h"
#include "marbles/scale_recorder.h"
#include "marbles/settings.h"
#include "marbles/ui.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/hysteresis_quantizer.h"
#include "stmlib/dsp/units.h"

#define PROFILE_INTERRUPT 0
#define PROFILE_RENDER 0

using namespace marbles;
using namespace std;
using namespace stmlib;

const bool test_adc_noise = false;

const int kSampleRate = 32000;
const int kGateDelay = 2;

ClockInputs clock_inputs;
ClockSelfPatchingDetector self_patching_detector[kNumGateOutputs];
CvReader cv_reader;
Dac dac;
DebugPort debug_port;
GateOutputs gate_outputs;
HysteresisQuantizer deja_vu_length_quantizer;
IOBuffer io_buffer;
NoteFilter note_filter;
Rng rng;
ScaleRecorder scale_recorder;
Settings settings;
Ui ui;

RandomGenerator random_generator;
RandomStream random_stream;
TGenerator t_generator;
XYGenerator xy_generator;

// Default interrupt handlers.
extern "C" {

int __errno;

void NMI_Handler() { }
void HardFault_Handler() { while (1); }
void MemManage_Handler() { while (1); }
void BusFault_Handler() { while (1); }
void UsageFault_Handler() { while (1); }
void SVC_Handler() { }
void DebugMon_Handler() { }
void PendSV_Handler() { }

void SysTick_Handler() {
  IWDG_ReloadCounter();
  ui.Poll();
  if (settings.freshly_baked()) {
    if (debug_port.readable()) {
      uint8_t command = debug_port.Read();
      uint8_t response = ui.HandleFactoryTestingRequest(command);
      debug_port.Write(response);
    }
  }
}

}

IOBuffer::Slice FillBuffer(size_t size) {
  if (PROFILE_INTERRUPT) {
    TIC;
  }
  IOBuffer::Slice s = io_buffer.NextSlice(size);
  
  gate_outputs.Write(s);
  clock_inputs.Read(s, size);

  if (io_buffer.new_block()) {
    cv_reader.Copy(&s.block->adc_value[0]);
    clock_inputs.ReadNormalization(s.block);
  }

  if (rng.readable()) {
    random_stream.Write(rng.data());
  }

  if (PROFILE_INTERRUPT) {
    TOC;
  }
  
  return s;
}

inline uint16_t DacCode(int index, float voltage) {
  CONSTRAIN(voltage, -5.0f, 5.0f);
  const float scale = settings.calibration_data().dac_scale[index];
  const float offset = settings.calibration_data().dac_offset[index];
  return ClipU16(static_cast<int32_t>(voltage * scale + offset));
}

void ProcessTest(IOBuffer::Block* block, size_t size) {
  float parameters[kNumParameters];
  static float phase;
  cv_reader.Process(&block->adc_value[0], parameters);
  for (size_t i = 0; i < size; ++i) {
    phase += 100.0f / static_cast<float>(kSampleRate);
    if (phase >= 1.0f) {
      phase -= 1.0f;
    }
    block->cv_output[0][i] = DacCode(
        0, 4.0 * Interpolate(lut_sine, phase, 256.0f));
    block->cv_output[1][i] = DacCode(
        1, -8.0f * phase + 4.0f);
    block->cv_output[2][i] = DacCode(
        2, (phase < 0.5f ? phase : 1.0f - phase) * 16.0f - 4.0f);
    block->cv_output[3][i] = DacCode(
        3, phase < 0.5f ? -4.0f : 4.0f);

    for (int j = 0; j < 4; ++j) {
      uint16_t dac_code = ui.output_test_forced_dac_code(j);
      if (dac_code) {
        block->cv_output[j][i] = dac_code;
      }
    }
    
    block->gate_output[0][i] = block->input_patched[0]
        ? block->input[0][i]
        : phase < 0.2f;
    block->gate_output[1][i] = phase < 0.5f;
    block->gate_output[2][i] = block->input_patched[1]
        ? block->input[1][i]
        : phase < 0.8f;
  }
}

Ratio y_divider_ratios[] = {
  { 1, 64 },
  { 1, 48 },
  { 1, 32 },
  { 1, 24 },
  { 1, 16 },
  { 1, 12 },
  { 1, 8 },
  { 1, 6 },
  { 1, 4 },
  { 1, 3 },
  { 1, 2 },
  { 1, 1 },
};

int loop_length[] = {
  1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7,
  8, 8, 8, 8, 8, 8, 8, 8, 8,
  10, 10, 10,
  12, 12, 12, 12, 12, 12, 12,
  14,
  16
};
float parameters[kNumParameters];
float ramp_buffer[kBlockSize * 4];
bool gates[kBlockSize * 2];
float voltages[kBlockSize * 4];
Ramps ramps;
GroupSettings x, y;
bool gate_delay_tail[kNumGateOutputs][kGateDelay];

float SineOscillator(float voltage) {
  static float phase = 0.0f;
  CONSTRAIN(voltage, -5.0f, 5.0f);
  float frequency = stmlib::SemitonesToRatio(voltage * 12.0f) * 220.0f / kSampleRate;
  phase += frequency;
  if (phase >= 1.0f) {
    phase -= 1.0f;
  }
  return 5.0f * Interpolate(lut_sine, phase, 256.0f);
}

void Process(IOBuffer::Block* block, size_t size) {
  if (PROFILE_RENDER) {
    TIC;
  }

  // Filter CV values (3.5%)
  cv_reader.Process(&block->adc_value[0], parameters);
  
  float deja_vu = parameters[ADC_CHANNEL_DEJA_VU_AMOUNT];
  
  //  Deadband near 12 o'clock for the deja vu parameter.
  const float d = fabsf(deja_vu - 0.5f);
  if (d > 0.03f) {
    ui.set_deja_vu_lock(false);
  } else if (d < 0.02f) {
    ui.set_deja_vu_lock(true);
  }
  if (deja_vu < 0.47f) {
    deja_vu *= 1.06382978723f;
  } else if (deja_vu > 0.53f) {
    deja_vu = 0.5f + (deja_vu - 0.53f) * 1.06382978723f;
  } else {
    deja_vu = 0.5f;
  }
  
  GateFlags* t_clock = block->input[0];
  GateFlags* xy_clock = block->input[1];
  
  // Determine the clock source for the XY section (2%)
  ClockSource xy_clock_source = CLOCK_SOURCE_INTERNAL_T1_T2_T3;
  if (block->input_patched[1]) {
    xy_clock_source = CLOCK_SOURCE_EXTERNAL;
    size_t best_score = 8;
    for (size_t i = 0; i < kNumGateOutputs; ++i) {
      size_t score = self_patching_detector[i].Process(block, size);
      if (score >= best_score) {
        xy_clock_source = ClockSource(CLOCK_SOURCE_INTERNAL_T1 + i);
        best_score = score;
      }
    }
  }

  // Generate gates for T-section (16%).
  ramps.master = &ramp_buffer[0];
  ramps.external = &ramp_buffer[kBlockSize];
  ramps.slave[0] = &ramp_buffer[kBlockSize * 2];
  ramps.slave[1] = &ramp_buffer[kBlockSize * 3];
  
  const State& state = settings.state();
  int deja_vu_length = deja_vu_length_quantizer.Lookup(
      loop_length,
      parameters[ADC_CHANNEL_DEJA_VU_LENGTH],
      sizeof(loop_length) / sizeof(int));
  
  t_generator.set_model(TGeneratorModel(state.t_model));
  t_generator.set_range(TGeneratorRange(state.t_range));
  t_generator.set_rate(parameters[ADC_CHANNEL_T_RATE]);
  t_generator.set_bias(parameters[ADC_CHANNEL_T_BIAS]);
  t_generator.set_jitter(parameters[ADC_CHANNEL_T_JITTER]);
  t_generator.set_deja_vu(state.t_deja_vu ? deja_vu : 0.0f);
  t_generator.set_length(deja_vu_length);
  t_generator.set_pulse_width_mean(float(state.t_pulse_width_mean) / 256.0f);
  t_generator.set_pulse_width_std(float(state.t_pulse_width_std) / 256.0f);
  t_generator.Process(
      block->input_patched[0],
      t_clock,
      ramps,
      gates,
      size);
        
  // Generate voltages for X-section (40%).
  float note_cv_1 = cv_reader.channel(ADC_CHANNEL_X_SPREAD).scaled_raw_cv();
  float note_cv_2 = cv_reader.channel(ADC_CHANNEL_X_SPREAD_2).scaled_raw_cv();
  float note_cv = 0.5f * (note_cv_1 + note_cv_2);
  float u = note_filter.Process(0.5f * (note_cv + 1.0f));
  
  if (test_adc_noise) {
    static float note_lp = 0.0f;
    float note = note_cv_1;
    ONE_POLE(note_lp, note, 0.0001f);
    float cents = (note - note_lp) * 1200.0f * 5.0f;
    fill(&voltages[0], &voltages[4 * size], cents);
  } else if (ui.recording_scale()) {
    float voltage = (u - 0.5f) * 10.0f;
    for (size_t i = 0; i < size; ++i) {
      GateFlags gate = block->input_patched[1]
          ? block->input[1][i]
          : GATE_FLAG_LOW;
      if (gate & GATE_FLAG_RISING) {
        scale_recorder.NewNote(voltage);
      }
      if (gate & GATE_FLAG_HIGH) {
        scale_recorder.UpdateVoltage(voltage);
      }
      if (gate & GATE_FLAG_FALLING) {
        scale_recorder.AcceptNote();
      }
    }
    fill(&voltages[0], &voltages[4 * size], voltage);
  } else {
    x.control_mode = ControlMode(state.x_control_mode);
    x.voltage_range = VoltageRange(state.x_range % 3);
    x.register_mode = state.x_register_mode;
    x.register_value = u;
    cv_reader.set_attenuverter(
        ADC_CHANNEL_X_SPREAD, state.x_register_mode ? 0.5f : 1.0f);
  
    x.spread = parameters[ADC_CHANNEL_X_SPREAD];
    x.bias = parameters[ADC_CHANNEL_X_BIAS];
    x.steps = parameters[ADC_CHANNEL_X_STEPS];
    x.deja_vu = state.x_deja_vu ? deja_vu : 0.0f;
    x.length = deja_vu_length;
    x.ratio.p = 1;
    x.ratio.q = 1;
  
    y.control_mode = CONTROL_MODE_IDENTICAL;
    y.voltage_range = VoltageRange(state.y_range);
    y.register_mode = false;
    y.register_value = 0.0f;
    y.spread = float(state.y_spread) / 256.0f;
    y.bias = float(state.y_bias) / 256.0f;
    y.steps = float(state.y_steps) / 256.0f;
    y.deja_vu = 0.0f;
    y.length = 1;
    y.ratio = y_divider_ratios[
        static_cast<uint16_t>(state.y_divider) * 12 >> 8];
    
    if (settings.dirty_scale_index() != -1) {
      int i = settings.dirty_scale_index();
      xy_generator.LoadScale(i, settings.persistent_data().scale[i]);
      settings.set_dirty_scale_index(-1);
    }
    
    y.scale_index = x.scale_index = state.x_scale;

    xy_generator.Process(
        xy_clock_source,
        x,
        y,
        xy_clock,
        ramps,
        voltages,
        size);
  }
  
  const float* v = voltages;
  const bool* g = gates;
  
  for (size_t i = 0; i < size; ++i) {
    block->cv_output[1][i] = DacCode(1, *v++);
    block->cv_output[2][i] = DacCode(2, *v++);
    block->cv_output[3][i] = DacCode(3, *v++);
    block->cv_output[0][i] = DacCode(0, *v++);
    block->gate_output[0][i + kGateDelay] = *g++;
    block->gate_output[1][i + kGateDelay] = ramps.master[i] < 0.5f;
    block->gate_output[2][i + kGateDelay] = *g++;
  }
  
  for (size_t i = 0; i < kNumGateOutputs; ++i) {
    for (size_t j = 0; j < kGateDelay; ++j) {
      block->gate_output[i][j] = gate_delay_tail[i][j];
      gate_delay_tail[i][j] = block->gate_output[i][size + j];
    }
  }
  
  if (PROFILE_RENDER) {
    TOC;
  }
}

void Init() {
  System sys;
  sys.Init(true);
  settings.Init();
  
  clock_inputs.Init();
  dac.Init(kSampleRate, 1);
  rng.Init();
  note_filter.Init();
  gate_outputs.Init();
  io_buffer.Init();
    
  deja_vu_length_quantizer.Init();
  cv_reader.Init(settings.mutable_calibration_data());
  scale_recorder.Init();
  ui.Init(&settings, &cv_reader, &scale_recorder, &clock_inputs);
  
  if (settings.freshly_baked()) {
    settings.ProgramOptionBytes();
    if (PROFILE_INTERRUPT || PROFILE_RENDER) {
      DebugPin::Init();
    } else {
      debug_port.Init();
    }
  }
  
  random_generator.Init(1);
  random_stream.Init(&random_generator);
  t_generator.Init(&random_stream, static_cast<float>(kSampleRate));
  xy_generator.Init(&random_stream, static_cast<float>(kSampleRate));

  for (size_t i = 0; i < kNumScales; ++i) {
    xy_generator.LoadScale(i, settings.persistent_data().scale[i]);
  }
  
  for (size_t i = 0; i < kNumGateOutputs; ++i) {
    self_patching_detector[i].Init(i);
  }
  
  sys.StartTimers();
  dac.Start(&FillBuffer);
}

int main(void) {
  Init();
  while (1) {
    ui.DoEvents();
    io_buffer.Process(ui.output_test_mode() ? &ProcessTest : &Process);
  }
}
