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

#include "warps/dsp/filter_bank.h"

#include <algorithm>

#include "warps/resources.h"

namespace warps {

using namespace std;
using namespace stmlib;

void FilterBank::Init(float sample_rate) {
  low_src_down_.Init();
  low_src_up_.Init();
  mid_src_down_.Init();
  mid_src_up_.Init();
  
  int32_t max_delay = 0;
  float* samples = &samples_[0];
  
  int32_t group = -1;
  int32_t decimation_factor = -1;
  for (int32_t i = 0; i < kNumBands; ++i) {
    const float* coefficients = filter_bank_table[i];

    Band& b = band_[i];

    b.decimation_factor = static_cast<int32_t>(coefficients[0]);
    
    if (b.decimation_factor != decimation_factor) {
      decimation_factor = b.decimation_factor;
      ++group;
    }
    
    b.group = group;
    b.sample_rate = sample_rate / static_cast<float>(b.decimation_factor);
    b.samples = samples;
    samples += kMaxFilterBankBlockSize / b.decimation_factor;
    
    b.delay = static_cast<int32_t>(coefficients[1]);
    b.delay *= b.decimation_factor;
    b.post_gain = coefficients[2];

    max_delay = max(max_delay, b.delay);
    for (int32_t pass = 0; pass < 2; ++pass) {
      b.svf[pass].Init();
      b.svf[pass].set_f_fq(
          coefficients[pass * 2 + 3],
          coefficients[pass * 2 + 4]);
    }
  }
  band_[kNumBands].group = band_[kNumBands - 1].group + 1;
  max_delay = min(max_delay, int32_t(256));
  float* delay_ptr = &delay_buffer_[0];
  for (int32_t i = 0; i < kNumBands; ++i) {
    Band& b = band_[i];
    int32_t compensation = max_delay - b.delay;
    if (b.group == 0) {
      compensation -= kLowFactor * \
          (low_src_down_.delay() + low_src_up_.delay());
      compensation -= mid_src_down_.delay();
      compensation -= mid_src_up_.delay();
    } else if (b.group == 1) {
      compensation -= mid_src_down_.delay();
      compensation -= mid_src_up_.delay();
    }
    compensation = max(compensation - b.decimation_factor / 2, int32_t(0));
    b.delay_line.Init(delay_ptr, compensation / b.decimation_factor);
    delay_ptr += b.delay_line.size();
  }
}

void FilterBank::Analyze(const float* in, size_t size) {
  mid_src_down_.Process(in, tmp_[0], size);
  low_src_down_.Process(tmp_[0], tmp_[1], size / kMidFactor);
  
  const float* sources[3] = { tmp_[1], tmp_[0], in };
  for (int32_t i = 0; i < kNumBands; ++i) {
    Band& b = band_[i];
    const size_t band_size = size / b.decimation_factor;
    const float* input = sources[b.group];
    
    for (int32_t pass = 0; pass < 2; ++pass) {
      const float* source = pass == 0 ? input : b.samples;
      float* destination = b.samples;
      if (i == 0) {
        b.svf[pass].Process<FILTER_MODE_LOW_PASS>(
            source, destination, band_size);
      } else if (i == kNumBands - 1) {
        b.svf[pass].Process<FILTER_MODE_HIGH_PASS>(
            source, destination, band_size);
      } else {
        b.svf[pass].Process<FILTER_MODE_BAND_PASS_NORMALIZED>(
            source, destination, band_size);
      }
    }
    // Apply post-gain
    const float gain = b.post_gain;
    float* output = b.samples;
    for (size_t i = 0; i < band_size; ++i) {
      output[i] *= gain;
    }
  }
}

void FilterBank::Synthesize(float* out, size_t size) {
  float* buffers[3] = { tmp_[1], tmp_[0], out };

  fill(&buffers[0][0], &buffers[0][size / band_[0].decimation_factor], 0.0f);
  for (int32_t i = 0; i < kNumBands; ++i) {
    Band& b = band_[i];
    
    size_t band_size = size / b.decimation_factor;
    float* s = buffers[b.group];
    for (size_t j = 0; j < band_size; ++j) {
      s[j] += b.delay_line.ReadWrite(b.samples[j]);
    }
    
    if (band_[i + 1].group != b.group) {
      if (b.group == 0) {
        low_src_up_.Process(tmp_[1], tmp_[0], band_size);
      } else if (b.group == 1) {
        mid_src_up_.Process(tmp_[0], out, band_size);
      }
    }
  }
}

}  // namespace warps
