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
//
// -----------------------------------------------------------------------------
//
// Lightweight DAC driver used for the firmware update procedure.
// Initializes the I2S port as SPI, and relies on a timer for clock generation.

#ifndef STAGES_DRIVERS_FIRMWARE_UPDATE_DAC_H_
#define STAGES_DRIVERS_FIRMWARE_UPDATE_DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f37x_conf.h>

#include "stages/io_buffer.h"

namespace stages {

class FirmwareUpdateDac {
 public:
  FirmwareUpdateDac() { }
  ~FirmwareUpdateDac() { }
  
  typedef uint16_t (*NextSampleFn)();
  
  void Init(int sample_rate);
  
  void Start(NextSampleFn next_sample_fn) {
    next_sample_fn_ = next_sample_fn;
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);  
  }
  
  void Stop() {
    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);  
  }
  
  template<int N>
  inline void Wait() {
    Wait<N-1>();
    __asm__("nop");
  }

  void NextSample() {
    GPIOA->BSRR = GPIO_Pin_15;

    uint16_t sample = (*next_sample_fn_)();
    GPIOA->BRR = GPIO_Pin_15;

    sample <<= 4;
    SPI3->DR = 0x0340 | (sample >> 12);
    Wait<64>();
    SPI3->DR = (sample << 4);
  }
  
  static FirmwareUpdateDac* GetInstance() { return instance_; }
  
 private:
  NextSampleFn next_sample_fn_;
  static FirmwareUpdateDac* instance_;
  
  DISALLOW_COPY_AND_ASSIGN(FirmwareUpdateDac);
};

template<> inline void FirmwareUpdateDac::Wait<0>() { }

}  // namespace stages

#endif  // STAGES_DRIVERS_FIRMWARE_UPDATE_DAC_H_
