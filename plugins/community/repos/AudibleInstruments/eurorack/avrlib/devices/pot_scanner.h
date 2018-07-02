// Copyright 2009 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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
// Two algorithms for getting stable readings from an array of pots connected
// to the ADC:
// - PotScanner uses averaging.
// - HysteresisPotScanner uses a deadband around the latest stable reading.

#ifndef AVRLIB_DEVICES_POT_SCANNER_H_
#define AVRLIB_DEVICES_POT_SCANNER_H_

#include "avrlib/adc.h"
#include "avrlib/log2.h"

namespace avrlib {

template<
    uint8_t num_inputs,
    uint8_t first_input_index = 0,
    uint8_t oversampling = 8,
    uint8_t resolution = 7>
class PotScanner {
 public:
  PotScanner() { }

  static inline void Init() {
    Adc::Init();
    Adc::set_alignment(ADC_LEFT_ALIGNED);
    scan_cycle_ = 0;
    Adc::StartConversion(scan_cycle_ + first_input_index);
    history_ptr_ = 0;
    memset(history_, 0, sizeof(history_));
    memset(value_, 0, sizeof(value_));
  }
  
  static inline void Read() {
    uint8_t index = history_ptr_ + oversampling * scan_cycle_;
    value_[scan_cycle_] -= history_[index];
    Adc::Wait();
    history_[index] = Adc::ReadOut8();
    value_[scan_cycle_] += history_[index];
    
    ++scan_cycle_;
    if (scan_cycle_ == num_inputs) {
      scan_cycle_ = 0;
      ++history_ptr_;
      if (history_ptr_ == oversampling) {
        history_ptr_ = 0;
      }
    }
    
    Adc::StartConversion(scan_cycle_ + first_input_index);
  }
  
  static inline uint16_t value(uint8_t index) {
    uint16_t shift = (Log2<oversampling>::value + 8 - resolution);
    return value_[index] >> shift;
  }
  
  static inline uint8_t last_read() {
    if (scan_cycle_ == 0) {
      return num_inputs - 1;
    } else {
      return (scan_cycle_ - 1);
    }
  }

 private:
  static uint8_t scan_cycle_;
  static uint8_t history_[num_inputs * oversampling];
  static uint16_t value_[num_inputs];
  static uint8_t history_ptr_;

  DISALLOW_COPY_AND_ASSIGN(PotScanner);
};

/* static */
template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
uint8_t PotScanner<a, b, c, d>::scan_cycle_;

/* static */
template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
uint8_t PotScanner<a, b, c, d>::history_ptr_;

/* static */
template<uint8_t num_inputs, uint8_t b, uint8_t c, uint8_t d>
uint16_t PotScanner<num_inputs, b, c, d>::value_[num_inputs];

/* static */
template<uint8_t num_inputs, uint8_t b, uint8_t oversampling, uint8_t d>
uint8_t PotScanner<num_inputs, b, oversampling, d>::history_[
    num_inputs * oversampling];


template<
    uint8_t num_inputs,
    uint8_t first_input_index = 0,
    uint8_t threshold = 8,
    uint8_t resolution = 10>
class HysteresisPotScanner {
 public:
  HysteresisPotScanner() { }

  static inline void Init() {
    Adc::Init();
    scan_cycle_ = 0;
    Adc::StartConversion(scan_cycle_ + first_input_index);
    Lock(threshold);
  }
  
  static void Lock(uint16_t locked_threshold) {
    for (uint8_t i = 0; i < num_inputs; ++i) {
      thresholds_[i] = locked_threshold;
    }
  }
  
  static inline void Read() {
    Adc::Wait();
    uint16_t value = Adc::ReadOut();
    int16_t delta = static_cast<int16_t>(value_[scan_cycle_]) - 
        static_cast<int16_t>(value);
    if (delta < 0) {
      delta = -delta;
    }
    if (delta >= thresholds_[scan_cycle_]) {
      thresholds_[scan_cycle_] = threshold;
      value_[scan_cycle_] = value;
    }
    ++scan_cycle_;
    if (scan_cycle_ == num_inputs) {
      scan_cycle_ = 0;
    }
    Adc::StartConversion(scan_cycle_ + first_input_index);
  }
  
  static inline uint16_t value(uint8_t index) {
    uint16_t shift = (10 - resolution);
    return value_[index] >> shift;
  }
  
  static inline uint8_t last_read() {
    if (scan_cycle_ == 0) {
      return num_inputs - 1;
    } else {
      return (scan_cycle_ - 1);
    }
  }

 private:
  static uint8_t scan_cycle_;
  static uint16_t value_[num_inputs];
  static uint16_t thresholds_[num_inputs];

  DISALLOW_COPY_AND_ASSIGN(HysteresisPotScanner);
};


/* static */
template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
uint8_t HysteresisPotScanner<a, b, c, d>::scan_cycle_;

/* static */
template<uint8_t num_inputs, uint8_t b, uint8_t c, uint8_t d>
uint16_t HysteresisPotScanner<num_inputs, b, c, d>::value_[num_inputs];

/* static */
template<uint8_t num_inputs, uint8_t b, uint8_t c, uint8_t d>
uint16_t HysteresisPotScanner<num_inputs, b, c, d>::thresholds_[num_inputs];


}  // namespace avrlib

#endif  // AVRLIB_DEVICES_POT_SCANNER_H_
