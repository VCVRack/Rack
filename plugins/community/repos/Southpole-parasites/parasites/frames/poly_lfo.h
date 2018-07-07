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
// Poly LFO.

#ifndef FRAMES_POLY_LFO_H_
#define FRAMES_POLY_LFO_H_

#include "stmlib/stmlib.h"

#include "frames/keyframer.h"

namespace frames {

class PolyLfo {
 public:
  PolyLfo() { }
  ~PolyLfo() { }
  
  void Init();
  void Render(int32_t frequency);

  inline void set_shape(uint16_t shape) {
    shape_ = shape;
  }
  inline void set_shape_spread(uint16_t shape_spread) {
    shape_spread_ = static_cast<int16_t>(shape_spread - 32768) >> 1;
  }
  inline void set_spread(uint16_t spread) {
    if (spread < 32768) {
      int32_t x = spread - 32768;
      int32_t scaled = -(x * x >> 15);
      spread_ = (x + 3 * scaled) >> 2;
    } else {
      spread_ = spread - 32768;
    }
  }
  inline void set_coupling(uint16_t coupling) {
    int32_t x = coupling - 32768;
    int32_t scaled = x * x >> 15;
    scaled = x > 0 ? scaled : - scaled;
    scaled = (x + 3 * scaled) >> 2;
    coupling_ = (scaled >> 4) * 10;
    
  }
  inline uint8_t level(uint8_t index) const {
    return level_[index];
  }
  inline const uint8_t* color() const {
    return &color_[0];
  }
  inline const uint16_t dac_code(uint8_t index) const {
    return dac_code_[index];
  }
  static uint32_t FrequencyToPhaseIncrement(int32_t frequency);

  void Reset();
  void Randomize();

 private:
  static const uint8_t rainbow_[17][3];

  uint16_t shape_;
  int16_t shape_spread_;
  int32_t spread_;
  int16_t coupling_;

  int16_t value_[kNumChannels];
  uint32_t phase_[kNumChannels];
  uint8_t level_[kNumChannels];
  uint16_t dac_code_[kNumChannels];
  uint8_t color_[3];

  DISALLOW_COPY_AND_ASSIGN(PolyLfo);
};

}  // namespace frames

#endif  // FRAMES_POLY_LFO_H_
