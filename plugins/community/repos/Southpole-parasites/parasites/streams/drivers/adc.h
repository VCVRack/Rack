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
// Driver for ADC.

#ifndef STREAMS_DRIVERS_ADC_H_
#define STREAMS_DRIVERS_ADC_H_

#include "stmlib/stmlib.h"

// #include "streams/cv_scaler.h"

namespace streams {
  
const uint8_t kNumPots = 4;
const uint8_t kNumCVs = 6;

class Adc {
 public:
  typedef void (*CvProcessingCallback)(uint16_t* cv);
  
  Adc() { }
  ~Adc() { }
  
  void Init(bool single_channel, CvProcessingCallback callback);
  void Start();
  void DeInit();
  void ScanPots();
  inline uint8_t last_read_pot() const { return last_read_pot_; }
  inline int32_t pot(uint8_t index) const { return pots_[index]; }
  inline int32_t cv(uint8_t index) const { return cvs_[index]; }
  
  static Adc* GetInstance() { return instance_; }
  void Callback() {
    if (callback_) {
      callback_(cvs_);
    }
  }
  
 private:
  static Adc* instance_;
  static uint8_t pots_sequence_[kNumPots];
  
  CvProcessingCallback callback_;
  uint8_t pot_index_;
  uint8_t last_read_pot_;
  uint16_t pots_[kNumPots];
  uint16_t cvs_[kNumCVs];
  
  DISALLOW_COPY_AND_ASSIGN(Adc);
};

}  // namespace streams

#endif  // STREAMS_DRIVERS_ADC_H_
