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
//
// -----------------------------------------------------------------------------
//
// FSK demodulator for firmware updater (through gate inputs)

#ifndef STM_AUDIO_BOOTLOADER_FSK_DEMODULATOR_H_
#define STM_AUDIO_BOOTLOADER_FSK_DEMODULATOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

#include <cstdio>

namespace stm_audio_bootloader {

class Demodulator {
 public:
  Demodulator() { }
  ~Demodulator() { }
  
  void Init(uint32_t pause, uint32_t one, uint32_t zero) {
    pause_one_threshold_ = (pause + one) >> 1;
    one_zero_threshold_ = (one + zero) >> 1;
  }
  
  void Sync() {
    symbols_.Init();
    previous_sample_ = false;
    duration_ = 0;
    swallow_ = 4;
  }
  
  void PushSample(bool sample) {
    if (previous_sample_ == sample) {
      ++duration_;
    } else {
      previous_sample_ = sample;
      uint8_t symbol = 0;
      
      if (duration_ >= pause_one_threshold_) {
        symbol = 2;
      } else if (duration_ >= one_zero_threshold_) {
        symbol = 1;
      } else {
        symbol = 0;
      }
      
      if (swallow_) {
        symbol = 2;
        --swallow_;
      }
      
      symbols_.Overwrite(symbol);
      duration_ = 0;
    }
  }
  
  inline size_t available() const {
    return symbols_.readable();
  }
  
  inline uint8_t NextSymbol() {
    return symbols_.ImmediateRead();
  }
  
 private:
  stmlib::RingBuffer<uint8_t, 128> symbols_;
   
  uint32_t pause_one_threshold_;
  uint32_t one_zero_threshold_;
  
  bool previous_sample_;
  uint32_t duration_;
  
  uint8_t swallow_;
  
  DISALLOW_COPY_AND_ASSIGN(Demodulator);
};

}  // namespace stm_audio_bootloader

#endif // STM_AUDIO_BOOTLOADER_FSK_DEMODULATOR_H_
