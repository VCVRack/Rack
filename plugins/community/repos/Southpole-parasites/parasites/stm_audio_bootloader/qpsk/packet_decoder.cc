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

#include "stm_audio_bootloader/qpsk/packet_decoder.h"

#include "stmlib/utils/crc32.h"

namespace stm_audio_bootloader {

void PacketDecoder::Init(uint16_t max_sync_duration) {
  Reset();
  packet_count_ = 0;
  max_sync_duration_ = max_sync_duration;
}

void PacketDecoder::ParseSyncHeader(uint8_t symbol) {
  if (!((1 << symbol) & expected_symbols_)) {
    state_ = PACKET_DECODER_STATE_ERROR_SYNC;
    return;
  }
  
  switch (symbol) {
    case 4:
      ++sync_blank_size_;
      if (sync_blank_size_ >= max_sync_duration_ && packet_count_) {
        state_ = PACKET_DECODER_STATE_END_OF_TRANSMISSION;
        return;
      }
      expected_symbols_ = (1 << 1) | (1 << 2) | (1 << 4);
      preamble_remaining_size_ = kPreambleSize;
      break;
      
    case 3:
      expected_symbols_ = (1 << 0);
      --preamble_remaining_size_;
      break;
      
    case 2:
      expected_symbols_ = (1 << 1);
      break;
      
    case 1:
      expected_symbols_ = (1 << 2) | (1 << 3);
      break;
      
    case 0:
      expected_symbols_ = (1 << 3);
      --preamble_remaining_size_;
      break;
  }
  
  if (preamble_remaining_size_ == 0) {
    state_ = PACKET_DECODER_STATE_DECODING_PACKET;
    packet_size_ = 0;
    packet_[packet_size_] = 0;
    symbol_count_ = 0;
  }
}

PacketDecoderState PacketDecoder::ProcessSymbol(uint8_t symbol) {
  switch (state_) {
    case PACKET_DECODER_STATE_SYNCING:
      ParseSyncHeader(symbol);
      break;
      
    case PACKET_DECODER_STATE_DECODING_PACKET:
      packet_[packet_size_] |= symbol;
      ++symbol_count_;
      if (symbol_count_ == kSymbolMask + 1) {
        symbol_count_ = 0;
        ++packet_size_;
        if (packet_size_ == kPacketSize + 4) {
          uint32_t crc = crc32(0, packet_, kPacketSize);
          uint32_t expected_crc = \
              (static_cast<uint32_t>(packet_[kPacketSize + 0]) << 24) | \
              (static_cast<uint32_t>(packet_[kPacketSize + 1]) << 16) | \
              (static_cast<uint32_t>(packet_[kPacketSize + 2]) <<  8) | \
              (static_cast<uint32_t>(packet_[kPacketSize + 3]) <<  0);
          state_ = crc == expected_crc \
              ? PACKET_DECODER_STATE_OK
              : PACKET_DECODER_STATE_ERROR_CRC;
          ++packet_count_;
        } else {
          packet_[packet_size_] = 0;
        }
      } else {
        packet_[packet_size_] <<= kSymbolShift;
      }
      break;

    case PACKET_DECODER_STATE_OK:
    case PACKET_DECODER_STATE_ERROR_SYNC:
    case PACKET_DECODER_STATE_ERROR_CRC:
    case PACKET_DECODER_STATE_END_OF_TRANSMISSION:
      break;
  }
  return state_;
}

}  // namespace stm_audio_bootloader
