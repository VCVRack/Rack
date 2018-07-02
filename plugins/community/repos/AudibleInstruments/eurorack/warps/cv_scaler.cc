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

#include "warps/cv_scaler.h"

#include <algorithm>
#include <cmath>

#include "stmlib/dsp/dsp.h"
#include "stmlib/system/storage.h"
#include "stmlib/utils/random.h"

#include "warps/dsp/parameters.h"
#include "warps/resources.h"

namespace warps {

using namespace std;
using namespace stmlib;

void CvScaler::Init(CalibrationData* calibration_data) {
  calibration_data_ = calibration_data;

  adc_.Init();
  normalization_probe_.Init();
  note_cv_ = 0.0f;
  note_pot_ = 0.0f;
  fill(&lp_state_[0], &lp_state_[ADC_LAST], 0.0f);
  for (int32_t i = 0; i < 4; ++i) {
    normalization_detector_[i].Init(0.01f, 0.5f);
  }
  
  normalization_probe_enabled_ = true;
}

float CvScaler::UnwrapPot(float x) const {
  return Interpolate(lut_pot_curve, x, 512.0f);
}

#define BIND(destination, NAME, unwrap, scale, lp_coefficient, attenuate) \
  { \
    lp_state_[ADC_ ## NAME ## _POT] += 0.33f * lp_coefficient * (adc_.float_value(ADC_ ## NAME ## _POT) - lp_state_[ADC_ ## NAME ## _POT]); \
    lp_state_[ADC_ ## NAME ## _CV] += lp_coefficient * (adc_.float_value(ADC_ ## NAME ## _CV) - lp_state_[ADC_ ## NAME ## _CV]); \
    float pot = lp_state_[ADC_ ## NAME ## _POT]; \
    if (unwrap) pot = UnwrapPot(pot); \
    float cv = calibration_data_->offset[ADC_ ## NAME ## _CV] - lp_state_[ADC_ ## NAME ## _CV]; \
    float value = attenuate ? (pot * pot * cv * scale) : (pot + cv * scale); \
    CONSTRAIN(value, 0.0f, 1.0f); \
    destination = value; \
  }

void CvScaler::DetectNormalization() {
  // Check if the value read by the ADC is correlated with the noise sent to the
  // switch. If so, we can conclude that nothing is connected in the input.
  float x = normalization_probe_value_[1] ? 1.0f : -1.0f;
  for (size_t i = 0; i < 2; ++i) {
    float y = adc_.float_value(ADC_LEVEL_1_CV + i) - calibration_data_->normalization_detection_threshold[i];
    if (y > -0.1f && y < 0.1f) {
      y = y > 0.0f ? -1.0f : 1.0f;
    } else {
      y = 0.0f;
    }
    normalization_detector_[i].Process(x, y);
  }
  // Flip normalization probe for next round.
  normalization_probe_value_[1] = normalization_probe_value_[0];
  normalization_probe_value_[0] = (Random::GetWord() >> 31) & 1;
  bool new_state = normalization_probe_enabled_
      ? normalization_probe_value_[0]
      : normalization_probe_forced_state_;
  normalization_probe_.Write(new_state);
}

void CvScaler::DetectAudioNormalization(Codec::Frame* in, size_t size) {
  for (int32_t channel = 0; channel < 2; ++channel) {
    int32_t count = 0;
    short* input_samples = &in->l + channel;
    for (size_t i = 0; i < size; i += 16) {
      short s = input_samples[i * 2];
      if (s > 50 && s < 1500) {
        ++count;
      } else if (s > -1500 && s < -50) {
        --count;
      }
    }
    float y = static_cast<float>(count) / static_cast<float>(size >> 4);
    float x = normalization_probe_value_[0] ? -1.0f : 1.0f;
    
    normalization_detector_[channel + 2].Process(x, y);
    if (normalization_detector_[channel + 2].normalized()) {
      for (size_t i = 0; i < size; ++i) {
        input_samples[i * 2] = 0;
      }
    }
  }
}

void CvScaler::Read(Parameters* p) {
  // Modulation parameters.
  BIND(p->channel_drive[0], LEVEL_1, false, 1.6f, 0.08f, true);
  BIND(p->channel_drive[1], LEVEL_2, false, 1.6f, 0.08f, true);
  BIND(p->modulation_algorithm, ALGORITHM, true, 2.0f, 0.08f, false);
  BIND(p->modulation_parameter, PARAMETER, false, 2.0f, 0.08f, false);
  
  // Prevent wavefolder bleed caused by a slight offset in the pot or ADC.
  if (p->modulation_algorithm <= 0.125f) {
    p->modulation_algorithm = p->modulation_algorithm * 1.08f - 0.01f;
    CONSTRAIN(p->modulation_algorithm, 0.0f, 1.0f);
  }
  
  // Easter egg parameter mappings.
  p->frequency_shift_pot = lp_state_[ADC_ALGORITHM_POT];
  float frequency_shift_cv = -lp_state_[ADC_ALGORITHM_CV];
  frequency_shift_cv += calibration_data_->offset[ADC_ALGORITHM_CV];
  
  p->frequency_shift_cv = frequency_shift_cv * 2.0f;
  CONSTRAIN(p->frequency_shift_cv, -1.0f, 1.0f);

  float phase_shift = lp_state_[ADC_ALGORITHM_POT] + frequency_shift_cv * 2.0f;
  CONSTRAIN(phase_shift, 0.0f, 1.0f);
  p->phase_shift = phase_shift;

  // Internal oscillator parameters.
  float note;
  note = calibration_data_->pitch_offset;
  note += adc_.float_value(ADC_LEVEL_1_CV) * calibration_data_->pitch_scale;
  float interval = note - note_cv_;
  if (interval < -0.4f || interval > 0.4f) {
    note_cv_ = note;
  } else {
    note_cv_ += 0.1f * interval;
  }
  
  note = 60.0f * adc_.float_value(ADC_LEVEL_1_POT) + 12.0f;
  note_pot_ += 0.1f * (note - note_pot_);
  p->note = note_pot_ + note_cv_;
  
  DetectNormalization();
  
  for (int32_t i = 0; i < 2; ++i) {
    if (normalization_detector_[i].normalized()) {
      float pot = lp_state_[ADC_LEVEL_1_POT + i];
      p->channel_drive[i] = pot * pot;
    }
  }
  if (normalization_detector_[0].normalized()) {
    p->note = note_pot_ + 24.0f;
  }
  
  adc_.Convert();
}

}  // namespace warps
