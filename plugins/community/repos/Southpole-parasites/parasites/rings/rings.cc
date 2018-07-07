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

#include "rings/drivers/adc.h"
#include "rings/drivers/codec.h"
#include "rings/drivers/debug_pin.h"
#include "rings/drivers/debug_port.h"
#include "rings/drivers/system.h"
#include "rings/drivers/version.h"
#include "rings/dsp/part.h"
#include "rings/dsp/strummer.h"
#include "rings/dsp/string_synth_part.h"
#include "rings/cv_scaler.h"
#include "rings/settings.h"
#include "rings/ui.h"

// #define PROFILE_INTERRUPT 1

using namespace rings;
using namespace stmlib;

uint16_t reverb_buffer[32768] __attribute__ ((section (".ccmdata")));

Codec codec;
CvScaler cv_scaler;
DebugPort debug_port;
Part part;
Settings settings;
StringSynthPart string_synth;
Strummer strummer;
Ui ui;

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

float in[kMaxBlockSize];
float out[kMaxBlockSize];
float aux[kMaxBlockSize];

const float kNoiseGateThreshold = 0.00003f;
float in_level = 0.0f;

void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t size) {
#ifdef PROFILE_INTERRUPT
  TIC
#endif  // PROFILE_INTERRUPT
  PerformanceState performance_state;
  Patch patch;
  
  cv_scaler.DetectAudioNormalization(input, size);
  cv_scaler.Read(&patch, &performance_state);
  
  if (settings.state().easter_egg) {
    for (size_t i = 0; i < size; ++i) {
      in[i] = static_cast<float>(input[i].r) / 32768.0f;
    }
    strummer.Process(NULL, size, &performance_state);
    string_synth.Process(performance_state, patch, in, out, aux, size);
  } else {
    // Apply noise gate.
    for (size_t i = 0; i < size; ++i) {
      float in_sample = static_cast<float>(input[i].r) / 32768.0f;
      float error, gain;
      error = in_sample * in_sample - in_level;
      in_level += error * (error > 0.0f ? 0.1f : 0.0001f);
      gain = in_level <= kNoiseGateThreshold 
            ? (1.0f / kNoiseGateThreshold) * in_level : 1.0f;
      in[i] = gain * in_sample;
    }
    strummer.Process(in, size, &performance_state);
    part.Process(performance_state, patch, in, out, aux, size);
  }
  
  for (size_t i = 0; i < size; ++i) {
    output[i].l = Clip16(static_cast<int32_t>(out[i] * 32768.0f));
    output[i].r = Clip16(static_cast<int32_t>(aux[i] * 32768.0f));
  }
  ui.set_strumming_flag(performance_state.strum);
#ifdef PROFILE_INTERRUPT
  TOC
#endif  // PROFILE_INTERRUPT
}

void Init() {
  System sys;
  Version version;
  
  sys.Init(true);
  version.Init();

  strummer.Init(0.01f, kSampleRate / kMaxBlockSize);
  part.Init(reverb_buffer);
  string_synth.Init(reverb_buffer);

  settings.Init();
  cv_scaler.Init(settings.mutable_calibration_data());
  ui.Init(&settings, &cv_scaler, &part, &string_synth);
  
  if (!codec.Init(!version.revised(), kSampleRate)) {
    ui.Panic();
  }
  if (!codec.Start(kMaxBlockSize, &FillBuffer)) {
    ui.Panic();
  }
  codec.set_line_input_gain(22);

  if (settings.freshly_baked()) {
#ifdef PROFILE_INTERRUPT
    DebugPin::Init();
#else
    debug_port.Init();
#endif  // PROFILE_INTERRUPT
  }
  sys.StartTimers();
}

int main(void) {
  Init();
  while (1) {
    ui.DoEvents();
  }
}
