// Copyright 2017 Olivier Gillet.
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

#include "stmlib/dsp/dsp.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"
#include "stages/chain_state.h"
#include "stages/drivers/dac.h"
#include "stages/drivers/gate_inputs.h"
#include "stages/drivers/serial_link.h"
#include "stages/drivers/system.h"
#include "stages/cv_reader.h"
#include "stages/factory_test.h"
#include "stages/io_buffer.h"
#include "stages/oscillator.h"
#include "stages/resources.h"
#include "stages/segment_generator.h"
#include "stages/settings.h"
#include "stages/ui.h"

using namespace stages;
using namespace std;
using namespace stmlib;

const bool skip_factory_test = false;
const bool test_adc_noise = false;

ChainState chain_state;
CvReader cv_reader;
Dac dac;
FactoryTest factory_test;
GateFlags no_gate[kBlockSize];
GateInputs gate_inputs;
SegmentGenerator segment_generator[kNumChannels];
Oscillator oscillator[kNumChannels];
IOBuffer io_buffer;
SerialLink left_link;
SerialLink right_link;
Settings settings;
Ui ui;

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

// SysTick and 32kHz handles
extern "C" {

void SysTick_Handler() {
  IWDG_ReloadCounter();
  ui.Poll();
  if (!skip_factory_test) {
    factory_test.Poll();
  }
}

}

IOBuffer::Slice FillBuffer(size_t size) {
  IOBuffer::Slice s = io_buffer.NextSlice(size);
  gate_inputs.Read(s, size);
  if (io_buffer.new_block()) {
    cv_reader.Read(s.block);
    gate_inputs.ReadNormalization(s.block);
  }
  return s;
}

SegmentGenerator::Output out[kBlockSize];

static float note_lp[kNumChannels] = { 0, 0, 0, 0, 0, 0 };

void Process(IOBuffer::Block* block, size_t size) {
  chain_state.Update(
      *block,
      &settings,
      &segment_generator[0],
      out);
  for (size_t channel = 0; channel < kNumChannels; ++channel) {
    bool led_state = segment_generator[channel].Process(
        block->input_patched[channel] ? block->input[channel] : no_gate,
        out,
        size);
    ui.set_slider_led(channel, led_state, 5);
    
    if (test_adc_noise) {
      float note = block->cv_slider[channel];
      ONE_POLE(note_lp[channel], note, 0.0001f);
      float cents = (note - note_lp[channel]) * 1200.0f * 0.5f;
      CONSTRAIN(cents, -1.0f, +1.0f)
      for (size_t i = 0; i < size; ++i) {
        out[i].value = cents;
      }
    }
    
    for (size_t i = 0; i < size; ++i) {
      block->output[channel][i] = settings.dac_code(channel, out[i].value);
    }
  }
}


float ouroboros_ratios[] = {
  0.25f, 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f, 8.0f
};

float this_channel[kBlockSize];
float sum[kBlockSize];
float channel_amplitude[kNumChannels];
float previous_amplitude[kNumChannels];

void ProcessOuroboros(IOBuffer::Block* block, size_t size) {
  const float coarse = (block->cv_slider[0] - 0.5f) * 96.0f;
  const float fine = block->pot[0] * 2.0f - 1.0f;
  const float f0 = SemitonesToRatio(coarse + fine) * 261.6255f / kSampleRate;
  
  std::fill(&sum[0], &sum[size], 0.0f);
  
  for (int channel = kNumChannels - 1; channel >= 0; --channel) {
    
    const float harmonic = block->pot[channel] * 9.999f;
    MAKE_INTEGRAL_FRACTIONAL(harmonic);
    harmonic_fractional = 8.0f * (harmonic_fractional - 0.5f) + 0.5f;
    CONSTRAIN(harmonic_fractional, 0.0f, 1.0f);
    const float ratio = channel == 0 ? 1.0f : Crossfade(
        ouroboros_ratios[harmonic_integral],
        ouroboros_ratios[harmonic_integral + 1],
        harmonic_fractional);
    const float amplitude = channel == 0
        ? 1.0f
        : std::max(block->cv_slider[channel], 0.0f);
    bool trigger = false;
    for (size_t i = 0; i < size; ++i) {
      trigger = trigger || (block->input[channel][i] & GATE_FLAG_RISING);
    }
    if (trigger || !block->input_patched[channel]) {
      channel_amplitude[channel] = 1.0f;
    } else {
      channel_amplitude[channel] *= 0.999f;
    }
    ui.set_slider_led(
        channel, channel_amplitude[channel] * amplitude > 0.00001f, 1);
    const float f = f0 * ratio;
    
    uint8_t waveshape = settings.state().segment_configuration[channel];
    switch (waveshape) {
      case 0:
        oscillator[channel].Render<OSCILLATOR_SHAPE_SINE>(
            f, 0.5f, this_channel, size);
        break;
      case 1:
        oscillator[channel].Render<OSCILLATOR_SHAPE_TRIANGLE>(
            f, 0.5f, this_channel, size);
        break;
      case 2:
      case 3:
        oscillator[channel].Render<OSCILLATOR_SHAPE_SQUARE>(
            f, 0.5f, this_channel, size);
        break;
      case 4:
        oscillator[channel].Render<OSCILLATOR_SHAPE_SAW>(
            f, 0.5f, this_channel, size);
        break;
      case 5:
        oscillator[channel].Render<OSCILLATOR_SHAPE_SQUARE>(
            f, 0.75f, this_channel, size);
        break;
      case 6:
      case 7:
        oscillator[channel].Render<OSCILLATOR_SHAPE_SQUARE>(
            f, 0.9f, this_channel, size);
        break;
    }
    
    ParameterInterpolator am(
        &previous_amplitude[channel],
        amplitude * amplitude * channel_amplitude[channel],
        size);
    for (size_t i = 0; i < size; ++i) {
      sum[i] += this_channel[i] * am.Next();
    }
    
    const float gain = channel == 0 ? 0.2f : 0.66f;
    const float* source = channel == 0 ? sum : this_channel;
    for (size_t i = 0; i < size; ++i) {
      block->output[channel][i] = settings.dac_code(channel, source[i] * gain);
    }
  }
}

void Init() {
  System sys;
  sys.Init(true);
  dac.Init(int(kSampleRate), 2);
  gate_inputs.Init();
  io_buffer.Init();
  
  bool freshly_baked = !settings.Init();
  for (size_t i = 0; i < kNumChannels; ++i) {
    segment_generator[i].Init();
    oscillator[i].Init();
  }
  std::fill(&no_gate[0], &no_gate[kBlockSize], GATE_FLAG_LOW);

  cv_reader.Init(&settings);

  ui.Init(&settings, &chain_state);

  if (freshly_baked && !skip_factory_test) {
    factory_test.Start(&settings, &cv_reader, &gate_inputs, &ui);
    ui.set_factory_test(true);
  } else {
    chain_state.Init(&left_link, &right_link);
  }
  
  sys.StartTimers();
  dac.Start(&FillBuffer);
}

int main(void) {
  Init();
  while (1) {
    io_buffer.Process(factory_test.running()
          ? &FactoryTest::ProcessFn
          : (chain_state.ouroboros() ? &ProcessOuroboros : &Process));
  }
}
