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
// Search for stretch/shift splicing points by maximizing correlation.

#include "clouds/dsp/correlator.h"

#include <algorithm>

namespace clouds {

using namespace std;

void Correlator::Init(uint32_t* source, uint32_t* destination) {
  source_ = source;
  destination_ = destination;
  offset_ = 0;
  best_match_ = 0;
  done_ = true;
}

void Correlator::EvaluateNextCandidate() {
  if (done_) {
    return;
  }
  uint32_t num_words = size_ >> 5;
  uint32_t offset_words = candidate_ >> 5;
  uint32_t offset_bits = candidate_ & 0x1f;
  uint32_t* source = &source_[0];
  uint32_t* destination = &destination_[offset_words];
  
  uint32_t xcorr = 0;
  for (uint32_t i = 0; i < num_words; ++i) {
    uint32_t source_bits = source[i];
    uint32_t destination_bits = 0;
    destination_bits |= destination[i] << offset_bits;
    destination_bits |= destination[i + 1] >> (32 - offset_bits);
    uint32_t count = ~(source_bits ^ destination_bits);
    count = count - ((count >> 1) & 0x55555555);
    count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
    count = (((count + (count >> 4)) & 0xf0f0f0f) * 0x1010101) >> 24;
    xcorr += count;
  }
  if (xcorr > best_score_) {
    best_match_ = candidate_;
    best_score_ = xcorr;
  }
  ++candidate_;
  done_ = candidate_ >= size_;
}

void Correlator::StartSearch(
    int32_t size,
    int32_t offset,
    int32_t increment) {
  offset_ = offset;
  increment_ = increment;
  best_score_ = 0;
  best_match_ = 0;
  candidate_ = 0;
  size_ = size;
  done_ = false;
}

}  // namespace clouds
