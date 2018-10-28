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
//
// -----------------------------------------------------------------------------
//
// Driver for quad SPI DAC.

#ifndef MARBLES_DRIVERS_DAC_H_
#define MARBLES_DRIVERS_DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

#include "marbles/io_buffer.h"

namespace marbles {

enum DacChannel {
  DAC_CHANNEL_X_1,
  DAC_CHANNEL_X_2,
  DAC_CHANNEL_X_3,
  DAC_CHANNEL_Y,
  DAC_CHANNEL_LAST
};

const size_t kMaxDacBlockSize = 8;
const size_t kNumDacChannels = 4;

class Dac {
 public:
  Dac() { }
  ~Dac() { }
  
  typedef IOBuffer::Slice (*FillBufferCallback)(size_t size);
  
  void Init(int sample_rate, size_t block_size);
  void Start(FillBufferCallback callback);
  void Stop();
  void Fill(size_t offset);
  
  static Dac* GetInstance() { return instance_; }
  
 private:
  void InitializeGPIO();
  void InitializeAudioInterface(int sample_rate);
  void InitializeDMA(size_t block_size);
  static Dac* instance_;
  
  size_t block_size_;
  FillBufferCallback callback_;
  
  // There are 8 16-bit words per frame.
  uint16_t tx_dma_buffer_[2 * kMaxDacBlockSize * kNumDacChannels * 2];

  DISALLOW_COPY_AND_ASSIGN(Dac);
};

}  // namespace marbles

#endif  // MARBLES_DRIVERS_DAC_H_
