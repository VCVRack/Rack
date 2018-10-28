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
// Quantize voltages by sampling from a discrete distribution.

#include "marbles/random/discrete_distribution_quantizer.h"

#include "stmlib/dsp/dsp.h"

#include <cmath>
#include <algorithm>

namespace marbles {

using namespace stmlib;
using namespace std;

void DiscreteDistributionQuantizer::Init(const Scale& scale) {
  int n = scale.num_degrees;

  // We don't want garbage scale data here...
  if (!n || n > kMaxDegrees || scale.base_interval == 0.0f) {
    return;
  }

  base_interval_ = scale.base_interval;
  base_interval_reciprocal_ = 1.0f / scale.base_interval;
  num_cells_ = n + 1;
  for (int i = 0; i <= n; ++i) {
    float previous_voltage = scale.cell_voltage(i == 0 ? 0 : i - 1);
    float next_voltage = scale.cell_voltage(i == n ? n : i + 1);
    cells_[i].center = scale.cell_voltage(i);
    cells_[i].width = 0.5f * (next_voltage - previous_voltage);
    cells_[i].weight = static_cast<float>(scale.degree[i % n].weight) / 256.0f;
  }
}

float DiscreteDistributionQuantizer::Process(float value, float amount) {
  if (amount < 0.0f) {
    return value;
  }
  
  float raw_value = value;

  // Assuming 1V/Octave and a scale repeating every octave, note_integral
  // will store the octave number, and note_fractional the fractional
  // pitch class.
  const float note = value * base_interval_reciprocal_;
  MAKE_INTEGRAL_FRACTIONAL(note);
  if (value < 0.0f) {
    note_integral -= 1;
    note_fractional += 1.0f;
  }

  // For amount ranging between 0 and 0.25, do not remove notes from the scale
  // just crossfade from the unquantized output to the quantized output.
  const float scaled_amount = amount < 0.25f ? 0.0f : (amount - 0.25f) * 1.333f;
  
  distribution_.Init();
  for (int i = 0; i < num_cells_ - 1; ++i) {
    distribution_.AddToken(i, cells_[i].scaled_width(scaled_amount));
  }
  distribution_.NoMoreTokens();
  Distribution::Result r = distribution_.Sample(note_fractional);
  
  float quantized_value = cells_[r.token_id].center;
  float offset = static_cast<float>(note_integral) * base_interval_;
  quantized_value += offset;
  
  r.start *= base_interval_;
  r.start += offset;
  
  if (amount < 0.25f) {
    amount *= 4.0f;
    
    float x;
    if (r.token_id == 0) {
      x = r.fraction - 1.0f;
    } else if (r.token_id == num_cells_ - 1) {
      x = -r.fraction;
    } else {
      x = 2.0f * (fabs(r.fraction - 0.5f) - 0.5f);
    }
    const float slope = amount / (1.01f - amount);
    const float y = max(x * slope + 1.0f, 0.0f);
    quantized_value -= y * (quantized_value - raw_value);
  }
  
  return quantized_value;
}

}  // namespace marbles