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
// CV scaler.

#ifndef WARPS_CV_SCALER_H_
#define WARPS_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "warps/drivers/adc.h"
#include "warps/drivers/codec.h"
#include "warps/drivers/normalization_probe.h"

#include "warps/settings.h"

namespace warps {

class Parameters;

class NormalizationDetector {
 public:
  NormalizationDetector() { }
  ~NormalizationDetector() { } 

  void Init(float lp_coefficient, float threshold) {
    score_ = 0.0f;
    state_ = false;
    threshold_ = threshold;
    lp_coefficient_ = lp_coefficient;
  }
  
  void Process(float x, float y) {
    // x is supposed to be centered!
    score_ += lp_coefficient_ * (x * y - score_);
    
    float hysteresis = state_ ? -0.05f * threshold_ : 0.0f;
    state_ = score_ >= (threshold_ + hysteresis);
  }
  
  inline bool normalized() const { return state_; }
  inline float score() const { return score_; }

 private:
  float score_;
  float lp_coefficient_;
  float threshold_;
  
  bool state_;

  DISALLOW_COPY_AND_ASSIGN(NormalizationDetector);
};

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init(CalibrationData* calibration_data);
  void Read(Parameters* parameters);
  
  void DetectAudioNormalization(Codec::Frame* in, size_t size);
  
  void StartCalibration() {
    normalization_probe_enabled_ = false;
    normalization_probe_forced_state_ = false;
  }
  
  void CalibrateC1() {
    cv_c1_ = adc_.float_value(ADC_LEVEL_1_CV);
  }

  void CalibrateOffsets() {
    for (size_t i = 1; i < 4; ++i) {
      calibration_data_->offset[i] = adc_.float_value(i);
    }
  }
  
  bool CalibrateC3() {
    float c3 = adc_.float_value(ADC_LEVEL_1_CV); // 0.5818181818181818
    float c1 = cv_c1_; // 0.8
    float delta = c3 - c1;
    bool success = delta > -0.4f && delta < -0.1f;
    if (success) {
      calibration_data_->pitch_scale = 24.0f / (c3 - c1);
      calibration_data_->pitch_offset = 12.0f - \
          calibration_data_->pitch_scale * c1;
      calibration_data_->offset[0] = c1 + 0.5f * (c1 - c3);
    }
    normalization_probe_enabled_ = true;
    return success;
  }
  
  void StartNormalizationCalibration() {
    normalization_probe_enabled_ = false;
    normalization_probe_forced_state_ = false;
  }
  
  void CalibrateLow() {
    cv_low_[0] = adc_.float_value(ADC_LEVEL_1_CV);
    cv_low_[1] = adc_.float_value(ADC_LEVEL_2_CV);
    normalization_probe_forced_state_ = true;
  }
  
  bool CalibrateHigh() {
    bool success = true;
    for (int i = 0; i < 2; ++i) {
      float high = adc_.float_value(ADC_LEVEL_1_CV + i);
      float threshold = (cv_low_[i] + high) * 0.5f;
      bool within_range = threshold >= 0.8f && threshold <= 0.9f;
      if (within_range) {
        calibration_data_->normalization_detection_threshold[i] = threshold;
      }
      success = success && within_range;
    }
    normalization_probe_enabled_ = true;
    return success;
  }
  
  inline bool ready_for_calibration() const {
    return adc_.float_value(ADC_LEVEL_2_CV) > 0.8f && \
         adc_.float_value(ADC_LEVEL_1_CV) > 0.7f;
  }
  
  inline uint8_t easter_egg_digit() const {
    if (lp_state_[ADC_LEVEL_1_POT] < 0.05f && \
        lp_state_[ADC_LEVEL_2_POT] < 0.05f && \
        lp_state_[ADC_PARAMETER_POT] < 0.05f) {
      return static_cast<uint8_t>(
          UnwrapPot(lp_state_[ADC_ALGORITHM_POT]) * 8.0f + 0.5f);
    } else {
      return 0;
    }
  }
  
  inline uint8_t adc_value(size_t index) const {
    return adc_.value(index) >> 8;
  }
  
  inline uint8_t normalization(size_t index) const {
    return normalization_detector_[index].normalized() ? 255 : 0;
  }
  
  float UnwrapPot(float x) const;
  
 private:
  void DetectNormalization();
  
  Adc adc_;
  CalibrationData* calibration_data_;
  NormalizationProbe normalization_probe_;
  NormalizationDetector normalization_detector_[4];

  bool normalization_probe_value_[2];
  bool normalization_probe_enabled_;
  bool normalization_probe_forced_state_;
  
  float lp_state_[ADC_LAST];
  float note_cv_;
  float note_pot_;
  float note_pot_quantized_;
  float cv_c1_;
  float cv_low_[2];
  
  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

}  // namespace warps

#endif  // WARPS_CV_SCALER_H_
