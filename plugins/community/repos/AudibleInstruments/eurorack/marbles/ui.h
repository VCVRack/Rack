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
// User interface.

#ifndef MARBLES_UI_H_
#define MARBLES_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/ui/event_queue.h"

#include "marbles/drivers/adc.h"
#include "marbles/drivers/leds.h"
#include "marbles/drivers/switches.h"

namespace marbles {

class ClockInputs;
class CvReader;
class ScaleRecorder;
class Settings;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_SELECT_SCALE,
  UI_MODE_RECORD_SCALE,
  UI_MODE_CALIBRATION_1,
  UI_MODE_CALIBRATION_2,
  UI_MODE_CALIBRATION_3,
  UI_MODE_CALIBRATION_4,
  UI_MODE_PANIC,
};

enum FactoryTestingCommand {
  FACTORY_TESTING_READ_POT,
  FACTORY_TESTING_READ_CV,
  FACTORY_TESTING_READ_GATE,
  FACTORY_TESTING_GENERATE_TEST_SIGNALS,
  FACTORY_TESTING_CALIBRATE,
  FACTORY_TESTING_READ_NORMALIZATION,
  FACTORY_TESTING_FORCE_DAC_CODE,
  FACTORY_TESTING_WRITE_CALIBRATION_DATA_NIBBLE,
};

struct AlternateKnobMapping {
  Switch unlock_switch;
  uint8_t* destination;
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(
      Settings* settings,
      CvReader* cv_reader,
      ScaleRecorder* scale_recorder,
      ClockInputs* clock_inputs);
  void Poll();
  void DoEvents();
  void FlushEvents();
  uint8_t HandleFactoryTestingRequest(uint8_t command);
  
  bool recording_scale() const {
    return mode_ == UI_MODE_RECORD_SCALE;
  }
  
  bool output_test_mode() const {
    return output_test_mode_;
  }
  
  uint16_t output_test_forced_dac_code(int i) const {
    return output_test_forced_dac_code_[i];
  }
  
  void set_deja_vu_lock(bool deja_vu_lock) {
    deja_vu_lock_ = deja_vu_lock;
  }
  
 private:
  void UpdateLEDs();
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void NextCalibrationStep();
  void SaveState();
  void UpdateHiddenParameters();
  void TerminateScaleRecording();
  static LedColor MakeColor(uint8_t value, bool color_blind);
  
  stmlib::EventQueue<16> queue_;
  
  Leds leds_;
  Switches switches_;
  uint32_t press_time_[SWITCH_LAST];
  bool ignore_release_[SWITCH_LAST];
  
  UiMode mode_;
  
  Settings* settings_;
  CvReader* cv_reader_;
  ScaleRecorder* scale_recorder_;
  ClockInputs* clock_inputs_;
  
  static const LedColor palette_[4];
  static AlternateKnobMapping alternate_knob_mappings_[ADC_CHANNEL_LAST];
  
  float pot_value_[ADC_CHANNEL_LAST];
  
  bool setting_modification_flag_;
  bool deja_vu_lock_;
  
  bool output_test_mode_;
  uint16_t output_test_forced_dac_code_[4];
  uint32_t calibration_data_;
  
  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace marbles

#endif  // MARBLES_UI_H_
