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

#ifndef PEAKS_CALIBRATION_DATA_H_
#define PEAKS_CALIBRATION_DATA_H_

#include "stmlib/stmlib.h"

namespace peaks {

struct CalibrationSettings {
  int16_t dac_offset[2];
};

class CalibrationData {
 public:
  CalibrationData() { }
  ~CalibrationData() { }
  
  void Init();
  
  inline uint16_t DacCode(uint8_t channel, int16_t value) const {
    int32_t shifted_value = 32767 - static_cast<int32_t>(value);
    shifted_value += static_cast<int32_t>(
        calibration_settings_.dac_offset[channel]);
    CONSTRAIN(shifted_value, 0, 65535);
    return static_cast<uint16_t>(shifted_value);
  }
  
  inline void set_dac_offset(uint8_t channel, int16_t offset) {
    calibration_settings_.dac_offset[channel] = offset;
  }
  
  void Save();
  
 private:
  CalibrationSettings calibration_settings_;
  
  DISALLOW_COPY_AND_ASSIGN(CalibrationData);
};

}  // namespace peaks

#endif  // PEAKS_CALIBRATION_DATA_H_
