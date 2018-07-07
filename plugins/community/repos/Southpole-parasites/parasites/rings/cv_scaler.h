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
// Filtering and scaling of ADC values + input calibration.

#ifndef RINGS_CV_SCALER_H_
#define RINGS_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "rings/drivers/adc.h"
#include "rings/drivers/codec.h"
#include "rings/drivers/normalization_probe.h"
#include "rings/drivers/trigger_input.h"

#include "rings/settings.h"

namespace rings {

enum Law {
  LAW_LINEAR,
  LAW_QUADRATIC_BIPOLAR,
  LAW_QUARTIC_BIPOLAR
};

struct ChannelSettings {
  Law law;
  bool remove_offset;
  float lp_coefficient;
};

struct Patch;
struct PerformanceState;

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
  void Read(Patch* patch, PerformanceState* performance_state);

  void DetectAudioNormalization(Codec::Frame* in, size_t size);

  inline bool ready_for_calibration() const {
    return true;
  }
  
  inline bool easter_egg() const {
    return adc_lp_[ADC_CHANNEL_POT_FREQUENCY] < 0.1f &&
        adc_lp_[ADC_CHANNEL_POT_STRUCTURE] > 0.9f &&
        adc_lp_[ADC_CHANNEL_POT_BRIGHTNESS] < 0.1f &&
        adc_lp_[ADC_CHANNEL_POT_POSITION] > 0.9f &&
        adc_lp_[ADC_CHANNEL_POT_DAMPING] > 0.4f &&
        adc_lp_[ADC_CHANNEL_POT_DAMPING] < 0.6f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_BRIGHTNESS] < -1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_FREQUENCY] > 1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_DAMPING] < -1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_STRUCTURE] > 1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_POSITION] < -1.00f;
  }
  
  inline void CalibrateC1() {
    cv_c1_ = adc_.float_value(ADC_CHANNEL_CV_V_OCT);
  }

  inline void CalibrateOffsets() {
    for (size_t i = 0; i < ADC_CHANNEL_NUM_OFFSETS; ++i) {
      calibration_data_->offset[i] = adc_.float_value(i);
    }
  }
  
  inline bool CalibrateC3() {
    float c3 = adc_.float_value(ADC_CHANNEL_CV_V_OCT);  // 0.3640 v0.2
    float c1 = cv_c1_;  // 0.6488 v0.2
    float delta = c3 - c1;
    if (delta > -0.4f && delta < -0.1f) {
      calibration_data_->pitch_scale = 24.0f / (c3 - c1);
      calibration_data_->pitch_offset = 12.0f - \
          calibration_data_->pitch_scale * c1;
      return true;
    }
    normalization_probe_enabled_ = true;
    return false;
  }
  
  inline void StartNormalizationCalibration() {
    normalization_probe_enabled_ = false;
    normalization_probe_forced_state_ = false;
  }
  
  inline void CalibrateLow() {
    cv_low_ = adc_.float_value(ADC_CHANNEL_CV_V_OCT);
    normalization_probe_forced_state_ = true;
  }
  
  inline bool CalibrateHigh() {
    float threshold = (cv_low_ + adc_.float_value(ADC_CHANNEL_CV_V_OCT)) * 0.5f;
    bool within_range = threshold >= 0.7f && threshold < 0.8f;
    if (within_range) {
      calibration_data_->normalization_detection_threshold = threshold;
    }
    normalization_probe_enabled_ = true;
    return within_range;
  }
  
  inline uint8_t adc_value(size_t index) const {
    return adc_.value(index) >> 8;
  }
  
  inline bool gate_value() const {
    return trigger_input_.value();
  }

  inline uint8_t normalization(size_t index) const {
    switch (index) {
      case 0:
        return fm_cv_ * 3.3f > 0.8f ? 255 : 0;
        break;
        
      case 1:
        return normalization_detector_trigger_.normalized() ? 255 : 0;
        break;

      case 2:
        return normalization_detector_v_oct_.normalized() ? 255 : 0;
        break;
      
      default:
        return normalization_detector_exciter_.normalized() ? 255 : 0;
        break;
    }
  }
  
 private:
  void DetectNormalization();
  
  Adc adc_;
  CalibrationData* calibration_data_;
  TriggerInput trigger_input_;
  
  NormalizationProbe normalization_probe_;
  
  NormalizationDetector normalization_detector_trigger_;
  NormalizationDetector normalization_detector_v_oct_;
  NormalizationDetector normalization_detector_exciter_;
  
  bool normalization_probe_value_[2];
  int32_t inhibit_strum_;
  
  float adc_lp_[ADC_CHANNEL_LAST];
  float transpose_;
  float fm_cv_;
  float cv_c1_;
  float cv_low_;
  int32_t chord_;
  
  bool normalization_probe_enabled_;
  bool normalization_probe_forced_state_;
  
  static ChannelSettings channel_settings_[ADC_CHANNEL_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

}  // namespace rings

#endif  // RINGS_CV_SCALER_H_
