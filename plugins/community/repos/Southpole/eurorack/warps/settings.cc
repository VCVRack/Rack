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
// Settings storage.

#include "warps/settings.h"

#include "stmlib/system/storage.h"

namespace warps {

stmlib::Storage<1> storage;

void Settings::Init() {
  freshly_baked_ = false;
  if (!storage.ParsimoniousLoad(&data_, &version_token_)) {
    data_.calibration_data.pitch_offset = 100.00f;
    data_.calibration_data.pitch_scale = -110.00f;
    data_.calibration_data.offset[0] = 0.909f;  // (-10*-36/120.0)/3.3
    data_.calibration_data.offset[1] = 0.909f;
    data_.calibration_data.offset[2] = 0.495f;
    data_.calibration_data.offset[3] = 0.505f;
    // normalization probe = LOW ->
    //   ADC = 1 / 3.3 * (-36/120.0 * -10 + -36/300.0 * 0.0) = 0.91
    // normalization probe = HIGH ->
    //   ADC = 1 / 3.3 * (-36/120.0 * -10 + -36/300.0 * 3.3) = 0.79
    data_.calibration_data.normalization_detection_threshold[0] = 0.85f;
    data_.calibration_data.normalization_detection_threshold[1] = 0.85f;
    data_.state.carrier_shape = 1;
    data_.state.boot_in_easter_egg_mode = 0;
    freshly_baked_ = true;
    Save();
  }
  for (int i = 0; i < 2; ++i) {
    if (data_.calibration_data.normalization_detection_threshold[i] > 0.9f ||
        data_.calibration_data.normalization_detection_threshold[i] < 0.8f) {
      data_.calibration_data.normalization_detection_threshold[i] = 0.85f;
    }
  }
}

void Settings::Save() {
  storage.ParsimoniousSave(data_, &version_token_);
}

}  // namespace warps
