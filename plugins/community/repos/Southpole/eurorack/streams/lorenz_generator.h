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
// Lorenz system.

#ifndef STREAMS_LORENZ_GENERATOR_H_
#define STREAMS_LORENZ_GENERATOR_H_

#include "stmlib/stmlib.h"

#include "streams/meta_parameters.h"

namespace streams {

class LorenzGenerator {
 public:
  LorenzGenerator() { }
  ~LorenzGenerator() { }
  
  void Init();
  void Process(
      int16_t audio,
      int16_t excite,
      uint16_t* gain,
      uint16_t* frequency);
  
  void set_index(uint8_t index) {
    index_ = index;
  }
  
  void Configure(bool alternate, int32_t* parameters, int32_t* globals) {
    rate_ = parameters[0] >> 8;
    int32_t vcf_amount = 65535 - parameters[1];
    int32_t vca_amount = parameters[1];
    if (vcf_amount >= 32767) vcf_amount = 32767;
    if (vca_amount >= 32767) vca_amount = 32767;
    target_vcf_amount_ = vcf_amount;
    target_vca_amount_ = vca_amount;
  }


 private:
  int32_t x_, y_, z_;
  int32_t rate_;
  int32_t vcf_amount_;
  int32_t vca_amount_;
  int32_t target_vcf_amount_;
  int32_t target_vca_amount_;
  
  uint8_t index_;
  
  DISALLOW_COPY_AND_ASSIGN(LorenzGenerator);
};

}  // namespace streams

#endif  // STREAMS_LORENZ_GENERATOR_H_
