// Copyright 2016 Olivier Gillet.
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
// Settings storage.

#include "plaits/settings.h"

#include <algorithm>

#include "stmlib/system/storage.h"

namespace plaits {

using namespace std;

bool Settings::Init() {
  ChannelCalibrationData* c = persistent_data_.channel_calibration_data;
  c[CV_ADC_CHANNEL_MODEL].offset = 0.025f;
  c[CV_ADC_CHANNEL_MODEL].scale = -1.03f;
  c[CV_ADC_CHANNEL_MODEL].normalization_detection_threshold = 0;
  
  c[CV_ADC_CHANNEL_V_OCT].offset = 25.71;
  c[CV_ADC_CHANNEL_V_OCT].scale = -60.0f;
  c[CV_ADC_CHANNEL_V_OCT].normalization_detection_threshold = 0;

  c[CV_ADC_CHANNEL_FM].offset = 0.0f;
  c[CV_ADC_CHANNEL_FM].scale = -60.0f;
  c[CV_ADC_CHANNEL_FM].normalization_detection_threshold = -2945;

  c[CV_ADC_CHANNEL_HARMONICS].offset = 0.0f;
  c[CV_ADC_CHANNEL_HARMONICS].scale = -1.0f;
  c[CV_ADC_CHANNEL_HARMONICS].normalization_detection_threshold = 0;

  c[CV_ADC_CHANNEL_TIMBRE].offset = 0.0f;
  c[CV_ADC_CHANNEL_TIMBRE].scale = -1.6f;
  c[CV_ADC_CHANNEL_TIMBRE].normalization_detection_threshold = -2945;

  c[CV_ADC_CHANNEL_MORPH].offset = 0.0f;
  c[CV_ADC_CHANNEL_MORPH].scale = -1.6f;
  c[CV_ADC_CHANNEL_MORPH].normalization_detection_threshold = -2945;

  c[CV_ADC_CHANNEL_TRIGGER].offset = 0.4f;
  c[CV_ADC_CHANNEL_TRIGGER].scale = -0.6f;
  c[CV_ADC_CHANNEL_TRIGGER].normalization_detection_threshold = 13663;

  c[CV_ADC_CHANNEL_LEVEL].offset = 0.49f;
  c[CV_ADC_CHANNEL_LEVEL].scale = -0.6f;
  c[CV_ADC_CHANNEL_LEVEL].normalization_detection_threshold = 21403;
  
  state_.engine = 0;
  state_.lpg_colour = 0;
  state_.decay = 128;
  state_.octave = 255;
  state_.color_blind = 0;
  
  bool success = chunk_storage_.Init(&persistent_data_, &state_);
  
  CONSTRAIN(state_.engine, 0, 15);

  return success;
}

void Settings::SavePersistentData() {
  chunk_storage_.SavePersistentData();
}

void Settings::SaveState() {
  chunk_storage_.SaveState();
}

}  // namespace plaits
