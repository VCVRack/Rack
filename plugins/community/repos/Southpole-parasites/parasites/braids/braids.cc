// Copyright 2012 Olivier Gillet.
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

#include <algorithm>

#include "stmlib/utils/dsp.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/system_clock.h"
#include "stmlib/system/uid.h"

#include "braids/drivers/adc.h"
#include "braids/drivers/dac.h"
#include "braids/drivers/debug_pin.h"
#include "braids/drivers/gate_input.h"
#include "braids/drivers/internal_adc.h"
#include "braids/drivers/system.h"
#include "braids/envelope.h"
#include "braids/macro_oscillator.h"
#include "braids/quantizer.h"
#include "braids/signature_waveshaper.h"
#include "braids/vco_jitter_source.h"
#include "braids/ui.h"

#include "braids/quantizer_scales.h"

// #define PROFILE_RENDER 1

using namespace braids;
using namespace std;
using namespace stmlib;


const size_t kNumBlocks = 4;
const size_t kBlockSize = 24;

MacroOscillator osc;
Envelope envelope;
Adc adc;
Dac dac;
DebugPin debug_pin;
GateInput gate_input;
InternalAdc internal_adc;
Quantizer quantizer;
SignatureWaveshaper ws;
System sys;
VcoJitterSource jitter_source;
Ui ui;

uint8_t current_scale = 0xff;
size_t current_sample;
volatile size_t playback_block;
volatile size_t render_block;
int16_t audio_samples[kNumBlocks][kBlockSize];
uint8_t sync_samples[kNumBlocks][kBlockSize];

bool trigger_detected_flag;
volatile bool trigger_flag;
uint16_t trigger_delay;

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

void SysTick_Handler() {
  ui.Poll();
}

void TIM1_UP_IRQHandler(void) {
  if (!(TIM1->SR & TIM_IT_Update)) {
    return;
  }
  TIM1->SR = (uint16_t)~TIM_IT_Update;
  
  dac.Write(-audio_samples[playback_block][current_sample] + 32768);

  bool trigger_detected = gate_input.raised();
  sync_samples[playback_block][current_sample] = trigger_detected;
  trigger_detected_flag = trigger_detected_flag | trigger_detected;
  
  current_sample = current_sample + 1;
  if (current_sample >= kBlockSize) {
    current_sample = 0;
    playback_block = (playback_block + 1) % kNumBlocks;
  }
  
  bool adc_scan_cycle_complete = adc.PipelinedScan();
  if (adc_scan_cycle_complete) {
    ui.UpdateCv(adc.channel(0), adc.channel(1), adc.channel(2), adc.channel(3));
    if (trigger_detected_flag) {
      trigger_delay = settings.trig_delay()
          ? (1 << settings.trig_delay()) : 0;
      ++trigger_delay;
      trigger_detected_flag = false;
    }
    if (trigger_delay) {
      --trigger_delay;
      if (trigger_delay == 0) {
        trigger_flag = true;
      }
    }
  }
}

}

void Init() {
  sys.Init(F_CPU / 96000 - 1, true);
  settings.Init();
  ui.Init();
  system_clock.Init();
  adc.Init(false);
  gate_input.Init();
#ifdef PROFILE_RENDER
  debug_pin.Init();
#endif
  dac.Init();
  osc.Init();
  quantizer.Init();
  internal_adc.Init();
  
  for (size_t i = 0; i < kNumBlocks; ++i) {
    fill(&audio_samples[i][0], &audio_samples[i][kBlockSize], 0);
    fill(&sync_samples[i][0], &sync_samples[i][kBlockSize], 0);
  }
  playback_block = kNumBlocks / 2;
  render_block = 0;
  current_sample = 0;
  
  envelope.Init();
  ws.Init(GetUniqueId(1));
  jitter_source.Init();
  sys.StartTimers();
}

const uint16_t bit_reduction_masks[] = {
    0xc000,
    0xe000,
    0xf000,
    0xf800,
    0xff00,
    0xfff0,
    0xffff };

const uint16_t decimation_factors[] = { 24, 12, 6, 4, 3, 2, 1 };

