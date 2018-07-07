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
// Calibration settings.

#ifndef CLOUDS_CV_SCALER_H_
#define CLOUDS_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "clouds/settings.h"
#include "clouds/drivers/adc.h"
#include "clouds/drivers/gate_input.h"
#include "clouds/dsp/parameters.h"

namespace clouds {

enum BlendParameter {
  BLEND_PARAMETER_DRY_WET,
  BLEND_PARAMETER_STEREO_SPREAD,
  BLEND_PARAMETER_FEEDBACK,
  BLEND_PARAMETER_REVERB,
  BLEND_PARAMETER_LAST
};

struct CvTransformation {
  bool flip;
  bool remove_offset;
  float filter_coefficient;
};

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init(CalibrationData* calibration_data);
  void Read(Parameters* parameters);
  
  void CalibrateC1() {
    cv_c1_ = adc_.float_value(ADC_V_OCT_CV);
  }

  void CalibrateOffsets() {
    for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
      calibration_data_->offset[i] = adc_.float_value(i);
    }
  }
  
  bool CalibrateC3() {
    float c3 = adc_.float_value(ADC_V_OCT_CV);  // 0.4848 v0.1 ; 0.3640 v0.2
    float c1 = cv_c1_;  // 0.6666 v0.1 ; 0.6488 v0.2
    float delta = c3 - c1;
    if (delta > -0.5f && delta < -0.0f) {
      calibration_data_->pitch_scale = 24.0f / (c3 - c1);
      calibration_data_->pitch_offset = 12.0f - \
          calibration_data_->pitch_scale * c1;
      return true;
    } else {
      return false;
    }
  }
  
  inline uint8_t adc_value(size_t index) const {
    return adc_.value(index) >> 8;
  }
  
  inline bool gate(size_t index) const {
    return index == 0 ? gate_input_.freeze() : gate_input_.trigger();
  }
  
  inline void set_blend_parameter(BlendParameter parameter) {
    blend_parameter_ = parameter;
    blend_knob_origin_ = previous_blend_knob_value_;
  }
  
  inline BlendParameter blend_parameter() const {
    return blend_parameter_;
  }
  
  inline float blend_value(BlendParameter parameter) const {
    return blend_[parameter];
  }
  
  inline void set_blend_value(BlendParameter parameter, float value) {
    blend_[parameter] = value;
  }
  
  inline bool blend_knob_touched() const {
    return blend_knob_touched_;
  }
  
  void UnlockBlendKnob() {
    previous_blend_knob_value_ = -1.0f;
  }

 private:
  void UpdateBlendParameters(float knob, float cv);
  static const int kAdcLatency = 5;
  
  Adc adc_;
  GateInput gate_input_;
  CalibrationData* calibration_data_;
  
  float smoothed_adc_value_[ADC_CHANNEL_LAST];
  static CvTransformation transformations_[ADC_CHANNEL_LAST];
  
  float note_;
  
  BlendParameter blend_parameter_;
  float blend_[BLEND_PARAMETER_LAST];
  float blend_mod_[BLEND_PARAMETER_LAST];
  float previous_blend_knob_value_;

  float blend_knob_origin_;
  float blend_knob_quantized_;
  bool blend_knob_touched_;

  float cv_c1_;
  
  bool previous_trigger_[kAdcLatency];
  bool previous_gate_[kAdcLatency];
  
  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

}  // namespace clouds

#endif  // CLOUDS_CV_SCALER_H_
