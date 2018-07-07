// Copyright 2015 Olivier Gillet.
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
// Note quantizer

#include "braids/quantizer.h"

#include <algorithm>
#include <cstdlib>

namespace braids {

void Quantizer::Init() {
  enabled_ = true;
  codeword_ = 0;
  previous_boundary_ = 0;
  next_boundary_ = 0;
  for (int16_t i = 0; i < 128; ++i) {
    codebook_[i] = (i - 64) << 7;
  }
}

void Quantizer::Configure(
    const int16_t* notes,
    int16_t span,
    size_t num_notes) {
  enabled_ = notes != NULL && num_notes != 0 && span != 0;
  if (enabled_) {
    int32_t octave = 0;
    size_t note = 0;
    int16_t root = 0;
    for (int32_t i = 0; i < 64; ++i) {
      int32_t up = root + notes[note] + span * octave;
      int32_t down = root + notes[num_notes - 1 - note] + (-octave - 1) * span;
      CLIP(up)
      CLIP(down)
      codebook_[64 + i] = up;
      codebook_[64 - i - 1] = down;
      ++note;
      if (note >= num_notes) {
        note = 0;
        ++octave;
      }
    }
  }
}

int32_t Quantizer::Process(int32_t pitch, int32_t root) {
  if (!enabled_) {
    return pitch;
  }

  pitch -= root;
  if (pitch >= previous_boundary_ && pitch <= next_boundary_) {
    // We're still in the voronoi cell for the active codeword.
    pitch = codeword_;
  } else {
    // Search for the nearest neighbour in the codebook.
    int16_t upper_bound_index = std::upper_bound(
        &codebook_[3],
        &codebook_[126],
        static_cast<int16_t>(pitch)) - &codebook_[0];
    int16_t lower_bound_index = upper_bound_index - 2;

    int16_t best_distance = 16384;
    int16_t q = -1;
    for (int16_t i = lower_bound_index; i <= upper_bound_index; ++i) {
      int16_t distance = abs(pitch - codebook_[i]);
      if (distance < best_distance) {
        best_distance = distance;
        q = i;
      }
    }
    codeword_ = codebook_[q];
    // Enlarge the current voronoi cell a bit for hysteresis.
    previous_boundary_ = (9 * codebook_[q - 1] + 7 * codeword_) >> 4;
    next_boundary_ = (9 * codebook_[q + 1] + 7 * codeword_) >> 4;
    pitch = codeword_;
  }
  pitch += root;
  return pitch;
}

}  // namespace braids
