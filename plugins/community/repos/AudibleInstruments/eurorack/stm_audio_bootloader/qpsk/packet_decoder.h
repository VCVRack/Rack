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
// QPSK decoder for firmware updater.

#ifndef STM_AUDIO_BOOTLOADER_QPSK_PACKET_DECODER_H_
#define STM_AUDIO_BOOTLOADER_QPSK_PACKET_DECODER_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

namespace stm_audio_bootloader {

enum PacketDecoderState {
  PACKET_DECODER_STATE_SYNCING,
  PACKET_DECODER_STATE_DECODING_PACKET,
  PACKET_DECODER_STATE_OK,
  PACKET_DECODER_STATE_ERROR_SYNC,
  PACKET_DECODER_STATE_ERROR_CRC,
  PACKET_DECODER_STATE_END_OF_TRANSMISSION
};

const uint16_t kPacketSize = 256;
const uint16_t kPreambleSize = 16;
const uint8_t kSymbolMask = 0x3;
const uint8_t kSymbolShift = 2;

class PacketDecoder {
 public:
  PacketDecoder() { }
  ~PacketDecoder() { }
  
  void Init() {
    Init(1000);  // 1000 symbols, ie 0.250s at 8000 bps.
  }
  void Init(uint16_t max_sync_duration);
  void Reset() { 
    state_ = PACKET_DECODER_STATE_SYNCING;
    expected_symbols_ = (1 << 4);
    sync_blank_size_ = 0;
  }
  
  PacketDecoderState ProcessSymbol(uint8_t symbol);
  const uint8_t* packet_data() const { return packet_; }
  
 private:
  void ParseSyncHeader(uint8_t symbol);
   
  PacketDecoderState state_;
  uint8_t packet_[kPacketSize + 4];
  uint16_t packet_size_;
  uint16_t symbol_count_;
  uint16_t packet_count_;
  
  uint8_t expected_symbols_;
  uint8_t preamble_remaining_size_;

  uint16_t sync_blank_size_;
  uint16_t max_sync_duration_;
  
  DISALLOW_COPY_AND_ASSIGN(PacketDecoder);
};

}  // namespace stm_audio_bootloader

#endif // STM_AUDIO_BOOTLOADER_QPSK_PACKET_DECODER_H_
