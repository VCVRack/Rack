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

#ifndef PLAITS_SETTINGS_H_
#define PLAITS_SETTINGS_H_

#include "stmlib/stmlib.h"
#include "stmlib/system/storage.h"

#include "plaits/drivers/cv_adc.h"

namespace plaits {
  
struct ChannelCalibrationData {
  float offset;
  float scale;
  int16_t normalization_detection_threshold;
  inline float Transform(float x) const {
    return x * scale + offset;
  }
};

struct PersistentData {
  ChannelCalibrationData channel_calibration_data[CV_ADC_CHANNEL_LAST];
  uint8_t padding[16];
  enum { tag = 0x494C4143 };  // CALI
};

struct State {
  uint8_t engine;
  uint8_t lpg_colour;
  uint8_t decay;
  uint8_t octave;
  uint8_t color_blind;
  uint8_t padding[3];
  enum { tag = 0x54415453 };  // STAT
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  bool Init();

  void SavePersistentData();
  void SaveState();
  
  inline const ChannelCalibrationData& calibration_data(int channel) const {
    return persistent_data_.channel_calibration_data[channel];
  }
  
  inline ChannelCalibrationData* mutable_calibration_data(int channel) {
    return &persistent_data_.channel_calibration_data[channel];
  }

  inline const State& state() const {
    return state_;
  }

  inline State* mutable_state() {
    return &state_;
  }
  
 private:
  PersistentData persistent_data_;
  State state_;
  
  stmlib::ChunkStorage<
      0x08004000,
      0x08008000,
      PersistentData,
      State> chunk_storage_;
  
  DISALLOW_COPY_AND_ASSIGN(Settings);
};

}  // namespace plaits

#endif  // PLAITS_SETTINGS_H_
