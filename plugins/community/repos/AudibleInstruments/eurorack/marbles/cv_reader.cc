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

#include "marbles/cv_reader.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"

namespace marbles {
  
using namespace std;
using namespace stmlib;

/* static */
const CvReaderChannel::Settings CvReader::channel_settings_[] = {
  // cv_lp | pot_scale | pot_offset | pot_lp | min | max | hysteresis
  // ADC_CHANNEL_DEJA_VU_AMOUNT,
  { 0.05f, 1.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.00f },
  // ADC_CHANNEL_X_SPREAD_2 / ADC_CHANNEL_DEJA_VU_LENGTH,
  { 0.05f, 1.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.00f },
  // ADC_CHANNEL_T_RATE,
  { 0.2f, 120.0f, -60.0f, 0.01f, -120.0f, 120.0f, 0.001f },
  // ADC_CHANNEL_T_BIAS,
  { 0.05f, 1.05f, -0.025f, 0.01f, 0.0f, 1.0f, 0.00f },
  // ADC_CHANNEL_T_JITTER,
  { 0.05f, 1.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.00f },
  // ADC_CHANNEL_X_SPREAD,
  { 0.1f, 1.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.01f },
  // ADC_CHANNEL_X_BIAS,
  { 0.1f, 1.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.02f },
  // ADC_CHANNEL_X_STEPS,
  { 0.05f, 1.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.02f },
};

void CvReader::Init(CalibrationData* calibration_data) {
  calibration_data_ = calibration_data;
  adc_.Init(false);
  for (int i = 0; i < ADC_CHANNEL_LAST; ++i) {
    channel_[i].Init(
        &calibration_data_->adc_scale[i],
        &calibration_data_->adc_offset[i],
        channel_settings_[i]);
  }
  
  fill(&attenuverter_[0], &attenuverter_[ADC_CHANNEL_LAST], 1.0f);

  // Set virtual attenuverter to 12 o'clock to ignore the non-existing
  // CV input for DEJA VU length.
  attenuverter_[ADC_CHANNEL_DEJA_VU_LENGTH] = 0.5f;

  // Set virtual attenuverter to a little more than 100% to
  // compensate for op-amp clipping and get full parameter swing.
  attenuverter_[ADC_CHANNEL_DEJA_VU_AMOUNT] = 1.01f;
  attenuverter_[ADC_CHANNEL_T_BIAS] = 1.01f;
  attenuverter_[ADC_CHANNEL_T_JITTER] = 1.01f;
  attenuverter_[ADC_CHANNEL_X_SPREAD] = 1.01f;
  attenuverter_[ADC_CHANNEL_X_BIAS] = 1.01f;
  attenuverter_[ADC_CHANNEL_X_STEPS] = 1.01f;
}

void CvReader::Copy(uint16_t* output) {
  const uint16_t* adc_values = adc_.values();
  copy(&adc_values[0], &adc_values[ADC_CHANNEL_LAST * 2], output);
  adc_.Convert();
}

void CvReader::Process(const uint16_t* raw_values, float* output) {
  for (int i = 0; i < ADC_CHANNEL_LAST; ++i) {
    output[i] = channel_[i].Process(
        static_cast<float>(raw_values[ADC_GROUP_POT + i]) / 65536.0f,
        static_cast<float>(raw_values[ADC_GROUP_CV + i]) / 65536.0f,
        attenuverter_[i]);
  }
}

}  // namespace marbles
