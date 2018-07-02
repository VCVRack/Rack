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

#ifndef STREAMS_CV_SCALER_H_
#define STREAMS_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "streams/cv_scaler.h"

#include "streams/drivers/adc.h"

namespace streams {

const uint8_t kNumChannels = 2;

struct CalibrationSettings {
  uint16_t excite_offset[kNumChannels];
  uint16_t audio_offset[kNumChannels];
  uint16_t dac_offset[kNumChannels];
  uint16_t padding[2];
};

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init(Adc* adc);
  void CaptureAdcOffsets();
  void SaveCalibrationData();
  
  bool can_calibrate() const;
  
  // This class owns the calibration data and is reponsible for removing
  // offsets.
  inline int16_t audio_sample(uint8_t channel) const {
    int32_t value = static_cast<int32_t>(
            calibration_settings_.audio_offset[channel]) - \
        adc_->cv(channel * 3 + 1);
    CLIP(value);
    return value;
  }

  inline int16_t excite_sample(uint8_t channel) const {
    int32_t value = static_cast<int32_t>(
            calibration_settings_.excite_offset[channel]) - \
        adc_->cv(channel * 3);
    CLIP(value);
    return value;
  }
  
  inline int32_t gain_sample(uint8_t channel) const {
    return -2 * adc_->cv(channel * 3 + 2);
  }

  inline int32_t raw_gain_sample(uint8_t channel) const {
    return adc_->cv(channel * 3 + 2);
  }
  
  inline void set_dac_offset(uint8_t channel, uint16_t offset) {
    calibration_settings_.dac_offset[channel] = offset;
  }
  
  inline uint16_t ScaleGain(uint8_t channel, uint16_t gain) const {
    int32_t g = static_cast<int32_t>(gain);
    g += static_cast<int32_t>(calibration_settings_.dac_offset[channel]);
    if (g > 65535) {
      g = 65535;
    }
    return static_cast<uint16_t>(g);
  }
  
 private:
  Adc* adc_;
  CalibrationSettings calibration_settings_;
  
  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

}  // namespace streams

#endif  // STREAMS_CV_SCALER_H_
