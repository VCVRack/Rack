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

#include "rings/settings.h"

#include "stmlib/system/storage.h"

namespace rings {

stmlib::Storage<1> storage;

void Settings::Init() {
  freshly_baked_ = false;
  if (!storage.ParsimoniousLoad(&data_, &version_token_)) {
    data_.calibration_data.pitch_offset = 66.67f;
    data_.calibration_data.pitch_scale = -84.26f;
    for (size_t i = 0; i < ADC_CHANNEL_NUM_OFFSETS; ++i) {
      data_.calibration_data.offset[i] = 0.505f;
    }
    data_.state.polyphony = 1;
    data_.state.model = 0;
    data_.state.easter_egg = 0;
    data_.calibration_data.normalization_detection_threshold = 0.75f;
    freshly_baked_ = true;
    Save();
  }
  if (data_.calibration_data.normalization_detection_threshold < 0.7f ||
    data_.calibration_data.normalization_detection_threshold > 0.8f) {
    data_.calibration_data.normalization_detection_threshold = 0.75f;
  }
  CONSTRAIN(data_.state.polyphony, 1, 4);
  CONSTRAIN(data_.state.model, 0, 5);
}

void Settings::Save() {
  storage.ParsimoniousSave(data_, &version_token_);
}

}  // namespace rings
