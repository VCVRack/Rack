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
// Driver for the 4 channels LEDs.

#ifndef STREAMS_DRIVERS_LEDS_H_
#define STREAMS_DRIVERS_LEDS_H_

#include "stmlib/stmlib.h"

namespace streams {

const uint8_t kNumLeds = 8;

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();
  void set(uint8_t led, uint8_t red, uint8_t green) {
    red_[led] = red;
    green_[led] = green;
  }
  void Write();
  void Clear();
  
  void PaintBar(int32_t start, int32_t direction, int32_t db) {
    if (db < 0) {
      return;
    }
    if (db > 32767) {
      db = 32767;
    }
    db <<= 1;
    if (db >= 49152) {
      set(start, (db - 49152) >> 6, 0);
      set(start + direction, 255, 255);
      set(start + 2 * direction, 0, 255);
      set(start + 3 * direction, 0, 255);
    } else if (db >= 32768) {
      set(start + direction, (db - 32768) >> 6, (db - 32768) >> 6);
      set(start + 2 * direction, 0, 255);
      set(start + 3 * direction, 0, 255);
    } else if (db >= 16384) {
      set(start + 2 * direction, 0, (db - 16384) >> 6);
      set(start + 3 * direction, 0, 255);
    } else {
      set(start + 3 * direction, 0, db >> 6);
    }
  }
  
  void PaintPositiveBar(uint8_t channel, int32_t db) {
    PaintBar(channel * 4, +1, db);
  }
  void PaintNegativeBar(uint8_t channel, int32_t db) {
    PaintBar(channel * 4 + 3, -1, -db);
  }

  void PaintCv(uint8_t channel, int32_t cv) {
    uint8_t bank = channel * 4;
    bool flip = false;
    if (cv < 0) {
      flip = true;
      cv = -cv;
    }
    if (cv < 1024) {
      cv = 0;
    }
    if (cv > 32767) {
      cv = 32767;
    }
    for (uint8_t i = 0; i < 4; ++i) {
      int16_t residual = (cv - (3 - i) * 8192);
      if (residual < 0) {
        residual = 0;
      } else if (residual > 8191) {
        residual = 8191;
      }
      if (flip) {
        set(bank + i, residual >> 5, 0);
      } else {
        set(bank + i, 0, residual >> 5);
      }
    }
  }

 private:
  void WriteMcp23s17(uint8_t address, uint8_t data);
  void ShiftOut(uint8_t byte);
   
  uint8_t red_[kNumLeds];
  uint8_t green_[kNumLeds];
  uint8_t pwm_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace streams

#endif  // STREAMS_DRIVERS_LEDS_H_
