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

#include "stm_audio_bootloader/qpsk/demodulator.h"

#include <cstring>

#define abs(x) (x < 0 ? -x : x)

namespace stm_audio_bootloader {

const uint16_t kObservationWindow = 4096;
const uint32_t kPowerThreshold = 30;
const uint16_t kNumSkippedZeroSymbols = 32;
const uint16_t kNumLockedTemplateSymbols = 4;

int16_t lut_sine[] = {
         0,    50,   100,   150,   200,   250,   300,   350,   399,
       448,   497,   546,   594,   642,   689,   737,   783,   829,
       875,   920,   965,  1009,  1052,  1095,  1137,  1179,  1219,
      1259,  1299,  1337,  1375,  1412,  1448,  1483,  1517,  1550,
      1583,  1614,  1644,  1674,  1702,  1730,  1756,  1781,  1806,
      1829,  1851,  1872,  1892,  1910,  1928,  1944,  1959,  1973,
      1986,  1998,  2008,  2017,  2025,  2032,  2038,  2042,  2045,
      2047,  2048,  2047,  2045,  2042,  2038,  2032,  2025,  2017,
      2008,  1998,  1986,  1973,  1959,  1944,  1928,  1910,  1892,
      1872,  1851,  1829,  1806,  1781,  1756,  1730,  1702,  1674,
      1644,  1614,  1583,  1550,  1517,  1483,  1448,  1412,  1375,
      1337,  1299,  1259,  1219,  1179,  1137,  1095,  1052,  1009,
       965,   920,   875,   829,   783,   737,   689,   642,   594,
       546,   497,   448,   399,   350,   300,   250,   200,   150,
       100,    50,     0,   -50,  -100,  -150,  -200,  -250,  -300,
      -350,  -399,  -448,  -497,  -546,  -594,  -642,  -689,  -737,
      -783,  -829,  -875,  -920,  -965, -1009, -1052, -1095, -1137,
     -1179, -1219, -1259, -1299, -1337, -1375, -1412, -1448, -1483,
     -1517, -1550, -1583, -1614, -1644, -1674, -1702, -1730, -1756,
     -1781, -1806, -1829, -1851, -1872, -1892, -1910, -1928, -1944,
     -1959, -1973, -1986, -1998, -2008, -2017, -2025, -2032, -2038,
     -2042, -2045, -2047, -2048, -2047, -2045, -2042, -2038, -2032,
     -2025, -2017, -2008, -1998, -1986, -1973, -1959, -1944, -1928,
     -1910, -1892, -1872, -1851, -1829, -1806, -1781, -1756, -1730,
     -1702, -1674, -1644, -1614, -1583, -1550, -1517, -1483, -1448,
     -1412, -1375, -1337, -1299, -1259, -1219, -1179, -1137, -1095,
     -1052, -1009,  -965,  -920,  -875,  -829,  -783,  -737,  -689,
      -642,  -594,  -546,  -497,  -448,  -399,  -350,  -300,  -250,
      -200,  -150,  -100,   -50,     0
};

const int16_t filter_coefficients_3_7[] = {
  -20, 40, -55, 64, 255, 64, -55, 40
};

const int16_t filter_coefficients_4_7[] = {
  -21, -51, 123, 255, 123, -51, -21
};

const int16_t filter_coefficients_6_7[] = {
  -70, 25, 180, 255, 180, 25, -70
};

const int16_t filter_coefficients_8_7[] = {
  -35, 85, 205, 255, 205, 85, -35
};

const int16_t filter_coefficients_12_7[] = {
  45, 149, 227, 255, 227, 149, 45
};

const int16_t filter_coefficients_16_7[] = {
  0, 98, 180, 236, 255, 236, 180, 98
};

const int16_t* const cutoff_table[] = {
  NULL,
  NULL,
  NULL,
  filter_coefficients_3_7,
  filter_coefficients_4_7,
  NULL,
  filter_coefficients_6_7,
  NULL,
  filter_coefficients_8_7,
  NULL,
  NULL,
  NULL,
  filter_coefficients_12_7,
  NULL,
  NULL,
  NULL,
  filter_coefficients_16_7
};

void Demodulator::Init(
    uint32_t phase_increment,
    uint16_t cutoff_reciprocal,
    uint16_t symbol_duration) {
  for (uint16_t i = 0; i < kFilterSize; ++i) {
    filter_coefficients_[i] = cutoff_table[cutoff_reciprocal][i];
  }
  symbol_duration_ = symbol_duration;
  phase_increment_ = phase_increment;
  dc_offset_ = 2048 << 8;

  samples_.Init();
  symbols_.Init();
  
  Reset();
}

void Demodulator::Reset() {
  memset(q_taps_, 0, sizeof(q_taps_));
  memset(i_taps_, 0, sizeof(i_taps_));
  memset(q_history_, 0, sizeof(q_history_));
  memset(i_history_, 0, sizeof(i_history_));
  phase_ = 0;
  phase_error_ = 0;
  decision_counter_ = 0;
  history_ptr_ = 0;
  decision_counter_ = symbol_duration_;
}

int16_t Demodulator::DecideSymbol(bool adjust_timing) {
  int32_t q_acc = 0;
  int32_t i_acc = 0;
  
  //   Summation sets for decision
  //         
  //    Present   --------------------------------------->    Past
  //
  //             =========================================    Early
  //        =========================================         On time
  //   =========================================              Late
  //   =====                                                  Head
  //        =====                                             Head 2
  //                                           =====          Tail
  //                                                =====     Tail 2
  //
  uint16_t now = history_ptr_ + kHistorySize;
  for (uint16_t i = 1; i < symbol_duration_ - 1; ++i) {
    uint16_t ptr = (now - i) & kHistoryMask;
    q_acc += q_history_[ptr];
    i_acc += i_history_[ptr];
  }
  
  if (adjust_timing) {
    uint16_t head_ptr = history_ptr_;
    uint16_t head_2_ptr = (now - 1) & kHistoryMask;
    uint16_t tail_ptr = (now - symbol_duration_ + 2) & kHistoryMask;
    uint16_t tail_2_ptr = (now - symbol_duration_ + 1) & kHistoryMask;

    int32_t q_acc_late = q_history_[head_ptr] + q_acc - q_history_[tail_ptr];
    int32_t i_acc_late = i_history_[head_ptr] + i_acc - q_history_[tail_ptr];
    int32_t q_acc_early = q_acc + q_history_[tail_2_ptr] - q_history_[head_2_ptr];
    int32_t i_acc_early = i_acc + i_history_[tail_2_ptr] - i_history_[head_2_ptr];

    uint32_t late_strength = abs(q_acc_late) + abs(i_acc_late);
    uint32_t on_time_strength = abs(q_acc) + abs(i_acc);
    uint32_t early_strength = abs(q_acc_early) + abs(i_acc_early);
    
    if (late_strength > ((5 * on_time_strength) >> 2)) {
      q_acc = q_acc_late;
      i_acc = i_acc_late;
      ++decision_counter_;
    } else if (early_strength > ((5 * on_time_strength) >> 2)) {
      q_acc = q_acc_early;
      i_acc = i_acc_early;
      --decision_counter_;
    }
  }
  
  return (i_acc < 0 ? 0 : 1) + (q_acc < 0 ? 0 : 2);
}

void Demodulator::EstimateDcComponent() {
  while (samples_.readable()) {
    int32_t sample = samples_.ImmediateRead() - (dc_offset_ >> 8);
    dc_offset_ += sample;
    ++skipped_samples_;
    if (skipped_samples_ >= kObservationWindow) {
      skipped_samples_ = 0;
      power_ = 0;
      state_ = DEMODULATOR_STATE_ESTIMATE_POWER;
      break;
    }
  }
}

void Demodulator::EstimatePower() {
  while (samples_.readable()) {
    int32_t sample = samples_.ImmediateRead() - (dc_offset_ >> 8);
    dc_offset_ += sample;
    
    power_ += sample * sample >> 4;
    
    ++skipped_samples_;
    if (skipped_samples_ >= kObservationWindow) {
      uint32_t detected_power = power_ / kObservationWindow;
      skipped_samples_ = 0;
      power_ = 0;
      if (detected_power >= kPowerThreshold) {
        state_ = DEMODULATOR_STATE_CARRIER_SYNC;
        break;
      }
    }
  }
}

void Demodulator::Demodulate() {
  while (samples_.readable()) {
    // Remove DC offset.
    int32_t sample = samples_.ImmediateRead() - (dc_offset_ >> 8);
    dc_offset_ += sample;
    phase_ += phase_increment_;
    
    // Demodulate.
    int32_t i_osc = lut_sine[((phase_ >> 24) + 64) & 0xff];
    int32_t q_osc = lut_sine[phase_ >> 24];
    i_taps_[0] = (sample * -i_osc) >> 8;
    q_taps_[0] = (sample * q_osc) >> 8;

    int32_t i = 0;
    int32_t q = 0;
    
    // Carrier rejection filter.
    for (uint16_t j = 0; j < kFilterSize; ++j) {
      i += i_taps_[j] * filter_coefficients_[j];
      q += q_taps_[j] * filter_coefficients_[j];
    }
    for (uint16_t j = kFilterSize - 1; j > 0; --j) {
      i_taps_[j] = i_taps_[j - 1];
      q_taps_[j] = q_taps_[j - 1];
    }

    // PLL to lock onto the carrier.
    int16_t i_sign = i < 0 ? -1 : 1;
    int16_t q_sign = q < 0 ? -1 : 1;
    int32_t phase_error = i_sign * q - q_sign * i;
    if (state_ == DEMODULATOR_STATE_CARRIER_SYNC) {
      phase_error = i - q;  // Lock to (-1 -1) during sync phase.
    }
    phase_error_ = (3 * phase_error_ + phase_error) >> 2;
    phase_ += phase_error_ << 3;
    phase_increment_ += (phase_error_ >> 1);

    // Store in history buffer (for detection).
    q_history_[history_ptr_] = q >> 8;
    i_history_[history_ptr_] = i >> 8;
    history_ptr_ = (history_ptr_ + 1) & kHistoryMask;
    
    --decision_counter_;
    if (!decision_counter_) {
      decision_counter_ = symbol_duration_;
      // In carrier sync mode, we just let the PLL stabilize until we
      // consistently decode a string of 0s.
      switch (state_) {
        case DEMODULATOR_STATE_CARRIER_SYNC:
          if (DecideSymbol(false) == 0) {
            ++skipped_symbols_;
            if (skipped_symbols_ == kNumSkippedZeroSymbols) {
              SyncDecision();
            }
          }
          break;

        case DEMODULATOR_STATE_DECISION_SYNC:
          symbols_.Overwrite(4);
          break;
          
        case DEMODULATOR_STATE_OK:
          symbols_.Overwrite(DecideSymbol(true));
          break;
          
        default:
          break;
      } 
    }
    
    if (state_ == DEMODULATOR_STATE_DECISION_SYNC) {
      // Match the demodulated Q/I channels with the shape of the alignment
      // sequence.
      int32_t correlation = 0;
      uint16_t ptr = history_ptr_ + kHistorySize;
      for (uint16_t k = 0; k < 2 * symbol_duration_; ++k) {
        int32_t expected_q = k < symbol_duration_ ? -1 :  1;
        int32_t expected_i = -expected_q;
        correlation += q_history_[(ptr - k) & kHistoryMask] * expected_q;
        correlation += i_history_[(ptr - k) & kHistoryMask] * expected_i;
      }
      if (correlation > correlation_maximum_) {
        correlation_maximum_ = correlation;
      }
      if (correlation < 0) {
        // Reset the peak detector at each valley in the detection function,
        // so that we can detect several consecutive peaks.
        correlation_maximum_ = 0;
      }
      
      // Detect a local maximum in the output of the correlator.
      ptr = history_ptr_ + kHistorySize - (symbol_duration_ >> 1);
      if (correlation_history_[0] > correlation && 
          correlation_history_[0] > correlation_history_[1] &&
          correlation_history_[0] == correlation_maximum_ && 
          q_history_[ptr & kHistoryMask] < 0 &&
          i_history_[ptr & kHistoryMask] > 0) {
        ++skipped_symbols_;
        if (skipped_symbols_ == kNumLockedTemplateSymbols) {
          state_ = DEMODULATOR_STATE_OK;
        }
        decision_counter_ = symbol_duration_ - 1;
      }

      correlation_history_[1] = correlation_history_[0];
      correlation_history_[0] = correlation;
    }
  }
}

}  // namespace stm_audio_bootloader
