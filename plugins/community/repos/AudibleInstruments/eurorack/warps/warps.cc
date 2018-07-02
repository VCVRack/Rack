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

#include "warps/cv_scaler.h"
#include "warps/drivers/codec.h"
#include "warps/drivers/debug_pin.h"
#include "warps/drivers/debug_port.h"
#include "warps/drivers/system.h"
#include "warps/drivers/version.h"
#include "warps/dsp/modulator.h"
#include "warps/settings.h"
#include "warps/ui.h"

// #define PROFILE_INTERRUPT 1

using namespace warps;
using namespace stmlib;

Codec codec;
CvScaler cv_scaler;
DebugPort debug_port;
Modulator modulator;
Settings settings;
Ui ui;

int __errno;

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

void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t n) {
#ifdef PROFILE_INTERRUPT
  TIC
#endif  // PROFILE_INTERRUPT
  cv_scaler.DetectAudioNormalization(input, n);  // 0.8% CPU
  cv_scaler.Read(modulator.mutable_parameters());
  modulator.Process((ShortFrame*)input, (ShortFrame*)output, n);
  ui.Poll();
  if (settings.freshly_baked()) {
    if (debug_port.readable()) {
      uint8_t command = debug_port.ImmediateRead();
      uint8_t response = ui.HandleFactoryTestingRequest(command);
      debug_port.Overwrite(response);
    }
  }
#ifdef PROFILE_INTERRUPT
  TOC
#endif  // PROFILE_INTERRUPT
}

void Init() {
  System sys;
  Version version;

  sys.Init(true);
  version.Init();

  // Init modulator.
  modulator.Init(96000.0f);
  settings.Init();
  cv_scaler.Init(settings.mutable_calibration_data());
  
  ui.Init(&settings, &cv_scaler, &modulator);
  
  if (!codec.Init(!version.revised(), 96000)) {
    ui.Panic();
  }
  if (!codec.Start(60, &FillBuffer)) {
    ui.Panic();
  }
  codec.set_line_input_gain(24);  // Max input level: 16Vpp.
  
  if (settings.freshly_baked()) {
#ifdef PROFILE_INTERRUPT
    DebugPin::Init();
#else
    debug_port.Init();
#endif  // PROFILE_INTERRUPT
  }
}

int main(void) {
  Init();
  while (1) {
    ui.DoEvents();
  }
}
