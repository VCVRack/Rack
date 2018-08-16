// Copyright 2011 Olivier Gillet.
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
// Fast 16-bit pseudo random number generator.

#ifndef AVRLIBX_UTILS_RANDOM_H_
#define AVRLIBX_UTILS_RANDOM_H_

#include "avrlibx/avrlibx.h"

namespace avrlibx {

class Random {
 public:
  static void Update() {
    // Galois LFSR with feedback polynomial = x^16 + x^14 + x^13 + x^11.
    // Period: 65535.
    rng_state_ = (rng_state_ >> 1) ^ (-(rng_state_ & 1) & 0xb400);
  }

  static inline uint16_t state() { return rng_state_; }

  static inline void Seed(uint16_t seed) {
    rng_state_ = seed;
  }

  static inline uint8_t state_msb() {
    return static_cast<uint8_t>(rng_state_ >> 8);
  }

  static inline uint8_t GetByte() {
    Update();
    return state_msb();
  }
  
  static inline uint16_t GetWord() {
    Update();
    return state();
  }

 private:
  static uint16_t rng_state_;

  DISALLOW_COPY_AND_ASSIGN(Random);
};

}  // namespace avrlibx

#endif  // AVRLIBX_UTILS_RANDOM_H_
