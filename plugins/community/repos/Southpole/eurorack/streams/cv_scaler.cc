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
// Calibration settings.

#include "streams/cv_scaler.h"

#include "stmlib/system/storage.h"

#include "streams/gain.h"

namespace streams {

using namespace stmlib;

Storage<0x8020000, 1> calibration_storage;

void CvScaler::Init(Adc* adc) {
  adc_ = adc;
  if (!calibration_storage.Load(&calibration_settings_)) {
    for (uint8_t i = 0; i < 2; ++i) {
      calibration_settings_.excite_offset[i] = 32768;
      calibration_settings_.audio_offset[i] = 32768;
      calibration_settings_.dac_offset[i] = \
          kDefaultOffset - (kDefaultOffset >> 4);
    }
  }
}

bool CvScaler::can_calibrate() const {
  for (uint8_t i = 0; i < 2; ++i) {
    for (uint8_t j = 0; j < 2; ++j) {
      if (adc_->cv(3 * i + j) > 36864 || adc_->cv(3 * i + j) < 28672) {
        // Do not calibrate when it looks like the user is doing something
        // stupid.
        return false;
      }
    }
  }
  return true;
}

void CvScaler::CaptureAdcOffsets() {
  for (uint8_t i = 0; i < 2; ++i) {
    calibration_settings_.excite_offset[i] = adc_->cv(3 * i + 0);
    calibration_settings_.audio_offset[i] = adc_->cv(3 * i + 1);
  }
}

void CvScaler::SaveCalibrationData() {
  calibration_storage.Save(calibration_settings_);
}

}  // namespace streams
