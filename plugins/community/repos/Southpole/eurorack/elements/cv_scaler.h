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

#ifndef ELEMENTS_CV_SCALER_H_
#define ELEMENTS_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "elements/drivers/cv_adc.h"
#include "elements/drivers/pots_adc.h"
#include "elements/drivers/gate_input.h"

namespace elements {

struct CalibrationSettings {
  float pitch_offset;
  float pitch_scale;
  float offset[CV_ADC_CHANNEL_LAST];
  uint8_t boot_in_easter_egg_mode;
  uint8_t resonator_model;
  uint8_t padding[62];
};

class Patch;
class PerformanceState;

enum Law {
  LAW_LINEAR,
  LAW_QUADRATIC_BIPOLAR,
  LAW_QUARTIC_BIPOLAR,
  LAW_QUANTIZED_NOTE
};

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init();
  void Read(Patch* patch, PerformanceState* state);
  bool ready_for_calibration() {
    return pot_lp_[POT_EXCITER_BOW_TIMBRE_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_BLOW_META_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_BLOW_TIMBRE_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_STRIKE_META_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_STRIKE_TIMBRE_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_RESONATOR_GEOMETRY_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_RESONATOR_BRIGHTNESS_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_RESONATOR_DAMPING_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_RESONATOR_POSITION_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_SPACE_ATTENUVERTER] < -3.0f;
  }
  
  bool exciter_low() {
    return pot_lp_[POT_EXCITER_BOW_TIMBRE_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_BLOW_META_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_BLOW_TIMBRE_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_STRIKE_META_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_STRIKE_TIMBRE_ATTENUVERTER] < -3.0f &&
      pot_lp_[POT_EXCITER_BLOW_TIMBRE] < 0.2f &&
      pot_lp_[POT_EXCITER_BOW_TIMBRE] < 0.2f &&
      pot_lp_[POT_EXCITER_STRIKE_TIMBRE] < 0.2f;
  }
  
  bool resonator_high() {
    return pot_lp_[POT_RESONATOR_GEOMETRY_ATTENUVERTER] > 3.0f &&
        pot_lp_[POT_RESONATOR_BRIGHTNESS_ATTENUVERTER] > 3.0f &&
        pot_lp_[POT_RESONATOR_DAMPING_ATTENUVERTER] > 3.0f &&
        pot_lp_[POT_RESONATOR_POSITION_ATTENUVERTER] > 3.0f &&
        pot_lp_[POT_SPACE_ATTENUVERTER] > 3.0f && 
        pot_lp_[POT_RESONATOR_DAMPING] > 0.8f &&
        pot_lp_[POT_RESONATOR_POSITION] > 0.8f &&
        pot_lp_[POT_SPACE] > 0.8f;
  }
  
  void CalibrateC1() {
    cv_c1_ = cv_.float_value(CV_ADC_V_OCT);
  }

  void CalibrateOffsets() {
    for (size_t i = 0; i < CV_ADC_CHANNEL_LAST; ++i) {
      calibration_settings_.offset[i] = cv_.float_value(i);
    }
  }
  
  bool CalibrateC3() {
    float c3 = cv_.float_value(CV_ADC_V_OCT);  // 0.4848 v0.1 ; 0.3640 v0.2
    float c1 = cv_c1_;  // 0.6666 v0.1 ; 0.6488 v0.2
    float delta = c3 - c1;
    if (delta > -0.4f && delta < -0.1f) {
      calibration_settings_.pitch_scale = 24.0f / (c3 - c1);
      calibration_settings_.pitch_offset = 12.0f - \
          calibration_settings_.pitch_scale * c1;
      return true;
    }
    return false;
  }
  
  void SaveCalibration();
  
  inline bool gate() const { return gate_; }
  inline uint8_t pot_value(size_t index) const {
    return pots_.value(index) >> 8;
  }
  inline uint8_t cv_value(size_t index) const {
    return cv_.value(index) >> 8;
    // return static_cast<uint8_t>(cv_.float_value(index) * 256.0f);
  }
  
  inline bool boot_in_easter_egg_mode() const {
    return calibration_settings_.boot_in_easter_egg_mode;
  }

  inline uint8_t resonator_model() const {
    return calibration_settings_.resonator_model;
  }
  
  inline void set_boot_in_easter_egg_mode(bool boot_in_easter_egg_mode) {
    calibration_settings_.boot_in_easter_egg_mode = boot_in_easter_egg_mode;
  }
  
  inline void set_resonator_model(uint8_t resonator_model) {
    calibration_settings_.resonator_model = resonator_model;
  }
  
  inline bool freshly_baked() const {
    return freshly_baked_;
  }
  
 private:
  void ReadPanelPots();
  PotsAdc pots_;
  CvAdc cv_;
  CalibrationSettings calibration_settings_;
  GateInput gate_input_;
  
  bool freshly_baked_;
  bool gate_;
  bool previous_gate_;
  float pot_raw_[POT_LAST];
  float pot_lp_[POT_LAST];
  float pot_quantized_[POT_LAST];
  float note_;
  float modulation_;
  float cv_c1_;
  
  uint16_t version_token_;
  
  static Law law_[POT_LAST];

  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

}  // namespace elements

#endif  // ELEMENTS_CV_SCALER_H_
