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
// CV scaling functions.

#include "tides/cv_scaler.h"

#include "stmlib/system/storage.h"

namespace tides {

using namespace stmlib;

/* static */
const CvScaler::CalibrationData CvScaler::init_calibration_data_ = {
  65535,  // 64576
  20653,  // 20030
  -9387,  // -9364
  33100,  // 33221
  -12673  // -12641
};

Storage<0x8020000, 1> storage;

void CvScaler::Init() {
  if (!storage.ParsimoniousLoad(&calibration_data_, &version_token_)) {
    calibration_data_ = init_calibration_data_;
  }
}

void CvScaler::SaveCalibrationData() {
  storage.ParsimoniousSave(calibration_data_, &version_token_);
}

void CvScaler::Calibrate() {
  calibration_data_.v_oct_offset = (v_oct_c2_ + v_oct_) >> 1;
  int32_t delta = v_oct_ - v_oct_c2_;
  if (delta != 0) {
    calibration_data_.v_oct_scale = (1 << 15) * (24 << 7) / delta;
  }
  calibration_data_.fm_offset = fm_;
  calibration_data_.level_offset = level_raw_;
  calibration_data_.fm_scale = calibration_data_.v_oct_scale * 27 / 20;
  SaveCalibrationData();
}

}  // namespace tides
