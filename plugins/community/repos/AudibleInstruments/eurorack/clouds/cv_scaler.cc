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

#include "clouds/cv_scaler.h"

#include <algorithm>
#include <cmath>

#include "stmlib/dsp/dsp.h"

#include "clouds/resources.h"

namespace clouds {

using namespace std;

/* static */
CvTransformation CvScaler::transformations_[ADC_CHANNEL_LAST] = {
  // ADC_POSITION_POTENTIOMETER_CV,
  { true, false, 0.05f },
  // ADC_DENSITY_POTENTIOMETER_CV,
  { true, false, 0.01f },
  // ADC_SIZE_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_SIZE_CV,
  { false, true, 0.1f },
  // ADC_PITCH_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_V_OCT_CV,
  { false, false, 1.00f },
  // ADC_BLEND_POTENTIOMETER,
  { false, false, 0.05f },
  // ADC_BLEND_CV,
  { false, true, 0.2f },
  // ADC_TEXTURE_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_TEXTURE_CV,
  { false, true, 0.01f }
};

void CvScaler::Init(CalibrationData* calibration_data) {
  adc_.Init();
  gate_input_.Init();
  calibration_data_ = calibration_data;
  fill(&smoothed_adc_value_[0], &smoothed_adc_value_[ADC_CHANNEL_LAST], 0.0f);
  note_ = 0.0f;

  fill(&blend_[0], &blend_[BLEND_PARAMETER_LAST], 0.0f);
  fill(&blend_mod_[0], &blend_mod_[BLEND_PARAMETER_LAST], 0.0f);
  previous_blend_knob_value_ = 0.0f;
  blend_parameter_ = BLEND_PARAMETER_DRY_WET;
  blend_knob_quantized_ = -1.0f;
  blend_knob_touched_ = false;
  
  fill(&previous_trigger_[0], &previous_trigger_[kAdcLatency], false);
  fill(&previous_gate_[0], &previous_gate_[kAdcLatency], false);
}

void CvScaler::UpdateBlendParameters(float knob_value, float cv) {
  // Update the blending settings (base value and modulation) from the 
  // Blend knob and CV.
  for (int32_t i = 0; i < BLEND_PARAMETER_LAST; ++i) {
    float target = i == blend_parameter_ ? cv : 0.0f;
    float coefficient = i == blend_parameter_ ? 0.1f : 0.002f;
    blend_mod_[i] += coefficient * (target - blend_mod_[i]);
  }
  
  // Determines if the blend knob has been touched.
  if (blend_knob_quantized_ == -1.0f) {
    blend_knob_quantized_ = knob_value;
  }
  blend_knob_touched_ = fabs(knob_value - blend_knob_quantized_) > 0.02f;
  if (blend_knob_touched_) {
    blend_knob_quantized_ = knob_value;
  }
  
  if (previous_blend_knob_value_ == -1.0f) {
    blend_[blend_parameter_] = knob_value;
    previous_blend_knob_value_ = knob_value;
    blend_knob_origin_ = knob_value;
  }
  
  float parameter_value = blend_[blend_parameter_];
  float delta = knob_value - previous_blend_knob_value_;
  float skew_ratio = delta > 0.0f
      ? (1.001f - parameter_value) / (1.001f - previous_blend_knob_value_)
      : (0.001f + parameter_value) / (0.001f + previous_blend_knob_value_);
  CONSTRAIN(skew_ratio, 0.1f, 10.0f);
  if (fabs(knob_value - blend_knob_origin_) < 0.02f) {
    delta = 0.0f;
  } else {
    blend_knob_origin_ = -1.0f;
  }
  parameter_value += skew_ratio * delta;
  CONSTRAIN(parameter_value, 0.0f, 1.0f);
  blend_[blend_parameter_] = parameter_value;
  previous_blend_knob_value_ = knob_value;
}

void CvScaler::Read(Parameters* parameters) {
  for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
    const CvTransformation& transformation = transformations_[i];
    
    float value = adc_.float_value(i);
    if (transformation.flip) {
      value = 1.0f - value;
    }
    if (transformation.remove_offset) {
      value -= calibration_data_->offset[i];
    }
    smoothed_adc_value_[i] += transformation.filter_coefficient * \
        (value - smoothed_adc_value_[i]);
  }
  
