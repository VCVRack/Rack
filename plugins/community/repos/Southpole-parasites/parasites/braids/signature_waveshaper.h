// Copyright 2012 Olivier Gillet.
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
// A waveshaper adding waveform impurities, seeded by the serial number of
// the MCU.

#ifndef BRAIDS_SIGNATURE_WAVESHAPER_H_
#define BRAIDS_SIGNATURE_WAVESHAPER_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/dsp.h"

#include "braids/resources.h"

namespace braids {

class SignatureWaveshaper {
 public:
  SignatureWaveshaper() { }
  ~SignatureWaveshaper() { }
  
  inline void Init(uint32_t seed) {
    int32_t skew = seed & 15;
    seed >>= 4;
    
    int32_t sigmoid_strength = seed & 31;
    seed >>= 5;

    int32_t bumplets_frequency = seed & 3;
    seed >>= 2;
    bumplets_frequency += 3;

    int32_t bumplets_width = seed & 7;
    seed >>= 3;
    bumplets_width += 1;
    bumplets_width <<= 7;
    bumplets_width *= bumplets_width;
    
    for (int i = 0; i < 256; ++i) {
      int16_t x = (i - 128) << 8;
      int16_t x_skew = i * i - 32768;
      x = stmlib::Mix(x, x_skew, skew << 11);
      
      int16_t sigmoid = x * (8192 + (sigmoid_strength << 10)) / \
          (8192 + (sigmoid_strength * abs(x) >> 5));
      int16_t bumplets = wav_sine[(i * bumplets_frequency) & 255];
      uint16_t bumplet_gain = x * x / (bumplets_width) + 16;
      bumplet_gain = 32768 * 128 / (128 + bumplet_gain);
      transfer_[i] = stmlib::Mix(sigmoid, bumplets, bumplet_gain);
    }
    transfer_[256] = transfer_[255];
  }
  
  inline int32_t transfer(uint16_t i) {
    return transfer_[i];
  }
  
  inline int32_t Transform(int16_t sample) {
    uint16_t i = sample + 32768;
    int32_t a = transfer_[i >> 8];
    int32_t b = transfer_[(i >> 8) + 1];
    return a + ((b - a) * (i & 0xff) >> 8);
  }

 private:
  int32_t transfer_[257];
   
  DISALLOW_COPY_AND_ASSIGN(SignatureWaveshaper);
};

}  // namespace braids

#endif // BRAIDS_VCO_JITTER_SOURCE_H_