void RenderBlock() {
  static int16_t previous_pitch = 0;
  static int16_t previous_shape = 0;
  static uint16_t gain_lp;

#ifdef PROFILE_RENDER
  debug_pin.High();
#endif
  envelope.Update(
      settings.GetValue(SETTING_AD_ATTACK) * 8,
      settings.GetValue(SETTING_AD_DECAY) * 8);
  uint32_t ad_value = envelope.Render();
  
  if (ui.paques()) {
    osc.set_shape(MACRO_OSC_SHAPE_QUESTION_MARK);
  } else if (settings.meta_modulation()) {
    int16_t shape = adc.channel(3);
    shape -= settings.data().fm_cv_offset;
    if (shape > previous_shape + 2 || shape < previous_shape - 2) {
      previous_shape = shape;
    } else {
      shape = previous_shape;
    }
    shape = MACRO_OSC_SHAPE_LAST * shape >> 11;
    shape += settings.shape();
    if (shape >= MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META) {
      shape = MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    } else if (shape <= 0) {
      shape = 0;
    }
    MacroOscillatorShape osc_shape = static_cast<MacroOscillatorShape>(shape);
    osc.set_shape(osc_shape);
    ui.set_meta_shape(osc_shape);
  } else {
    osc.set_shape(settings.shape());
  }
  
  // Set timbre and color: CV + internal modulation.
  uint16_t parameters[2];
  for (uint16_t i = 0; i < 2; ++i) {
    uint16_t value = adc.channel(i) << 3;
    Setting ad_mod_setting = i == 0 ? SETTING_AD_TIMBRE : SETTING_AD_COLOR;
    value += ad_value * settings.GetValue(ad_mod_setting) >> 5;
    if (value > 32767) value = 32767;
    parameters[i] = value;
  }
  osc.set_parameters(parameters[0], parameters[1]);
  
  // Apply hysteresis to ADC reading to prevent a single bit error to move
  // the quantized pitch up and down the quantization boundary.
  int32_t pitch = quantizer.Process(
      settings.adc_to_pitch(adc.channel(2)),
      (60 + settings.quantizer_root()) << 7);
  if (!settings.meta_modulation()) {
    pitch += settings.adc_to_fm(adc.channel(3));
  }
  // Check if the pitch has changed to cause an auto-retrigger
  int32_t pitch_delta = pitch - previous_pitch;
  if (settings.data().auto_trig &&
      (pitch_delta >= 0x40 || -pitch_delta >= 0x40)) {
    trigger_detected_flag = true;
  }
  previous_pitch = pitch;
  
  pitch += jitter_source.Render(settings.vco_drift());
  pitch += internal_adc.value() >> 8;
  pitch += ad_value * settings.GetValue(SETTING_AD_FM) >> 7;
  
  if (pitch > 16383) {
    pitch = 16383;
  } else if (pitch < 0) {
    pitch = 0;
  }
  
  if (settings.vco_flatten()) {
    pitch = Interpolate88(lut_vco_detune, pitch << 2);
  }
  osc.set_pitch(pitch + settings.pitch_transposition());

  if (trigger_flag) {
    osc.Strike();
    envelope.Trigger(ENV_SEGMENT_ATTACK);
    ui.StepMarquee();
    trigger_flag = false;
  }
  
  uint8_t* sync_buffer = sync_samples[render_block];
  int16_t* render_buffer = audio_samples[render_block];
  
  if (settings.GetValue(SETTING_AD_VCA) != 0
    || settings.GetValue(SETTING_AD_TIMBRE) != 0
    || settings.GetValue(SETTING_AD_COLOR) != 0
    || settings.GetValue(SETTING_AD_FM) != 0) {
    memset(sync_buffer, 0, kBlockSize);
  }
  
  osc.Render(sync_buffer, render_buffer, kBlockSize);
  
  // Copy to DAC buffer with sample rate and bit reduction applied.
  int16_t sample = 0;
  size_t decimation_factor = decimation_factors[settings.data().sample_rate];
  uint16_t bit_mask = bit_reduction_masks[settings.data().resolution];
  int32_t gain = settings.GetValue(SETTING_AD_VCA) ? ad_value : 65535;
  uint16_t signature = settings.signature() * settings.signature() * 4095;
  for (size_t i = 0; i < kBlockSize; ++i) {
    if ((i % decimation_factor) == 0) {
      sample = render_buffer[i] & bit_mask;
    }
    sample = sample * gain_lp >> 16;
    gain_lp += (gain - gain_lp) >> 4;
    int16_t warped = ws.Transform(sample);
    render_buffer[i] = Mix(sample, warped, signature);
  }
  render_block = (render_block + 1) % kNumBlocks;
#ifdef PROFILE_RENDER
  debug_pin.Low();
#endif
}

int main(void) {
  Init();
  while (1) {
    if (current_scale != settings.GetValue(SETTING_QUANTIZER_SCALE)) {
      current_scale = settings.GetValue(SETTING_QUANTIZER_SCALE);
      quantizer.Configure(scales[current_scale]);
    }
    while (render_block != playback_block) {
      RenderBlock();
    }
    ui.DoEvents();
  }
}
