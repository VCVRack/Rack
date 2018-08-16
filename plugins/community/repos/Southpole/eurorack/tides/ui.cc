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
// User interface.

#include "tides/ui.h"

#include "stmlib/system/storage.h"
#include "stmlib/system/system_clock.h"

#include "tides/cv_scaler.h"
#include "tides/generator.h"

namespace tides {

const int32_t kLongPressDuration = 1000;

using namespace stmlib;

Storage<0x801fc00, 4> mode_storage;

void Ui::Init(Generator* generator, CvScaler* cv_scaler) {
  factory_testing_switch_.Init();
  leds_.Init();
  switches_.Init();
  mode_ = factory_testing_switch_.Read()
      ? UI_MODE_NORMAL
      : UI_MODE_FACTORY_TESTING;
  
  generator_ = generator;
  cv_scaler_ = cv_scaler;
  
  if (!mode_storage.ParsimoniousLoad(&settings_, &version_token_)) {
    mode_counter_ = 1;
    range_counter_ = 2;
    generator->set_sync(false);
  } else {
    mode_counter_ = settings_.mode;
    range_counter_ = 2 - settings_.range;
    generator->set_sync(settings_.sync);
  }

  UpdateMode();
  UpdateRange();
  leds_.set_value(32768, 32768);
}

void Ui::SaveState() {
  settings_.mode = generator_->mode();
  settings_.range = generator_->range();
  settings_.sync = generator_->sync();
  mode_storage.ParsimoniousSave(settings_, &version_token_);
}

const uint16_t thresholds[ADC_CHANNEL_LAST][2] = {
  { 0, 64000 },
  { 16384, 49152 },
  { 16384, 49152 },
  { 4096, 61440 },
  { 4096, 61440 },
  { 4096, 61440 },
  { 4096, 61440 },
};

void Ui::UpdateFactoryTestingFlags(
    uint8_t gate_input_flags,
    const uint16_t* adc_values) {
  red_ = switches_.pressed(1);
  green_ = switches_.pressed(0);
  orange_ = gate_input_flags & CONTROL_GATE;
  orange_ |= gate_input_flags & CONTROL_FREEZE;
  orange_ |= gate_input_flags & CONTROL_CLOCK;
  
  for (uint16_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
    uint16_t value = adc_values[i];
    if (i == ADC_CHANNEL_FM_ATTENUVERTER) {
      value = ~value;
    }
    green_ |= value < thresholds[i][0];
    if (i == 0) {
      orange_ |= value > thresholds[i][1];
    } else {
      red_ |= value > thresholds[i][1];
    }
  }
}

void Ui::Poll() {
  system_clock.Tick();
  switches_.Debounce();
  
  for (uint8_t i = 0; i < kNumSwitches; ++i) {
    if (switches_.just_pressed(i)) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
    }
    if (switches_.pressed(i) && press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];
      if (pressed_time > kLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        press_time_[i] = 0;
      }
    }
    if (switches_.released(i) && press_time_[i] != 0) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
      press_time_[i] = 0;
    }
  }
  
  switch (mode_) {
    case UI_MODE_NORMAL:
      {
        GeneratorMode mode = generator_->mode();
        leds_.set_mode(mode == GENERATOR_MODE_AR, mode == GENERATOR_MODE_AD);

        GeneratorRange range = generator_->range();
        switch (range) {
          case GENERATOR_RANGE_LOW:
            leds_.set_rate(0, 65535);
            break;
          case GENERATOR_RANGE_MEDIUM:
            if (generator_->sync()) {
              leds_.set_rate(8192, 16384);
            } else {
              leds_.set_rate(0);
            }
            break;
          case GENERATOR_RANGE_HIGH:
            leds_.set_rate(65535, 0);
            break;
        }
        bool blink_rate_led = generator_->sync() && \
            system_clock.milliseconds() & 128;
        if (blink_rate_led) {
          leds_.set_rate(0);
        }
      }
      break;
      
    case UI_MODE_CALIBRATION_C2:
      leds_.set_mode(true);
      leds_.set_rate(65535);
      leds_.set_value(65535);
      break;
      
    case UI_MODE_CALIBRATION_C4:
      leds_.set_mode(false, true);
      leds_.set_rate(0, 65535);
      leds_.set_value(0, 65535);
      break;
      
    case UI_MODE_PAQUES:
      leds_.set_mode(true, false);
      leds_.set_rate(65535, 0);
      leds_.set_value(65535, 0);
      break;
      
    case UI_MODE_FACTORY_TESTING:
      if (orange_) {
        leds_.set_mode(true, true);
        leds_.set_rate(65535, 65535);
        leds_.set_value(65535, 65535);
      } else if (red_) {
        leds_.set_mode(true, 0);
        leds_.set_rate(65535, 0);
        leds_.set_value(65535, 0);
      } else if (green_) {
        leds_.set_mode(false, true);
        leds_.set_rate(0, 65535);
        leds_.set_value(0, 65535);
      } else {
        leds_.set_mode(false, false);
        leds_.set_rate(0, 0);
        leds_.set_value(0, 0);
      }
      break;
  }
  
  leds_.Write();
}

inline void Ui::UpdateMode() {
  uint8_t i = mode_counter_ & 3;
  if (i == 3) {
    i = 1;
  }
  generator_->set_mode(static_cast<GeneratorMode>(i));
  SaveState();
}

inline void Ui::UpdateRange() {
  uint8_t i = range_counter_ & 3;
  if (i == 3) {
    i = 1;
  }
  generator_->set_range(static_cast<GeneratorRange>(2 - i));
  SaveState();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {
  switch (e.control_id) {
    case 0:
      break;
      
    case 1:
      break;
  }
}

void Ui::OnSwitchReleased(const Event& e) {
  if (mode_ == UI_MODE_FACTORY_TESTING) {
    return;
  } else if (mode_ == UI_MODE_PAQUES) {
    mode_ = UI_MODE_NORMAL;
  } else if (mode_ == UI_MODE_CALIBRATION_C2) {
    if (e.data > kLongPressDuration) {
      ++long_press_counter_;
    }
    if (e.control_id == 0) {
      cv_scaler_->CaptureCalibrationValues();
      mode_ = UI_MODE_CALIBRATION_C4;
    } else {
      mode_ = UI_MODE_NORMAL;
    }
  } else if (mode_ == UI_MODE_CALIBRATION_C4) {
    mode_ = UI_MODE_NORMAL;
    if (e.control_id == 0) {
      cv_scaler_->Calibrate();
    } else if (e.control_id == 1 && long_press_counter_ == 1 &&
               e.data > kLongPressDuration) {
      mode_ = UI_MODE_PAQUES;
    }
  } else {
    long_press_counter_ = 0;
    switch (e.control_id) {
      case 0:
        if (e.data > kLongPressDuration &&
            cv_scaler_->can_enter_calibration()) {
          mode_ = UI_MODE_CALIBRATION_C2;
        } else {
          ++mode_counter_;
          UpdateMode();
        }
        break;

      case 1:
        if (e.data > kLongPressDuration) {
          generator_->set_sync(!generator_->sync());
          SaveState();
        } else {
          ++range_counter_;
          UpdateRange();
        }
        break;
    }
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
  if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }
}

}  // namespace tides
