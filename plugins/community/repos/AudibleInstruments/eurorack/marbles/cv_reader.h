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
// CV reader.

#ifndef MARBLES_CV_READER_H_
#define MARBLES_CV_READER_H_

#include "stmlib/stmlib.h"

#include "marbles/drivers/adc.h"

#include "marbles/cv_reader_channel.h"
#include "marbles/settings.h"

namespace marbles {

class CvReader {
 public:
  CvReader() { }
  ~CvReader() { }
  
  void Init(CalibrationData* calibration_data);

  inline bool ready_for_calibration() const {
    return true;
  }
  
  inline void CalibrateRateC1() {
    cv_c1_[0] = channel_[ADC_CHANNEL_T_RATE].unscaled_cv_lp();
  }
  
  inline void CalibrateRateC3() {
    cv_c3_[0] = channel_[ADC_CHANNEL_T_RATE].unscaled_cv_lp();
  }

  inline void CalibrateSpreadC1() {
    cv_c1_[1] = 0.5f * channel_[ADC_CHANNEL_X_SPREAD].unscaled_cv_lp() + \
        0.5f * channel_[ADC_CHANNEL_X_SPREAD_2].unscaled_cv_lp();
  }
  
  inline bool CalibrateSpreadC3() {
    cv_c3_[1] = 0.5f * channel_[ADC_CHANNEL_X_SPREAD].unscaled_cv_lp() + \
        0.5f * channel_[ADC_CHANNEL_X_SPREAD_2].unscaled_cv_lp();
    for (int i = 0; i < 2; ++i) {
      float c3 = cv_c3_[i]; // 0.2
      float c1 = cv_c1_[i];  // 0.4
      float delta = c3 - c1;
      float target_scale = i == 0 ? 24.0f : 0.4f;
      float target_offset = i == 0 ? 12.0f : 0.2f;
      if (delta > -0.3f && delta < -0.1f) {
        int channel = i == 0 ? ADC_CHANNEL_T_RATE : ADC_CHANNEL_X_SPREAD;
        calibration_data_->adc_scale[channel] = target_scale / (c3 - c1);
        calibration_data_->adc_offset[channel] = target_offset - \
            calibration_data_->adc_scale[channel] * c1;
      } else {
        return false;
      }
    }
    calibration_data_->adc_scale[ADC_CHANNEL_X_SPREAD_2] = calibration_data_->adc_scale[ADC_CHANNEL_X_SPREAD];
    calibration_data_->adc_offset[ADC_CHANNEL_X_SPREAD_2] = calibration_data_->adc_offset[ADC_CHANNEL_X_SPREAD];
    return true;
  }

  inline void CalibrateOffsets() {
    for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
      if (i != ADC_CHANNEL_T_RATE && i != ADC_CHANNEL_X_SPREAD) {
        calibration_data_->adc_offset[i] = \
            2.0f * channel_[i].unscaled_cv_lp();
      }
    }
  }
  
  inline uint8_t adc_value(int index) const {
    return adc_.value(index) >> 8;
  }
  
  void Copy(uint16_t* output);
  void Process(const uint16_t* values, float* output);
  
  inline const CvReaderChannel& channel(size_t index) {
    return channel_[index];
  }
  inline CvReaderChannel* mutable_channel(size_t index) {
    return &channel_[index];
  }
  inline void set_attenuverter(int index, float value) {
    attenuverter_[index] = value;
  }
  
 private:
  Adc adc_;
  CalibrationData* calibration_data_;
  float cv_c1_[2];
  float cv_c3_[2];
  
  CvReaderChannel channel_[ADC_CHANNEL_LAST];
  float attenuverter_[ADC_CHANNEL_LAST];
  static const CvReaderChannel::Settings channel_settings_[ADC_CHANNEL_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(CvReader);
};

}  // namespace marbles

#endif  // MARBLES_CV_READER_H_
