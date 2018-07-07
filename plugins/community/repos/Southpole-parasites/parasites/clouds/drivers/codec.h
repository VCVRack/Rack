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
//
// -----------------------------------------------------------------------------
//
// WM8371 Codec support.

#ifndef CLOUDS_DRIVERS_CODEC_H_
#define CLOUDS_DRIVERS_CODEC_H_

#include <stm32f4xx_conf.h>

#include "stmlib/stmlib.h"

namespace clouds {

const size_t kMaxCodecBlockSize = 32;

class Codec {
 public:
  Codec() { }
  ~Codec() { }
  
  typedef struct {
    short l;
    short r;
  } Frame;
  
  typedef void (*FillBufferCallback)(Frame* rx, Frame* tx, size_t size);
  
  bool Init(
      bool mcu_is_master,
      int32_t sample_rate);
  
  bool Start(size_t block_size) {
    // No callback - the caller is supposed to poll with available()
    return Start(block_size, NULL);
  }
  
  bool Start(size_t block_size, FillBufferCallback callback);
  void Stop();
  
  void Fill(size_t offset);
  
  bool set_line_input_gain(int32_t channel, int32_t gain);
  bool set_line_input_gain(int32_t gain);

  static Codec* GetInstance() { return instance_; }
  
 private:
  bool InitializeGPIO();
  bool InitializeControlInterface();
  bool InitializeAudioInterface(bool, int32_t);
  bool InitializeCodec(bool, int32_t);

  bool WriteControlRegister(uint8_t address, uint16_t data);

  bool InitializeDMA();
  
  static Codec* instance_;
  
  bool mcu_is_master_;
  int32_t sample_rate_;
  size_t block_size_;
  size_t stride_;
  
  FillBufferCallback callback_;
  
  DMA_InitTypeDef dma_init_tx_;
  DMA_InitTypeDef dma_init_rx_;
  
  short tx_dma_buffer_[kMaxCodecBlockSize * 6 * 2];
  short rx_dma_buffer_[kMaxCodecBlockSize * 6 * 2];

  DISALLOW_COPY_AND_ASSIGN(Codec);
};

}  // namespace clouds

#endif  // CLOUDS_DRIVERS_CODEC_H_
