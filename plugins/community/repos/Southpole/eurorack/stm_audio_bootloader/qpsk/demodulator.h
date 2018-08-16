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
// QPSK demodulator for firmware updater.

#ifndef STM_AUDIO_BOOTLOADER_DEMODULATOR_H_
#define STM_AUDIO_BOOTLOADER_DEMODULATOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

namespace stm_audio_bootloader {

enum DemodulatorState {
  DEMODULATOR_STATE_ESTIMATE_DC_COMPONENT,
  DEMODULATOR_STATE_ESTIMATE_POWER,
  DEMODULATOR_STATE_CARRIER_SYNC,
  DEMODULATOR_STATE_DECISION_SYNC,
  DEMODULATOR_STATE_OVERFLOW,
  DEMODULATOR_STATE_OK
};

const uint16_t kFilterSize = 7;
const uint16_t kHistorySize = 256;
// Works because kHistorySize is a power of 2!
const uint16_t kHistoryMask = kHistorySize - 1;

class Demodulator {
 public:
  Demodulator() { }
  ~Demodulator() { }
  
  void Init(
      uint32_t phase_increment,
      uint16_t cutoff_reciprocal,
      uint16_t symbol_duration);
  
  inline void PushSample(int16_t sample) {
    if (!samples_.writable()) {
      state_ = DEMODULATOR_STATE_OVERFLOW;
    }
    samples_.Overwrite(sample);
  }
  
  inline void SyncCarrier(bool discover) {
    skipped_samples_ = skipped_symbols_ = 0;
    samples_.Flush();
    symbols_.Flush();
    state_ = discover
        ? DEMODULATOR_STATE_ESTIMATE_DC_COMPONENT
        : DEMODULATOR_STATE_CARRIER_SYNC;
    phase_ = phase_error_ = 0;
  }
  
  inline void SyncDecision() {
    symbols_.Flush();
    state_ = DEMODULATOR_STATE_DECISION_SYNC;
    correlation_history_[0] = correlation_history_[1] = -1024;
    correlation_maximum_ = 0;
    skipped_symbols_ = 0;
  }
  
  inline size_t available() const {
    return symbols_.readable();
  }
  
  DemodulatorState state() const { return state_; }
  
  inline void ProcessAtLeast(size_t size) {
    if (samples_.readable() >= size) {
      while (samples_.readable()) {
        if (state_ == DEMODULATOR_STATE_ESTIMATE_DC_COMPONENT) {
          EstimateDcComponent();
        } else if (state_ == DEMODULATOR_STATE_ESTIMATE_POWER) {
          EstimatePower();
        } else {
          Demodulate();
        }
      }
    }
  }
  
  inline uint8_t NextSymbol() {
    return symbols_.ImmediateRead();
  }
  
  inline int32_t q() const { return q_history_[history_ptr_]; }
  inline int32_t i() const { return i_history_[history_ptr_]; }
  
 private:
  void Reset();
  void EstimateDcComponent();
  void EstimatePower();
  void Demodulate();
  int16_t DecideSymbol(bool adjust_timing);
  
  DemodulatorState state_;
   
  int32_t dc_offset_;
  uint32_t skipped_samples_;
  uint32_t power_;

  // Local oscillator.
  uint32_t phase_;
  uint32_t phase_increment_;
  int32_t phase_error_;
  
  // Demodulator filter.
  int32_t filter_coefficients_[kFilterSize];
  int32_t q_taps_[kFilterSize];
  int32_t i_taps_[kFilterSize];
  
  // History of demodulated signal for matched filter and decision.
  int16_t q_history_[kHistorySize];
  int16_t i_history_[kHistorySize];
  uint16_t history_ptr_;

  // Correlator between the demodulated signal and the synchronization
  // template.
  int32_t correlation_history_[2];
  int32_t correlation_maximum_;
  
  // Decision timing.
  uint16_t symbol_duration_;
  uint16_t decision_counter_;
  uint16_t skipped_symbols_;
  
  stmlib::RingBuffer<int16_t, 1024> samples_;
  stmlib::RingBuffer<uint8_t, 128> symbols_;
  
  DISALLOW_COPY_AND_ASSIGN(Demodulator);
};

}  // namespace stm_audio_bootloader

#endif // STM_AUDIO_BOOTLOADER_DEMODULATOR_H_
