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
const uint8_t Ui::feature_mode_palette_[10][3] = {
  { 255, 64, 0 },
  { 0, 192, 64 },
  { 255, 0, 64 },
  { 0, 255, 192 },
  { 64, 255, 0 },
  { 0, 0, 255 },
  { 255, 255, 0 },
  { 255, 0, 255 },
  { 0, 255, 192 },
  { 255, 0, 0 },
};

/* static */
const uint8_t Ui::freq_shifter_palette_[10][3] = {
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

const float kAlgoChangeThreshold = 0.01f;
  
void Ui::Init(Settings* settings, CvScaler* cv_scaler, Modulator* modulator) {
  leds_.Init();
  switches_.Init();

  mode_ = UI_MODE_NORMAL;
  settings_ = settings;
  cv_scaler_ = cv_scaler;
  modulator_ = modulator;
  
  modulator_->set_feature_mode(static_cast<FeatureMode>(settings_->state().feature_mode));
  feature_mode_ = modulator_->feature_mode();
  carrier_shape_ = settings_->state().carrier_shape;
  UpdateSettings();

  last_algo_pot_ = 0.0f;
  feature_mode_changed_ = false;
}

void Ui::UpdateSettings() {
  modulator_->set_feature_mode(static_cast<FeatureMode>(feature_mode_));
  settings_->mutable_state()->feature_mode = feature_mode_;
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
  
  if (switches_.pressed(0) && \
      press_time_ &&
      (system_clock.milliseconds() - press_time_) >= 7800) {
    if (!feature_mode_changed_ && cv_scaler_->ready_for_calibration()) {
      queue_.AddEvent(CONTROL_SWITCH, 1, 0);
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

        switch (modulator_->feature_mode()) {
        case FEATURE_MODE_META:
          zone = p.modulation_algorithm;
          palette = palette_;
          break;

        default:
          zone = p.raw_algorithm;
          palette = freq_shifter_palette_;
          break;
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

    case UI_MODE_FEATURE_SWITCH:
      {
        const Parameters& p = modulator_->parameters();
	if (p.raw_algorithm_pot >= last_algo_pot_ + kAlgoChangeThreshold ||
	    p.raw_algorithm_pot <= last_algo_pot_ - kAlgoChangeThreshold) {
	  feature_mode_changed_ = true;
	}

	if (feature_mode_changed_) {
	  feature_mode_ = static_cast<uint8_t>(p.raw_algorithm_pot * 8.0f + 0.5f);
	  int8_t ramp = system_clock.milliseconds() & 127;
	  uint8_t tri = (system_clock.milliseconds() & 255) < 128 ?
	    127 + ramp : 255 - ramp;
	  leds_.set_main((feature_mode_palette_[feature_mode_][0] * tri) >> 8,
			 (feature_mode_palette_[feature_mode_][1] * tri) >> 8,
			 (feature_mode_palette_[feature_mode_][2] * tri) >> 8);
	}
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

    case UI_MODE_CALIBRATION_ERROR:
    case UI_MODE_PANIC:
      leds_.set_osc(blink ? 255 : 0, 0);
      leds_.set_main(blink ? 255 : 0, 0, 0);
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
        case UI_MODE_NORMAL:
          last_algo_pot_ = modulator_->parameters().raw_algorithm_pot;
          mode_ = UI_MODE_FEATURE_SWITCH;
          break;
        default:
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
   switch (e.control_id) {
     case 0:
       if (mode_ == UI_MODE_CALIBRATION_C1 ||
           mode_ == UI_MODE_CALIBRATION_C3 ||
           mode_ == UI_MODE_CALIBRATION_LOW ||
           mode_ == UI_MODE_CALIBRATION_HIGH) {
         CalibrateC1();
       } else if (mode_ == UI_MODE_CALIBRATION_C3) {
         CalibrateC3();
       } else {
         mode_ = UI_MODE_NORMAL;
         if (feature_mode_changed_) {
           feature_mode_changed_ = false;
         }
         else {
           carrier_shape_ = (carrier_shape_ + 1) & 3;
         }
         UpdateSettings();
         settings_->Save();
       }
   }
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
  if (mode_ == UI_MODE_CALIBRATION_ERROR) {
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
            UpdateSettings();
            break;
        }
      }
      break;
  }
  return reply;
}

}  // namespace warps
