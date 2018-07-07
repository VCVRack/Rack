// Copyright 2013 Olivier Gillet.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// AVR FSK bootloader - inspired by its STM32F counterpart

#ifndef AVR_AUDIO_BOOTLOADER_FSK_DECODER_H_
#define AVR_AUDIO_BOOTLOADER_FSK_DECODER_H_

#include <avr/pgmspace.h>

namespace avr_audio_bootloader {

const uint16_t kPause = 16;
const uint16_t kOne = 8;
const uint16_t kZero = 4;
const uint16_t kOneZeroThreshold = (kOne + kZero) >> 1;
const uint16_t kPauseOneThreshold = (kPause + kOne) >> 1;

const uint16_t kMaxSyncDuration = 500;  // Symbols
const uint8_t kPreambleSize = 32;
const uint16_t kPacketSize = SPM_PAGESIZE;

enum DecoderState {
  DECODER_STATE_SYNCING,
  DECODER_STATE_DECODING_PACKET,
  DECODER_STATE_PACKET_RECEIVED,
  DECODER_STATE_ERROR_SYNC,
  DECODER_STATE_END_OF_TRANSMISSION
};

class Decoder {
 public:
  Decoder() { }
  ~Decoder() { }
  
  static inline void Init() {
    packet_count_ = 0;
  }
  
  static inline void Sync() {
    previous_sample_ = false;
    duration_ = 0;
    swallow_ = 4;

    state_ = DECODER_STATE_SYNCING;
    sync_blank_size_ = 0;
    expected_symbols_ = 0xff;
    preamble_remaining_size_ = kPreambleSize;
  }

  static inline void set_packet_destination(uint8_t* p) {
    packet_ = p;
  }

  static inline DecoderState PushSample(bool sample) {
    if (previous_sample_ == sample) {
      ++duration_;
    } else {
      previous_sample_ = sample;
      uint8_t symbol = 0;
      
      if (duration_ >= kPauseOneThreshold) {
        symbol = 2;
      } else if (duration_ >= kOneZeroThreshold) {
        symbol = 1;
      } else {
        symbol = 0;
      }
      
      if (swallow_) {
        symbol = 2;
        --swallow_;
      }
      
      PushSymbol(symbol);
      
      duration_ = 0;
    }
    return state_;
  }

  static inline void ParseSyncHeader(uint8_t symbol) {
    if (!((1 << symbol) & expected_symbols_)) {
      state_ = DECODER_STATE_ERROR_SYNC;
      return;
    }
  
    switch (symbol) {
      case 2:
        ++sync_blank_size_;
        if (sync_blank_size_ >= kMaxSyncDuration && packet_count_) {
          state_ = DECODER_STATE_END_OF_TRANSMISSION;
          return;
        }
        expected_symbols_ = (1 << 0) | (1 << 1) | (1 << 2);
        preamble_remaining_size_ = kPreambleSize;
        break;
      
      case 1:
        expected_symbols_ = (1 << 0);
        --preamble_remaining_size_;
        break;
    
      case 0:
        expected_symbols_ = (1 << 1);
        --preamble_remaining_size_;
        break;
    }
  
    if (preamble_remaining_size_ == 0) {
      state_ = DECODER_STATE_DECODING_PACKET;
      packet_size_ = 0;
      packet_[packet_size_] = 0;
      symbol_count_ = 0;
    }
  }

  static void PushSymbol(uint8_t symbol) {
    switch (state_) {
      case DECODER_STATE_SYNCING:
        ParseSyncHeader(symbol);
        break;
        
      case DECODER_STATE_DECODING_PACKET:
        if (symbol == 2) {
          state_ = DECODER_STATE_ERROR_SYNC;
        } else {
          packet_[packet_size_] |= symbol;
          ++symbol_count_;
          if (symbol_count_ == 8) {
            symbol_count_ = 0;
            ++packet_size_;
            if (packet_size_ == kPacketSize + 4) {
              ++packet_count_;
              state_ = DECODER_STATE_PACKET_RECEIVED;
            } else {
              packet_[packet_size_] = 0;
            }
          } else {
            packet_[packet_size_] <<= 1;
          }
        }
        break;

      case DECODER_STATE_PACKET_RECEIVED:
      case DECODER_STATE_ERROR_SYNC:
      case DECODER_STATE_END_OF_TRANSMISSION:
        break;
    }
  }

 private:
  static bool previous_sample_;
  static uint16_t duration_;  
  static uint8_t swallow_;
  static DecoderState state_;
  static uint8_t expected_symbols_;
  static uint8_t preamble_remaining_size_;
  static uint16_t sync_blank_size_;
  static uint8_t symbol_count_;
  static uint8_t* packet_;
  static uint16_t packet_size_;
  static uint16_t packet_count_;
};

}  // namespace avr_audio_bootloader

#endif // AVR_AUDIO_BOOTLOADER_FSK_DECODER_H_
