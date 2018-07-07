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
// Plain filter.

#ifndef STREAMS_FILTER_CONTROLLER_H_
#define STREAMS_FILTER_CONTROLLER_H_

#include "stmlib/stmlib.h"

namespace streams {

class FilterController {
 public:
  FilterController() { }
  ~FilterController() { }
  
  void Init() {
    frequency_offset_ = 0;
    frequency_amount_ = 0;
  }
  void Process(
      int16_t audio,
      int16_t excite,
      uint16_t* gain,
      uint16_t* frequency) {
    // Smooth frequency amount parameters.
    frequency_amount_ += (target_frequency_amount_ - frequency_amount_) >> 8;
    frequency_offset_ += (target_frequency_offset_ - frequency_offset_) >> 8;
    
    int32_t f;
    f = frequency_offset_ + (excite * frequency_amount_ >> 14);
    if (f < 0) {
      f = 0;
    } else if (f > 65535) {
      f = 65535;
    }
    *gain = 0;
    *frequency = f;
  }

  void Configure(bool alternate, int32_t* parameters, int32_t* globals) {
    int32_t amount = parameters[1];
    amount -= 32768;
    amount = amount * amount >> 15;
    target_frequency_amount_ = parameters[1] < 32768 ? -amount : amount;
    target_frequency_offset_ = parameters[0];
  }

 private:
  int32_t target_frequency_amount_;
  int32_t target_frequency_offset_;
  int32_t frequency_amount_;
  int32_t frequency_offset_;
  
  DISALLOW_COPY_AND_ASSIGN(FilterController);
};

}  // namespace streams

#endif  // STREAMS_ENVELOPE_H_
