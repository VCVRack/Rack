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
// User interface.

#include "elements/ui.h"

#include <algorithm>

#include "stmlib/system/system_clock.h"

#include "elements/cv_scaler.h"
#include "elements/dsp/part.h"

namespace elements {

using namespace std;
using namespace stmlib;

void Ui::Init(Part* part, CvScaler* cv_scaler) {
  leds_.Init();
  switch_.Init();
  mode_ = UI_MODE_NORMAL;  // UI_MODE_DISPLAY_MODEL;
  part_ = part;
  cv_scaler_ = cv_scaler;
  part_->set_easter_egg(cv_scaler_->boot_in_easter_egg_mode());
  part_->set_resonator_model(ResonatorModel(cv_scaler_->resonator_model()));
}

void Ui::Poll() {
  // 1kHz.
  system_clock.Tick();
  switch_.Debounce();
  if (switch_.just_pressed()) {
    queue_.AddEvent(CONTROL_SWITCH, 0, 0);
    press_time_ = system_clock.milliseconds();
  }
  
  if (switch_.pressed() && \
      press_time_ &&
      (system_clock.milliseconds() - press_time_) >= 3000) {
    if (cv_scaler_->ready_for_calibration()) {
      queue_.AddEvent(CONTROL_SWITCH, 1, 0);
      press_time_ = 0;
    } else if (cv_scaler_->resonator_high()) {
      if (cv_scaler_->exciter_low()) {
        queue_.AddEvent(CONTROL_SWITCH, 2, 0);
      } else {
        queue_.AddEvent(CONTROL_SWITCH, 3, 0);
      }
      press_time_ = 0;
    }
  }
  
  if (switch_.released() && press_time_) {
    queue_.AddEvent(
        CONTROL_SWITCH,
        0,
        system_clock.milliseconds() - press_time_ + 1);
  }
  
  bool blink = (system_clock.milliseconds() & 127) > 64;
  switch (mode_) {
    case UI_MODE_NORMAL:
      leds_.set_gate(part_->gate());
      leds_.set_exciter(
          lut_db_led_brightness[int32_t(part_->exciter_level() * 512.0f)]);
      leds_.set_resonator(
          lut_db_led_brightness[int32_t(part_->resonator_level() * 512.0f)]);
      break;
    
    case UI_MODE_CALIBRATION_1:
      leds_.set_gate(!blink);
      leds_.set_exciter(blink ? 255 : 0);
      leds_.set_resonator(0);
      break;

    case UI_MODE_CALIBRATION_2:
      leds_.set_gate(!blink);
      leds_.set_exciter(0);
      leds_.set_resonator(blink ? 255 : 0);
      break;
    
    case UI_MODE_PANIC:
      leds_.set_gate(blink);
      leds_.set_exciter(blink ? 255 : 0);
      leds_.set_resonator(blink ? 0 : 255);
      break;
      
    case UI_MODE_DISPLAY_MODEL:
      {
        bool blink = (system_clock.milliseconds() & 255) > 128;
        uint8_t count = ((system_clock.milliseconds()) >> 8) & 3;
        bool pulse = (count <= part_->resonator_model()) && blink;
        leds_.set_gate(pulse);
        leds_.set_exciter(pulse ? 255 : 0);
        leds_.set_resonator(pulse ? 255 : 0);
      }
      break;
  }
  
  if (part_->bypass()) {
    leds_.set_gate(true);
    leds_.set_exciter(255);
    leds_.set_resonator(255);
  }
  
  leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {
  switch (e.control_id) {
    case 0:
      if (mode_ == UI_MODE_CALIBRATION_1) {
        CalibrateC1();
      } else if (mode_ == UI_MODE_CALIBRATION_2) {
        CalibrateC3();
      } else {
        gate_ = true;
      }
      break;
    
    case 1:
      mode_ = UI_MODE_CALIBRATION_1;
      break;
    
    case 2:
      part_->set_easter_egg(!part_->easter_egg());
      cv_scaler_->set_boot_in_easter_egg_mode(part_->easter_egg());
      cv_scaler_->SaveCalibration();
      gate_ = false;
      break;
    
    case 3:
      part_->set_resonator_model(
          ResonatorModel((part_->resonator_model() + 1) % 3));
      cv_scaler_->set_resonator_model(part_->resonator_model());
      cv_scaler_->SaveCalibration();
      gate_ = false;
      mode_ = UI_MODE_DISPLAY_MODEL;
      break;
    
    default:
      break;
  }
}

void Ui::OnSwitchReleased(const Event& e) {
  gate_ = false;
}

void Ui::CalibrateC1() {
  cv_scaler_->CalibrateC1();
  cv_scaler_->CalibrateOffsets();
  mode_ = UI_MODE_CALIBRATION_2;
}

void Ui::CalibrateC3() {
  bool success = cv_scaler_->CalibrateC3();
  if (success) {
    cv_scaler_->SaveCalibration();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_PANIC;
  }
}

void Ui::DoEvents() {
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_SWITCH) {
      if (e.data == 0) {
        OnSwitchPressed(e);
      } else {
        OnSwitchReleased(e);
      }
    }
  }
  if (mode_ == UI_MODE_DISPLAY_MODEL) {
    if (queue_.idle_time() > 4000) {
      mode_ = UI_MODE_NORMAL;
      queue_.Touch();
    }
  } else {
    if (queue_.idle_time() > 800 && mode_ == UI_MODE_PANIC) {
      mode_ = UI_MODE_NORMAL;
    }
    if (queue_.idle_time() > 1000) {
      queue_.Touch();
    }
  }
}

uint8_t Ui::HandleFactoryTestingRequest(uint8_t command) {
  uint8_t argument = command & 0x1f;
  command = command >> 5;
  uint8_t reply = 0;
  switch (command) {
    case FACTORY_TESTING_READ_POT:
      reply = cv_scaler_->pot_value(argument);
      break;
      
    case FACTORY_TESTING_READ_CV:
      reply = cv_scaler_->cv_value(argument);
      break;
      
    case FACTORY_TESTING_READ_GATE:
      if (argument == 0x00) {
        return gate_;
      } else {
        return cv_scaler_->gate();
      }
      break;
      
    case FACTORY_TESTING_SET_BYPASS:
      part_->set_bypass(argument);
      break;
      
    case FACTORY_TESTING_CALIBRATE:
      if (argument == 0) {
        mode_ = UI_MODE_CALIBRATION_1;
      } else if (argument == 1) {
        CalibrateC1();
      } else {
        CalibrateC3();
        
      }
      break;
  }
  return reply;
}

}  // namespace elements