  parameters->position = smoothed_adc_value_[ADC_POSITION_POTENTIOMETER_CV];
  
  float texture = smoothed_adc_value_[ADC_TEXTURE_POTENTIOMETER];
  texture -= smoothed_adc_value_[ADC_TEXTURE_CV] * 2.0f;
  CONSTRAIN(texture, 0.0f, 1.0f);
  parameters->texture = texture;

  float density = smoothed_adc_value_[ADC_DENSITY_POTENTIOMETER_CV];
  CONSTRAIN(density, 0.0f, 1.0f);
  parameters->density = density;

  parameters->size = smoothed_adc_value_[ADC_SIZE_POTENTIOMETER];
  parameters->size -= smoothed_adc_value_[ADC_SIZE_CV];
  CONSTRAIN(parameters->size, 0.0f, 1.0f);

  UpdateBlendParameters(
      smoothed_adc_value_[ADC_BLEND_POTENTIOMETER],
      -smoothed_adc_value_[ADC_BLEND_CV] * 2.0f);
  
  float dry_wet = blend_[BLEND_PARAMETER_DRY_WET];
  dry_wet += blend_mod_[BLEND_PARAMETER_DRY_WET];
  dry_wet = dry_wet * 1.05f - 0.025f;
  CONSTRAIN(dry_wet, 0.0f, 1.0f);
  parameters->dry_wet = dry_wet;

  float reverb_amount = blend_[BLEND_PARAMETER_REVERB];
  reverb_amount += blend_mod_[BLEND_PARAMETER_REVERB];
  CONSTRAIN(reverb_amount, 0.0f, 1.0f);
  parameters->reverb = reverb_amount;

  float feedback = blend_[BLEND_PARAMETER_FEEDBACK];
  feedback += blend_mod_[BLEND_PARAMETER_FEEDBACK];
  CONSTRAIN(feedback, 0.0f, 1.0f);
  parameters->feedback = feedback;

  float stereo_spread = blend_[BLEND_PARAMETER_STEREO_SPREAD];
  stereo_spread += blend_mod_[BLEND_PARAMETER_STEREO_SPREAD];
  CONSTRAIN(stereo_spread, 0.0f, 1.0f);
  parameters->stereo_spread = stereo_spread;
  
  parameters->pitch = stmlib::Interpolate(
      lut_quantized_pitch,
      smoothed_adc_value_[ADC_PITCH_POTENTIOMETER],
      1024.0f);
  
  float note = calibration_data_->pitch_offset;
  note += smoothed_adc_value_[ADC_V_OCT_CV] * calibration_data_->pitch_scale;
  if (fabs(note - note_) > 0.5f) {
    note_ = note;
  } else {
    ONE_POLE(note_, note, 0.2f)
  }
  
  parameters->pitch += note_;
  CONSTRAIN(parameters->pitch, -48.0f, 48.0f);
  
  gate_input_.Read();
  if (gate_input_.freeze_rising_edge()) {
    parameters->freeze = true;
  } else if (gate_input_.freeze_falling_edge()) {
    parameters->freeze = false;
  }
  
  parameters->trigger = previous_trigger_[0];
  parameters->gate = previous_gate_[0];
  for (int i = 0; i < kAdcLatency - 1; ++i) {
    previous_trigger_[i] = previous_trigger_[i + 1];
    previous_gate_[i] = previous_gate_[i + 1];
  }
  previous_trigger_[kAdcLatency - 1] = gate_input_.trigger_rising_edge();
  previous_gate_[kAdcLatency - 1] = gate_input_.gate();
  
  adc_.Convert();
}

}  // namespace clouds
