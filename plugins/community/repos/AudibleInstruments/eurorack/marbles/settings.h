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
// Settings storage.

#ifndef MARBLES_SETTINGS_H_
#define MARBLES_SETTINGS_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/system/storage.h"

#include "marbles/drivers/adc.h"
#include "marbles/drivers/dac.h"
#include "marbles/random/quantizer.h"

namespace marbles {
  
struct CalibrationData {
  float adc_offset[ADC_CHANNEL_LAST];
  float adc_scale[ADC_CHANNEL_LAST];
  float dac_offset[DAC_CHANNEL_LAST];
  float dac_scale[DAC_CHANNEL_LAST];
};

const int kNumScales = 6;

struct PersistentData {
  CalibrationData calibration_data;
  Scale scale[kNumScales];
  
  uint8_t padding[16];
  
  enum { tag = 0x494C4143 };
};

struct State {
  uint8_t t_deja_vu;
  uint8_t t_model;
  uint8_t t_range;
  uint8_t t_pulse_width_mean;
  uint8_t t_pulse_width_std;
  uint8_t x_deja_vu;
  uint8_t x_control_mode;
  uint8_t x_register_mode;
  uint8_t x_range;
  uint8_t x_scale;
  uint8_t y_spread;
  uint8_t y_bias;
  uint8_t y_steps;
  uint8_t y_divider;
  uint8_t y_range;
  
  uint8_t color_blind;
  
  uint8_t padding[8];

  enum { tag = 0x54415453 };
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  void Init();

  void SavePersistentData();
  void SaveState();
  void ResetScale(int i);
  
  static void ProgramOptionBytes();
  
  inline const CalibrationData& calibration_data() {
    return persistent_data_.calibration_data;
  }
  
  inline CalibrationData* mutable_calibration_data() {
    return &persistent_data_.calibration_data;
  }
  
  inline const Scale& scale(int i) const {
    return persistent_data_.scale[i];
  }

  inline Scale* mutable_scale(int i) {
    return &persistent_data_.scale[i];
  }
  
  inline const State& state() const {
    return state_;
  }

  inline State* mutable_state() {
    return &state_;
  }
  
  inline const PersistentData& persistent_data() const {
    return persistent_data_;
  }

  inline bool freshly_baked() const {
    return freshly_baked_;
  }
  
  inline void set_dirty_scale_index(int i) {
    dirty_scale_index_ = i;
  }
  
  inline int dirty_scale_index() const {
    return dirty_scale_index_;
  }
  
 private:
  bool freshly_baked_;
  int dirty_scale_index_;
  
  PersistentData persistent_data_;
  State state_;
  
  stmlib::ChunkStorage<1, PersistentData, State> chunk_storage_;
  
  DISALLOW_COPY_AND_ASSIGN(Settings);
};

}  // namespace marbles

#endif  // MARBLES_SETTINGS_H_
