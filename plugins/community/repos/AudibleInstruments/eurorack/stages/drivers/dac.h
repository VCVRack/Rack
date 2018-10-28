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
// Driver for the DAC.

#ifndef STAGES_DRIVERS_DAC_H_
#define STAGES_DRIVERS_DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f37x_conf.h>

#include "stages/io_buffer.h"

namespace stages {

const size_t kMaxDacBlockSize = 8;

class IOBuffer;

const size_t kFrameSize = kNumChannels * 4;
  
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
  static int channel_selection_bits_[kNumChannels];
  
  size_t block_size_;
  FillBufferCallback callback_;
  bool first_frame_;
  
  uint16_t tx_dma_buffer_[2 * kMaxDacBlockSize * kFrameSize];

  DISALLOW_COPY_AND_ASSIGN(Dac);
};

}  // namespace stages

#endif  // STAGES_DRIVERS_DAC_H_
