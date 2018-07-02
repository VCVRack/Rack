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

#include "elements/drivers/cv_adc.h"
#include "elements/drivers/codec.h"
#include "elements/drivers/debug_pin.h"
#include "elements/drivers/debug_port.h"
#include "elements/drivers/pots_adc.h"
#include "elements/drivers/system.h"
#include "elements/dsp/part.h"
#include "elements/cv_scaler.h"
#include "elements/ui.h"

// #define PROFILE_INTERRUPT 1

using namespace elements;
using namespace stmlib;

Codec codec;
CvScaler cv_scaler;
DebugPort debug_port;
Part part;
Ui ui;

uint16_t reverb_buffer[32768] __attribute__ ((section (".ccmdata")));

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
void SysTick_Handler() {
  ui.Poll();
  if (debug_port.readable()) {
    uint8_t command = debug_port.Read();
    uint8_t response = ui.HandleFactoryTestingRequest(command);
    debug_port.Write(response);
  }
}

}

float blow_in[kAudioChunkSize];
float strike_in[kAudioChunkSize];
float out[kAudioChunkSize];
float aux[kAudioChunkSize];

const float kNoiseGateThreshold = 0.0001f;
float strike_in_level = 0.0f;
float blow_in_level = 0.0f;

void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t n) {
#ifdef PROFILE_INTERRUPT
  TIC
#endif  // PROFILE_INTERRUPT
  PerformanceState s;
  cv_scaler.Read(part.mutable_patch(), &s);
  s.gate |= ui.gate();
  for (size_t i = 0; i < n; ++i) {
    float blow_in_sample = static_cast<float>(input[i].r) / 32768.0f;
    float strike_in_sample = static_cast<float>(input[i].l) / 32768.0f;

    float error, gain;
    error = strike_in_sample * strike_in_sample - strike_in_level;
    strike_in_level += error * (error > 0.0f ? 0.1f : 0.0001f);
    gain = strike_in_level <= kNoiseGateThreshold 
          ? (1.0f / kNoiseGateThreshold) * strike_in_level : 1.0f;
    strike_in[i] = gain * strike_in_sample;
    
    error = blow_in_sample * blow_in_sample - blow_in_level;
    blow_in_level += error * (error > 0.0f ? 0.1f : 0.0001f);
    gain = blow_in_level <= kNoiseGateThreshold 
          ? (1.0f / kNoiseGateThreshold) * blow_in_level : 1.0f;
    blow_in[i] = gain * blow_in_sample;
  }
  part.Process(s, blow_in, strike_in, out, aux, n);
  for (size_t i = 0; i < n; ++i) {
    output[i].r = SoftConvert(out[i]);
    output[i].l = SoftConvert(aux[i]);
  }
#ifdef PROFILE_INTERRUPT
  TOC
#endif  // PROFILE_INTERRUPT
}

void Init() {
  System sys;
  
  sys.Init(true);

  // Init and seed the random parameters and generators with the serial number.
  part.Init(reverb_buffer);
  part.Seed((uint32_t*)(0x1fff7a10), 3);

  cv_scaler.Init();
  ui.Init(&part, &cv_scaler);
  
  if (!codec.Init(32000, CODEC_PROTOCOL_PHILIPS, CODEC_FORMAT_16_BIT)) {
    ui.Panic();
  }
  if (!codec.Start(&FillBuffer)) {
    ui.Panic();
  }
  
  if (cv_scaler.freshly_baked()) {
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
