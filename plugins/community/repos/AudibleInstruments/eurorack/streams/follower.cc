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
// Follower.

#include "streams/follower.h"

#include "stmlib/utils/dsp.h"

#include "streams/gain.h"
#include "streams/resources.h"

namespace streams {

using namespace stmlib;

void Follower::Init() {
  analysis_low_.Init();
  analysis_low_.set_frequency(45 << 7);
  analysis_low_.set_resonance(0);
  analysis_medium_.Init();
  analysis_medium_.set_frequency(86 << 7);
  analysis_medium_.set_resonance(0);
  
  for (uint8_t i = 0; i < 3; ++i) {
    energy_[i][0] = energy_[i][1] = 0;
    follower_[i] = 0;
    follower_lp_[i] = 0;
    spectrum_[i] = 0;
  }
  centroid_ = 0;
}

void Follower::Process(
    int16_t audio,
    int16_t excite,
    uint16_t* gain,
    uint16_t* frequency) {
  // Smooth frequency amount parameters.
  frequency_amount_ += (target_frequency_amount_ - frequency_amount_) >> 8;
  frequency_offset_ += (target_frequency_offset_ - frequency_offset_) >> 8;

  analysis_low_.Process(excite);
  analysis_medium_.Process(analysis_low_.hp());
  
  int32_t channel[3];
  channel[0] = analysis_low_.lp();
  channel[1] = analysis_medium_.lp();
  channel[2] = analysis_medium_.hp();

  int32_t envelope = 0;
  int32_t centroid_numerator = 0;
  int32_t centroid_denominator = 0;
  for (int32_t i = 0; i < 3; ++i) {
    int32_t energy = channel[i];
    energy *= energy;
    
    // Ride an ascending peak.
    if (energy_[i][0] < energy_[i][1] && energy_[i][1] < energy &&
        energy > follower_[i]) {
      follower_[i] = energy;
    }
    // Otherwise, hold and snap on local maxima.
    if (energy_[i][0] <= energy_[i][1] && energy_[i][1] >= energy) {
      follower_[i] = energy_[i][1];
    }
    energy_[i][0] = energy_[i][1];
    energy_[i][1] = energy;
    
    // Then let a low-pass filter smooth things out.
    int64_t error = follower_[i] - follower_lp_[i];
    if (error > 0) {
      follower_lp_[i] += error * attack_coefficient_[i] >> 31;
    } else {
      follower_lp_[i] += error * decay_coefficient_[i] >> 31;
    }
    envelope += follower_lp_[i] >> 13;

    // Integrate more slowly for spectrum estimation.
    if (only_filter_) {
      error = follower_lp_[i] - spectrum_[i];
      spectrum_[i] += error >> 6;
    } else {
      error = follower_[i] - spectrum_[i];
      spectrum_[i] += error >> 10;
    }
    centroid_numerator += i * (spectrum_[i] >> 1) >> 16;
    centroid_denominator += spectrum_[i] >> 16;
  }
  
  if (envelope > 65535) {
    envelope = 65535;
  } else if (envelope < 0) {
    envelope = 0;
  }
  
  uint16_t gain_mod = Interpolate824(lut_square_root, envelope << 16) >> 1;
  int32_t centroid = (centroid_numerator << 15) / (centroid_denominator + 1);
  if (gain_mod > 4096) {
    centroid_ = centroid;
  } else if (gain_mod > 2048) {
    centroid_ += (centroid - centroid_) >> 8;
  }

  *gain = gain_mod * kUnityGain >> 15;
  *frequency = frequency_offset_ + (centroid_ * frequency_amount_ >> 15);
  
  if (only_filter_) {
    *gain = *frequency;
    *frequency = 65535;
  }
}

}  // namespace streams
