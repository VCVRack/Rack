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

#include "frames/poly_lfo.h"

#include <cstdio>
#include <algorithm>

#include "stmlib/utils/dsp.h"

#include "frames/resources.h"

#include "stmlib/utils/random.h"

namespace frames {

using namespace std;
using namespace stmlib;

/* static */
const uint8_t PolyLfo::rainbow_[17][3] = {
  { 255, 0, 0 },
  { 255, 32, 0 },
  { 255, 192, 0 },
  { 255, 240, 0 },
  { 240, 255, 0 },
  { 192, 255, 0 },
  { 32, 255, 0 },
  { 0, 255, 0 },
  { 0, 255, 32 },
  { 0, 255, 192 },
  { 0, 255, 255 },
  { 0, 192, 255 },
  { 0, 32, 255 },
  { 0, 0, 255 },
  { 32, 0, 255 },
  { 192, 0, 192 },
  { 255, 0, 128 },
};

void PolyLfo::Init() {
  spread_ = 0;
  shape_ = 0;
  shape_spread_ = 0;
  coupling_ = 0;
  std::fill(&value_[0], &value_[kNumChannels], 0);
}

/* static */
uint32_t PolyLfo::FrequencyToPhaseIncrement(int32_t frequency) {
  int32_t shifts = frequency / 5040;
  int32_t index = frequency - shifts * 5040;
  uint32_t a = lut_increments[index >> 5];
  uint32_t b = lut_increments[(index >> 5) + 1];
  return (a + ((b - a) * (index & 0x1f) >> 5)) << shifts;
}
  
void PolyLfo::Render(int32_t frequency) {
  if (frequency < 0) frequency = 0;
  uint16_t rainbow_index = frequency > 65535 ? 65535 : frequency;
  for (uint8_t i = 0; i < 3; ++i) {
    int16_t a = rainbow_[rainbow_index >> 12][i];
    int16_t b = rainbow_[(rainbow_index >> 12) + 1][i];
    color_[i] = a + ((b - a) * (rainbow_index & 0x0fff) >> 12);
  }
  
  // Advance phasors.
  if (spread_ >= 0) {
    phase_[0] += FrequencyToPhaseIncrement(frequency);
    uint32_t phase_difference = static_cast<uint32_t>(spread_) << 15;
    phase_[1] = phase_[0] + phase_difference;
    phase_[2] = phase_[1] + phase_difference;
    phase_[3] = phase_[2] + phase_difference;
  } else {
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      phase_[i] += FrequencyToPhaseIncrement(frequency);
      frequency -= 5040 * spread_ >> 15;
    }
  }
  
  const uint8_t* sine = &wt_lfo_waveforms[17 * 257];
  
  uint16_t wavetable_index = shape_;
  // Wavetable lookup
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    uint32_t phase = phase_[i];
    if (coupling_ > 0) {
      phase += value_[(i + 1) % kNumChannels] * coupling_;
    } else {
      phase += value_[(i + kNumChannels - 1) % kNumChannels] * -coupling_;
    }
    const uint8_t* a = &wt_lfo_waveforms[(wavetable_index >> 12) * 257];
    const uint8_t* b = a + 257;
    int16_t value = Crossfade(a, b, phase, wavetable_index << 4);
    value_[i] = Interpolate824(sine, phase);
    level_[i] = (value + 32768) >> 8;
    dac_code_[i] = Keyframer::ConvertToDacCode(value + 32768, 0);
    wavetable_index += shape_spread_;
  }
}

void PolyLfo::Reset() {
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    phase_[i] = 0;
  }
}

void PolyLfo::Randomize() {
  for (int i=0; i<4; i++)
    phase_[i] = Random::GetWord();
}


}  // namespace frames
