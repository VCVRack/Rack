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

#include "warps/ui.h"

#include <algorithm>

#include "stmlib/system/system_clock.h"
#include "stmlib/dsp/units.h"

#include "warps/cv_scaler.h"

namespace warps {

using namespace std;
using namespace stmlib;

/* static */
const uint8_t Ui::palette_[10][3] = {
  { 0, 192, 64 },
  { 64, 255, 0 },
  { 255, 255, 0 },
  { 255, 64, 0 },
  { 255, 0, 0 },
  { 255, 0, 64 },
  { 255, 0, 255 },
  { 0, 0, 255 },
  { 0, 255, 192 },
  { 0, 255, 192 },
};

/* static */
const uint8_t Ui::easter_egg_palette_[10][3] = {
  { 0, 0, 64 },
  { 0, 0, 255 },
  { 0, 255, 192 },
  { 0, 192, 64 },
  { 64, 255, 0 },
  { 255, 255, 0 },
  { 255, 192, 0 },
  { 255, 64, 0 },
  { 255, 0, 0 },
  { 255, 0, 0 },
};

void Ui::Init(Settings* settings, CvScaler* cv_scaler, Modulator* modulator) {
  leds_.Init();
  switches_.Init();

  mode_ = UI_MODE_NORMAL;
  settings_ = settings;
  cv_scaler_ = cv_scaler;
  modulator_ = modulator;
  
  modulator_->set_easter_egg(settings_->state().boot_in_easter_egg_mode);
  carrier_shape_ = settings_->state().carrier_shape;
  UpdateCarrierShape();
}

void Ui::UpdateCarrierShape() {
  modulator_->mutable_parameters()->carrier_shape = carrier_shape_;
  settings_->mutable_state()->carrier_shape = carrier_shape_;
}

void Ui::Poll() {
  // Called at 1.6kHz instead of 1kHz, so the "milliseconds" clock actually runs
  // 1.6x faster. Not a big deal since it is used only for controlling LED
  // blinking rate and detecting long button presses.
  system_clock.Tick();
  switches_.Debounce();
  if (switches_.just_pressed(0)) {
    queue_.AddEvent(CONTROL_SWITCH, 0, 0);
    press_time_ = system_clock.milliseconds();
  }
  
  if (switches_.pressed(0) && press_time_) {
    if (cv_scaler_->ready_for_calibration() && (system_clock.milliseconds() - press_time_) >= 4800) {
      queue_.AddEvent(CONTROL_SWITCH, 1, 0);
      press_time_ = 0;
    } else if ((system_clock.milliseconds() - press_time_) >= 9600) {
      queue_.AddEvent(CONTROL_SWITCH, 2, 0);
      press_time_ = 0;
    }
  }
  
  if (switches_.released(0) && press_time_) {
    queue_.AddEvent(
        CONTROL_SWITCH,
        0,
        system_clock.milliseconds() - press_time_ + 1);
  }
  
  bool blink = (system_clock.milliseconds() & 127) > 64;
  bool slow_blink = (system_clock.milliseconds() & 255) > 128;
  switch (mode_) {
    case UI_MODE_NORMAL:
      {
        uint8_t rgb[3];
        float zone;
        const Parameters& p = modulator_->parameters();
        const uint8_t (*palette)[3];
        if (modulator_->easter_egg()) {
          zone = p.phase_shift;
          palette = easter_egg_palette_;
        } else {
          zone = p.modulation_algorithm;
          palette = palette_;
        }
        zone *= 8.0f;
        MAKE_INTEGRAL_FRACTIONAL(zone);
        int32_t zone_fractional_i = static_cast<int32_t>(
            zone_fractional * 256.0f);
        for (int32_t i = 0; i < 3; ++i) {
          int32_t a = palette[zone_integral][i];
          int32_t b = palette[zone_integral + 1][i];
          rgb[i] = a + ((b - a) * zone_fractional_i >> 8);
        }
        leds_.set_main(rgb[0], rgb[1], rgb[2]);
        leds_.set_osc(
            carrier_shape_ >= 2 ? 255 : 0,
            carrier_shape_ > 0 && carrier_shape_ <= 2 ? 255 : 0);
      }
      break;
    
    case UI_MODE_CALIBRATION_C1:
      leds_.set_main(0, blink ? 255 : 0, blink ? 64 : 0);
      leds_.set_osc(blink ? 255 : 0, blink ? 255 : 0);
      break;

    case UI_MODE_CALIBRATION_C3:
      leds_.set_main(blink ? 255 : 0, 0, blink ? 32 : 0);
      leds_.set_osc(blink ? 255 : 0, 0);
      break;

    case UI_MODE_CALIBRATION_LOW:
      leds_.set_main(slow_blink ? 255 : 0, 0, 0);
      leds_.set_osc(slow_blink ? 255 : 0, 0);
      break;

    case UI_MODE_CALIBRATION_HIGH:
      leds_.set_main(0, slow_blink ? 255 : 0, 0);
      leds_.set_osc(0, slow_blink ? 255 : 0);
      break;
    
    case UI_MODE_PANIC:
    case UI_MODE_CALIBRATION_ERROR:
      leds_.set_osc(blink ? 255 : 0, 0);
      leds_.set_main(blink ? 255 : 0, 0, 0);
      break;
    
    case UI_MODE_EASTER_EGG_DANCE:
      {
        leds_.set_osc(0, blink ? 255 : 0);
        uint8_t color = (system_clock.milliseconds() >> 9) % 9;
        leds_.set_main(
            easter_egg_palette_[color][0],
            easter_egg_palette_[color][1],
            easter_egg_palette_[color][2]);
      }
      break;
  }
  
  if (modulator_->bypass()) {
    uint16_t red = system_clock.milliseconds() & 4095;
    uint16_t green = (system_clock.milliseconds() + 1333) & 4095;
    uint16_t blue = (system_clock.milliseconds() + 2667) & 4095;
    green = green < 2048 ? green : 4095 - green;
    red = red < 2048 ? red : 4095 - red;
    blue = blue < 2048 ? blue : 4095 - blue;
    leds_.set_osc(255, 255);
    leds_.set_main(red >> 3, green >> 3, blue >> 3);
  }
  leds_.Write();
}

bool Ui::DetectSecretHandshake() {
  for (int32_t i = 0; i < 5; ++i) {
    secret_handshake_[i] = secret_handshake_[i + 1];
  }
  secret_handshake_[5] = cv_scaler_->easter_egg_digit();
  uint8_t expected[6] = { 2, 4, 3, 6, 1, 5 };
  return equal(
      &secret_handshake_[0],
      &secret_handshake_[6],
      &expected[0]);
}

void Ui::OnSwitchPressed(const Event& e) {
  switch (e.control_id) {
    case 0:
      switch (mode_) {
        case UI_MODE_CALIBRATION_C1:
          CalibrateC1();
          break;
        case UI_MODE_CALIBRATION_C3:
          CalibrateC3();
          break;
        case UI_MODE_CALIBRATION_LOW:
          CalibrateLow();
          break;
        case UI_MODE_CALIBRATION_HIGH:
          CalibrateHigh();
          break;
        default:
          if (!DetectSecretHandshake()) {
            carrier_shape_ = (carrier_shape_ + 1) & 3;
          } else {
            bool easter = !modulator_->easter_egg();
            modulator_->set_easter_egg(easter);
            settings_->mutable_state()->boot_in_easter_egg_mode = easter;
            carrier_shape_ = 1;
            mode_ = UI_MODE_EASTER_EGG_DANCE;
          }
          UpdateCarrierShape();
          settings_->Save();
          break;
      }
      break;
    
    case 1:
      StartCalibration();
      break;
      
    case 2:
      StartNormalizationCalibration();
      break;
    
    default:
      break;
  }
}

void Ui::OnSwitchReleased(const Event& e) {
  
}

void Ui::StartCalibration() {
  cv_scaler_->StartCalibration();
  mode_ = UI_MODE_CALIBRATION_C1;
}

void Ui::CalibrateC1() {
  cv_scaler_->CalibrateC1();
  cv_scaler_->CalibrateOffsets();
  mode_ = UI_MODE_CALIBRATION_C3;
}

void Ui::CalibrateC3() {
  if (cv_scaler_->CalibrateC3()) {
    settings_->Save();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_CALIBRATION_ERROR;
  }
}

void Ui::StartNormalizationCalibration() {
  cv_scaler_->StartNormalizationCalibration();
  mode_ = UI_MODE_CALIBRATION_LOW;
}

void Ui::CalibrateLow() {
  cv_scaler_->CalibrateLow();
  mode_ = UI_MODE_CALIBRATION_HIGH;
}

void Ui::CalibrateHigh() {
  if (cv_scaler_->CalibrateHigh()) {
    settings_->Save();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_CALIBRATION_ERROR;
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
  if (mode_ == UI_MODE_EASTER_EGG_DANCE || mode_ == UI_MODE_CALIBRATION_ERROR) {
    if (queue_.idle_time() > 6000) {
      mode_ = UI_MODE_NORMAL;
    }
  } else {
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
    case FACTORY_TESTING_READ_CV:
      reply = cv_scaler_->adc_value(argument);
      break;
      
    case FACTORY_TESTING_READ_NORMALIZATION:
      reply = cv_scaler_->normalization(argument);
      break;
      
    case FACTORY_TESTING_READ_GATE:
      return switches_.pressed(argument);
      break;
      
    case FACTORY_TESTING_SET_BYPASS:
      modulator_->set_bypass(argument);
      break;
      
    case FACTORY_TESTING_CALIBRATE:
      {
        switch (argument) {
          case 0:
            StartCalibration();
            break;
            
          case 1:
            CalibrateC1();
            break;
            
          case 2:
            CalibrateC3();
            break;
            
          case 3:
            StartNormalizationCalibration();
            break;

          case 4:
            CalibrateLow();
            break;
            
          case 5:
            CalibrateHigh();
            carrier_shape_ = 0;
            UpdateCarrierShape();
            break;
        }
      }
      break;
  }
  return reply;
}

}  // namespace warps
