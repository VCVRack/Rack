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
// Settings storage.

#ifndef CLOUDS_SETTINGS_H_
#define CLOUDS_SETTINGS_H_

#include "stmlib/stmlib.h"

#include "clouds/drivers/adc.h"

namespace clouds {

struct CalibrationData {
  float pitch_offset;
  float pitch_scale;
  float offset[ADC_CHANNEL_LAST];
};

struct State {
  uint8_t quality;
  uint8_t blend_parameter;
  uint8_t playback_mode;
  uint8_t blend_value[4];
  uint8_t padding;
};

struct SettingsData {
  CalibrationData calibration_data; // 48 bytes
  State state;  // 8 bytes
  uint8_t padding[8];
};

class PersistentBlock;

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  void Init();
  void Save();
  
  inline const uint32_t* sample_flash_data(uint32_t index) const {
    return (const uint32_t*)(mutable_sample_flash_data(index));
  }

  inline uint32_t* mutable_sample_flash_data(uint32_t index) const {
    return (uint32_t*)(0x08080000 + index * 0x0020000);
  }

  inline uint32_t sample_flash_sector(uint32_t index) {
    return index + 8;
  }
  
  void SaveSampleMemory(
      uint32_t index,
      PersistentBlock* blocks,
      size_t num_blocks);
  
  inline CalibrationData* mutable_calibration_data() {
    return &data_.calibration_data;
  }
  
  inline State* mutable_state() {
    return &data_.state;
  }

  inline const State& state() const {
    return data_.state;
  }
  
  // True when no calibration data has been found on flash sector 1, that is
  // to say when the module has just been flashed.
  inline bool freshly_baked() const {
    return freshly_baked_;
  }
  
 private:
  bool freshly_baked_;
  SettingsData data_;
  uint16_t version_token_;
  
  DISALLOW_COPY_AND_ASSIGN(Settings);
};

}  // namespace clouds

#endif  // CLOUDS_SETTINGS_H_
