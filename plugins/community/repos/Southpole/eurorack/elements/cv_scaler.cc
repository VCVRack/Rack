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

#include "elements/cv_scaler.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/system/storage.h"

#include "elements/dsp/part.h"
#include "elements/dsp/patch.h"

namespace elements {

using namespace std;
using namespace stmlib;

/* static */
Law CvScaler::law_[POT_LAST] = {
  LAW_LINEAR,  // POT_EXCITER_ENVELOPE_SHAPE,
  LAW_LINEAR,  // POT_EXCITER_BOW_LEVEL,
  LAW_LINEAR,  // POT_EXCITER_BOW_TIMBRE,
  LAW_QUADRATIC_BIPOLAR,  // POT_EXCITER_BOW_TIMBRE_ATTENUVERTER,
  LAW_LINEAR,  // POT_EXCITER_BLOW_LEVEL,
  LAW_LINEAR,  // POT_EXCITER_BLOW_META,
  LAW_QUADRATIC_BIPOLAR,  // POT_EXCITER_BLOW_META_ATTENUVERTER,
  LAW_LINEAR,  // POT_EXCITER_BLOW_TIMBRE,
  LAW_QUADRATIC_BIPOLAR,  // POT_EXCITER_BLOW_TIMBRE_ATTENUVERTER,
  LAW_LINEAR,  // POT_EXCITER_STRIKE_LEVEL,
  LAW_LINEAR,  // POT_EXCITER_STRIKE_META,
  LAW_QUADRATIC_BIPOLAR,  // POT_EXCITER_STRIKE_META_ATTENUVERTER,
  LAW_LINEAR,  // POT_EXCITER_STRIKE_TIMBRE,
  LAW_QUADRATIC_BIPOLAR,  // POT_EXCITER_STRIKE_TIMBRE_ATTENUVERTER,
  LAW_QUANTIZED_NOTE,  // POT_RESONATOR_COARSE,
  LAW_QUADRATIC_BIPOLAR,  // POT_RESONATOR_FINE,
  LAW_QUARTIC_BIPOLAR,  // POT_RESONATOR_FM_ATTENUVERTER,
  LAW_LINEAR,  // POT_RESONATOR_GEOMETRY,
  LAW_QUADRATIC_BIPOLAR,  // POT_RESONATOR_GEOMETRY_ATTENUVERTER,
  LAW_LINEAR,  // POT_RESONATOR_BRIGHTNESS,
  LAW_QUADRATIC_BIPOLAR,  // POT_RESONATOR_BRIGHTNESS_ATTENUVERTER,
  LAW_LINEAR,  // POT_RESONATOR_DAMPING,
  LAW_QUADRATIC_BIPOLAR,  // POT_RESONATOR_DAMPING_ATTENUVERTER,
  LAW_LINEAR,  // POT_RESONATOR_POSITION,
  LAW_QUADRATIC_BIPOLAR,  // POT_RESONATOR_POSITION_ATTENUVERTER,
  LAW_LINEAR,  // POT_SPACE,
  LAW_QUADRATIC_BIPOLAR  // POT_SPACE_ATTENUVERTER
};

Storage<1> storage;

void CvScaler::Init() {
  pots_.Init();
  cv_.Init(false);
  gate_input_.Init();
  
  freshly_baked_ = false;
  if (!storage.ParsimoniousLoad(&calibration_settings_, &version_token_)) {
    calibration_settings_.pitch_offset = 66.67f;
    calibration_settings_.pitch_scale = -84.26f;
    for (size_t i = 0; i < CV_ADC_CHANNEL_LAST; ++i) {
      calibration_settings_.offset[i] = 10.0f * 20.0f / 120.0f / 3.3f;
    }
    calibration_settings_.boot_in_easter_egg_mode = false;
    calibration_settings_.resonator_model = 0;
    freshly_baked_ = true;
    SaveCalibration();
  }
  
  CONSTRAIN(calibration_settings_.resonator_model, 0, 2);
  
  note_ = 0.0f;
  modulation_ = 0.0f;
  fill(&pot_raw_[0], &pot_raw_[POT_LAST], 0.0f);
  fill(&pot_lp_[0], &pot_lp_[POT_LAST], 0.0f);
  fill(&pot_quantized_[0], &pot_quantized_[POT_LAST], 0.0f);
}

void CvScaler::SaveCalibration() {
  storage.ParsimoniousSave(calibration_settings_, &version_token_);
}

void CvScaler::ReadPanelPots() {
  // Read one pot from the front panel, and map it to the correct range.
  pots_.Scan();
  uint8_t index = pots_.last_read();
  
  switch (law_[index]) {
    case LAW_LINEAR:
      pot_raw_[index] = static_cast<float>(pots_.value(index)) / 65536.0f;
      break;
    
    case LAW_QUADRATIC_BIPOLAR:
      {
        float x = static_cast<float>(pots_.value(index)) / 32768.0f - 1.0f;
        float x2 = x * x * 3.3f;
        pot_raw_[index] = x < 0.0f ? -x2 : x2;
      }
      break;
    
    case LAW_QUARTIC_BIPOLAR:
      {
        float x = static_cast<float>(pots_.value(index)) / 32768.0f - 1.0f;
        float x2 = x * x;
        float x4 = x2 * x2 * 3.3f;
        pot_raw_[index] = x < 0.0f ? -x4 : x4;
      }
      break;
    
    case LAW_QUANTIZED_NOTE:
      {
        // 5 octaves.
        float note = 60.0f * static_cast<float>(pots_.value(index)) / 65536.0f;
        pot_raw_[index] += 0.5f * (note - pot_raw_[index]);
        float n = pot_raw_[index];
        float hysteresis = n - pot_quantized_[index] > 0.0f ? -0.3f : +0.3f;
        pot_quantized_[index] = static_cast<int32_t>(n + hysteresis + 0.5f);
      }
      break;
  }
  
  // Low-pass filter all front-panel pots.
  for (size_t i = 0; i < POT_LAST; ++i) {
    pot_lp_[i] += 0.01f * (pot_raw_[i] - pot_lp_[i]);
  }
}

#define BIND(destination, NAME, lp_coefficient, min, max) \
  { \
    const float* offset = calibration_settings_.offset; \
    float pot = pot_lp_[POT_ ## NAME]; \
    float cv = offset[CV_ADC_ ## NAME] - cv_.float_value(CV_ADC_ ## NAME); \
    float value = pot + pot_lp_[POT_ ## NAME ## _ATTENUVERTER] * cv; \
    CONSTRAIN(value, min, max); \
    if (lp_coefficient != 1.0f) { \
      destination += (value - destination) * lp_coefficient; \
    } else { \
      destination = value; \
    } \
  }

void CvScaler::Read(
    Patch* patch,
    PerformanceState* state) {
  ReadPanelPots();
  
  patch->exciter_envelope_shape = pot_lp_[POT_EXCITER_ENVELOPE_SHAPE];
  patch->exciter_bow_level = pot_lp_[POT_EXCITER_BOW_LEVEL];
  patch->exciter_blow_level = pot_lp_[POT_EXCITER_BLOW_LEVEL];
  patch->exciter_strike_level = pot_lp_[POT_EXCITER_STRIKE_LEVEL];
  
  BIND(patch->exciter_bow_timbre, EXCITER_BOW_TIMBRE, 1.0f, 0.0f, 0.9995f);
  BIND(patch->exciter_blow_meta, EXCITER_BLOW_META, 0.05f, 0.0f, 0.9995f);
  BIND(patch->exciter_blow_timbre, EXCITER_BLOW_TIMBRE, 1.0f, 0.0f, 0.9995f);
  BIND(patch->exciter_strike_meta, EXCITER_STRIKE_META, 0.05f, 0.0f, 0.9995f);
  BIND(patch->exciter_strike_timbre, EXCITER_STRIKE_TIMBRE, 1.0f, 0.0f, 0.995f);
  BIND(patch->resonator_geometry, RESONATOR_GEOMETRY, 0.05f, 0.0f, 0.9995f);
  BIND(patch->resonator_brightness, RESONATOR_BRIGHTNESS, 1.0f, 0.0f, 0.9995f);
  BIND(patch->resonator_damping, RESONATOR_DAMPING, 1.0f, 0.0f, 0.9995f);
  BIND(patch->resonator_position, RESONATOR_POSITION, 0.01f, 0.0f, 0.9995f);
  BIND(patch->space, SPACE, 0.01f, 0.0f, 2.0f);
  
  float note = calibration_settings_.pitch_offset;
  note += cv_.float_value(CV_ADC_V_OCT) * calibration_settings_.pitch_scale;
  float interval = note - note_;
  // When a pitch difference of more than 1 quartertone is observed, do
  // not attempt to slew-limit and jump straight to the right pitch.
  if (interval < -0.4f || interval > 0.4f) {
    note_ = note;
  } else {
    note_ += 0.1f * interval;
  }

  float modulation = pot_lp_[POT_RESONATOR_FM_ATTENUVERTER] * 49.5f *
      (calibration_settings_.offset[CV_ADC_FM] - cv_.float_value(CV_ADC_FM));
  modulation_ += 0.5f * (modulation - modulation_);
  state->modulation = modulation_;
  
  state->note = note_;
  state->note += pot_quantized_[POT_RESONATOR_COARSE] + 19.0f;
  state->note += pot_lp_[POT_RESONATOR_FINE] * (2.0f / 3.3f);
  state->strength = 1.0f - cv_.float_value(CV_ADC_PRESSURE);

  CONSTRAIN(state->strength, 0.0f, 1.0f);
  CONSTRAIN(state->modulation, -60.0f, 60.0f);
  
  // Scan the gate input 1ms after the other CVs to prevent lag.
  state->gate = previous_gate_;
  previous_gate_ = gate_;
  gate_ = gate_input_.Read();
  
  cv_.Convert();
}

}  // namespace elements
