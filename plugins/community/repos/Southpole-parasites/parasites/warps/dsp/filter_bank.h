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
// Filter bank.

#ifndef WARPS_DSP_FILTER_BANK_H_
#define WARPS_DSP_FILTER_BANK_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

#include "warps/dsp/sample_rate_converter.h"
#include "warps/resources.h"

namespace warps {

const int32_t kNumBands = 20;
const int32_t kLowFactor = 4;
const int32_t kMidFactor = 3;
const int32_t kDelayLineSize = 6144;
const int32_t kMaxFilterBankBlockSize = 96;
const int32_t kSampleMemorySize = kMaxFilterBankBlockSize * kNumBands / 2;

class PooledDelayLine {
 public:
  PooledDelayLine() { }
  ~PooledDelayLine() { }
  
  void Init(float* ptr, int32_t delay) {
    delay_line_ = ptr;
    size_ = delay + 1;
    head_ = 0;
    std::fill(&ptr[0], &ptr[size_], 0.0f);
  }
  
  inline int32_t size() const { return size_; }
  
  float ReadWrite(float value) {
    delay_line_[head_] = value;
    head_ = (head_ + 1) % size_;
    return delay_line_[head_];
  };
  
 private:
  float* delay_line_;
  int32_t size_;
  int32_t head_;
  
  DISALLOW_COPY_AND_ASSIGN(PooledDelayLine);
};

struct Band {
  int32_t group;
  float sample_rate;
  float post_gain;
  stmlib::CrossoverSvf svf[2];
  int32_t decimation_factor;
  float* samples;
  PooledDelayLine delay_line;
  int32_t delay;
};

class FilterBank {
 public:
  FilterBank() { }
  ~FilterBank() { }
  void Init(float sample_rate);
  void Analyze(const float* in, size_t size);
  void Synthesize(float* out, size_t size);
  const Band& band(int32_t index) {
    return band_[index];
  }
  
 private:
  SampleRateConverter<SRC_DOWN, kMidFactor, 36> mid_src_down_;
  SampleRateConverter<SRC_UP, kMidFactor, 36> mid_src_up_;
  SampleRateConverter<SRC_DOWN, kLowFactor, 48> low_src_down_;
  SampleRateConverter<SRC_UP, kLowFactor, 48> low_src_up_;
  
  float tmp_[2][kMaxFilterBankBlockSize];
  float samples_[kSampleMemorySize];
  float delay_buffer_[kDelayLineSize];
  
  Band band_[kNumBands + 1];
  
  DISALLOW_COPY_AND_ASSIGN(FilterBank);
};

}  // namespace warps

#endif  // WARPS_DSP_FILTER_BANK_H_
