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

#ifndef ELEMENTS_DRIVERS_CODEC_H_
#define ELEMENTS_DRIVERS_CODEC_H_

#include <stm32f4xx_conf.h>

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

namespace elements {

enum CodecProtocol {
  CODEC_PROTOCOL_PHILIPS = I2S_Standard_Phillips,
  CODEC_PROTOCOL_MSB_FIRST = I2S_Standard_MSB,
  CODEC_PROTOCOL_LSB_FIRST = I2S_Standard_LSB
};

enum CodecFormat {
  CODEC_FORMAT_16_BIT = I2S_DataFormat_16b,
  CODEC_FORMAT_24_BIT = I2S_DataFormat_24b,
  CODEC_FORMAT_32_BIT = I2S_DataFormat_32b
};

enum CodecSettings {
  CODEC_INPUT_0_DB = 0x17,
  CODEC_HEADPHONES_MUTE = 0x00,
  CODEC_MIC_BOOST = 0x1,
  CODEC_MIC_MUTE = 0x2,
  CODEC_ADC_MIC = 0x4,
  CODEC_ADC_LINE = 0x0,
  CODEC_OUTPUT_DAC_ENABLE = 0x10,
  CODEC_OUTPUT_MONITOR = 0x20,
  CODEC_DEEMPHASIS_NONE = 0x00,
  CODEC_DEEMPHASIS_32K = 0x01,
  CODEC_DEEMPHASIS_44K = 0x02,
  CODEC_DEEMPHASIS_48K = 0x03,
  CODEC_SOFT_MUTE = 0x01,
  CODEC_ADC_HPF = 0x00,
  
  CODEC_POWER_DOWN_LINE_IN = 0x01,
  CODEC_POWER_DOWN_MIC = 0x02,
  CODEC_POWER_DOWN_ADC = 0x04,
  CODEC_POWER_DOWN_DAC = 0x08,
  CODEC_POWER_DOWN_LINE_OUT = 0x10,
  CODEC_POWER_DOWN_OSCILLATOR = 0x20,
  CODEC_POWER_DOWN_CLOCK_OUTPUT = 0x40,
  CODEC_POWER_DOWN_EVERYTHING = 0x80,
  
  CODEC_PROTOCOL_MASK_MSB_FIRST = 0x00,
  CODEC_PROTOCOL_MASK_LSB_FIRST = 0x01,
  CODEC_PROTOCOL_MASK_PHILIPS = 0x02,
  CODEC_PROTOCOL_MASK_DSP = 0x03,
  
  CODEC_FORMAT_MASK_16_BIT = 0x00 << 2,
  CODEC_FORMAT_MASK_20_BIT = 0x01 << 2,
  CODEC_FORMAT_MASK_24_BIT = 0x02 << 2,
  CODEC_FORMAT_MASK_32_BIT = 0x03 << 2,
  
  CODEC_FORMAT_LR_SWAP = 0x20,
  CODEC_FORMAT_MASTER = 0x40,
  CODEC_FORMAT_SLAVE = 0x00,
  CODEC_FORMAT_INVERT_CLOCK = 0x80,
  
  CODEC_RATE_48K_48K = 0x00 << 2,
  CODEC_RATE_96K_96K = 0x07 << 2,
  CODEC_RATE_32K_32K = 0x06 << 2,
  CODEC_RATE_44K_44K = 0x18 << 2,
};

// Size of an audio chunk in frames (stereo samples).
// The DMA buffer is 2 chunks long.
const size_t kAudioChunkSize = 16;

// Size of the additional FIFOs in chunks. This gives an additional layer of
// Buffering to make things more robust to jitter (for example if there are
// occasional expensive calls in the UI).
// Use 0 for using naive double-buffering.
const size_t kNumFIFOChunks = 0;

class Codec {
 public:
  Codec() { }
  ~Codec() { }
  
  typedef struct { short l; short r; } Frame;
  typedef void (*FillBufferCallback)(Frame* rx, Frame* tx, size_t size);
  
  bool Init(uint32_t sample_rate, CodecProtocol protocol, CodecFormat format);
  
  bool Start() {
    // No callback - the caller is supposed to poll with available()
    return Start(NULL);
  }
  
  bool Start(FillBufferCallback callback);
  void Stop();
  
  void Fill(size_t offset);

  static Codec* GetInstance() { return instance_; }
  
  // When naive double-buffering is used:
  // 1. Call available() to know if a half-buffer is available.
  // 2. Call Grab(&input_ptr, &output_ptr) to retrieve pointer on the
  //    available rx/tx half-buffers, along with their size.
  inline bool available() const {
    return transmitted_ != processed_;
  }
  
  inline size_t Grab(Frame** input, Frame** output) {
    ++processed_;
    *input = client_rx_;
    *output = client_tx_;
    return kAudioChunkSize;
  }

  // When the extra FIFOs are used:
  // 1. Call writable() and readable() to know how much data can be read
  //    and written from/to the FIFO.
  // 2. Call ImmedidateRead() and Overwrite() to read/write from/to the FIFO.
  size_t writable() const { return tx_buffer_.writable(); }
  size_t readable() const { return rx_buffer_.readable(); }
  void ImmediateRead(Frame* destination, size_t size) {
    rx_buffer_.ImmediateRead(destination, size);
  }
  
  void Overwrite(Frame* source, size_t size) {
    tx_buffer_.Overwrite(source, size);
  }
  
 private:
  bool InitializeGPIO();
  bool InitializeControlInterface();
  bool InitializeAudioInterface(uint32_t, CodecProtocol, CodecFormat);

  bool WriteControlRegister(uint8_t address, uint16_t data);
  bool InitializeCodec(uint32_t, CodecProtocol, CodecFormat);

  bool InitializeDMA();
  
  static Codec* instance_;
  
  bool use_buffering_;
  size_t transmitted_;
  size_t processed_;
  Frame* client_tx_;
  Frame* client_rx_;
  
  FillBufferCallback callback_;
  
  DMA_InitTypeDef dma_init_tx_;
  DMA_InitTypeDef dma_init_rx_;
  
  Frame tx_dma_buffer_[kAudioChunkSize * 2];
  Frame rx_dma_buffer_[kAudioChunkSize * 2];
  stmlib::RingBuffer<Frame, kAudioChunkSize * kNumFIFOChunks> rx_buffer_;
  stmlib::RingBuffer<Frame, kAudioChunkSize * kNumFIFOChunks> tx_buffer_;

  DISALLOW_COPY_AND_ASSIGN(Codec);
};

}  // namespace elements

#endif  // ELEMENTS_DRIVERS_CODEC_H_
