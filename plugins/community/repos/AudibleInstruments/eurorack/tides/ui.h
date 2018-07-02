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
// Settings

#ifndef TIDES_UI_H_
#define TIDES_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/ui/event_queue.h"

#include "tides/drivers/factory_testing_switch.h"
#include "tides/drivers/leds.h"
#include "tides/drivers/switches.h"

namespace tides {

class Generator;
class CvScaler;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_CALIBRATION_C2,
  UI_MODE_CALIBRATION_C4,
  UI_MODE_PAQUES,
  UI_MODE_FACTORY_TESTING
};

struct Settings {
  uint8_t mode;
  uint8_t range;
  uint8_t sync;
  uint8_t padding;
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Generator* generator, CvScaler* cv_scaler);
  void Poll();
  void DoEvents();
  void FlushEvents();
  void UpdateFactoryTestingFlags(
      uint8_t gate_input_flags,
      const uint16_t* adc_values);
  void set_led_color(uint16_t brightness, bool end_of_attack) {
    if (mode_ == UI_MODE_NORMAL) {
      if (end_of_attack) {
        leds_.set_value(brightness, 0);
      } else {
        leds_.set_value(0, brightness);
      }
    }
  }
  
  inline UiMode mode() const { return mode_; }

 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void UpdateMode();
  void UpdateRange();
  void SaveState();

  stmlib::EventQueue<16> queue_;
  
  bool red_;
  bool green_;
  bool orange_;
  
  Leds leds_;
  Switches switches_;
  FactoryTestingSwitch factory_testing_switch_;
  uint32_t press_time_[kNumSwitches];
  UiMode mode_;
  
  Generator* generator_;
  CvScaler* cv_scaler_;
  
  Settings settings_;
  uint16_t version_token_;
  
  uint8_t mode_counter_;
  uint8_t range_counter_;
  uint8_t long_press_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace tides

#endif  // TIDES_UI_H_
