// Copyright 2012 Olivier Gillet.
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

#include "braids/ui.h"

#include <cstring>

#include "stmlib/system/system_clock.h"

namespace braids {

using namespace stmlib;

const uint32_t kEncoderLongPressTime = 800;

void Ui::Init() {
  encoder_.Init();
  display_.Init();
  queue_.Init();
  sub_clock_ = 0;
  value_ = 0;
  mode_ = MODE_SPLASH;
  setting_ = SETTING_OSCILLATOR_SHAPE;
  setting_index_ = 0;
}

void Ui::Poll() {
  system_clock.Tick();  // Tick global ms counter.
  ++sub_clock_;
  encoder_.Debounce();
  if (encoder_.just_pressed()) {
    encoder_press_time_ = system_clock.milliseconds();
    inhibit_further_switch_events_ = false;
  } 
  if (!inhibit_further_switch_events_) {
    if (encoder_.pressed()) {
      uint32_t duration = system_clock.milliseconds() - encoder_press_time_;
      if (duration >= kEncoderLongPressTime) {
        queue_.AddEvent(CONTROL_ENCODER_LONG_CLICK, 0, 0);
        inhibit_further_switch_events_ = true;
      }
    } else if (encoder_.released()) {
      queue_.AddEvent(CONTROL_ENCODER_CLICK, 0, 0);
    }
  }
  int32_t increment = encoder_.increment();
  if (increment != 0) {
    queue_.AddEvent(CONTROL_ENCODER, 0, increment);
  }
  
  if ((sub_clock_ & 1) == 0) {
    display_.Refresh();
  }
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::RefreshDisplay() {
  switch (mode_) {
    case MODE_SPLASH:
      {
        char text[] = "    ";
        text[0] = '\x98' + (splash_frame_ & 0x7);
        display_.Print(text);
      }
      break;
    
    case MODE_EDIT:
      {
        uint8_t value = settings.GetValue(setting_);
        if (setting_ == SETTING_OSCILLATOR_SHAPE &&
            settings.meta_modulation()) {
          value = meta_shape_;
        }
        display_.Print(settings.metadata(setting_).strings[value]);
      }
      break;
      
    case MODE_MENU:
      {
        if (setting_ == SETTING_CV_TESTER) {
          char text[] = "    ";
          if (!blink_) {
            for (uint8_t i = 0; i < kDisplayWidth; ++i) {
              text[i] = '\x90' + (cv_[i] * 7 >> 12);
            }
          }
          display_.Print(text);
        } else if (setting_ == SETTING_MARQUEE) {
          uint8_t length = strlen(settings.marquee_text());
          uint8_t padded_length = length + 2 * kDisplayWidth - 4;
          uint8_t position = ((cv_[0] >> 4) * (padded_length - 1)) >> 8;
          position += (marquee_step_ % padded_length);
          position += 1;
          char text[] = "    ";
          for (uint8_t i = 0; i < kDisplayWidth; ++i) {
            uint8_t index = (position + i) % padded_length;
            if (index >= kDisplayWidth && index < kDisplayWidth + length) {
              text[i] = settings.marquee_text()[index - kDisplayWidth];
            }
          }
          display_.Print(text);
        } else {
          display_.Print(settings.metadata(setting_).name);
        }
      }
      break;
      
    case MODE_CALIBRATION_STEP_1:
      display_.Print(">C2 ");
      break;

    case MODE_CALIBRATION_STEP_2:
      display_.Print(">C4 ");
      break;
      
    case MODE_MARQUEE_EDITOR:
      {
        char text[] = "    ";
        for (uint8_t i = 0; i < kDisplayWidth; ++i) {
          if ((marquee_character_ + i) >= kDisplayWidth - 1) {
            const char* marquee_text = settings.marquee_text();
            text[i] = marquee_text[marquee_character_ + i - kDisplayWidth + 1];
          }
        }
        display_.Print(text);
      }
      break;

    default:
      break;
  }
}

void Ui::OnLongClick() {
  switch (mode_) {
    case MODE_MARQUEE_EDITOR:
      settings.Save();
      mode_ = MODE_MENU;
      break;

    case MODE_MENU:
      if (setting_ == SETTING_CALIBRATION) {
        mode_ = MODE_CALIBRATION_STEP_1;
      } else if (setting_ == SETTING_MARQUEE) {
        mode_ = MODE_MARQUEE_EDITOR;
        marquee_character_ = 0;
        marquee_dirty_character_ = false;
        uint8_t length = strlen(settings.marquee_text());
        settings.mutable_marquee_text()[length] = '\xA0';
      } else if (setting_ == SETTING_VERSION) {
        settings.Reset();
        settings.Save();
      }
      break;
    
    default:
      break;
  }
}

void Ui::OnClick() {
  switch (mode_) {
    case MODE_EDIT:
      mode_ = MODE_MENU;
      break;
      
    case MODE_MARQUEE_EDITOR:
      {
        if (marquee_character_ == 61) {
          ++marquee_character_;
          settings.mutable_marquee_text()[marquee_character_] = '\0';
        } else if (settings.marquee_text()[marquee_character_] == '\xA0') {
          settings.mutable_marquee_text()[marquee_character_] = '\0';
        } else {
          if (marquee_dirty_character_) {
            settings.mutable_marquee_text()[marquee_character_ + 1] = \
                settings.marquee_text()[marquee_character_];
          }
          ++marquee_character_;
          marquee_dirty_character_ = false;
        }
        if (settings.marquee_text()[marquee_character_] == '\0') {
          settings.Save();
          mode_ = MODE_MENU;
        }
      }
      break;
      
    case MODE_MENU:
      if (setting_ <= SETTING_LAST_EDITABLE_SETTING) {
        mode_ = MODE_EDIT;
        if (setting_ == SETTING_OSCILLATOR_SHAPE) {
          settings.Save();
        }
      } else if (setting_ == SETTING_VERSION) {
        mode_ = MODE_SPLASH;
      }
      break;
      
    case MODE_CALIBRATION_STEP_1:
      dac_code_c2_ = cv_[2];
      mode_ = MODE_CALIBRATION_STEP_2;
      break;
      
    case MODE_CALIBRATION_STEP_2:
      settings.Calibrate(dac_code_c2_, cv_[2], cv_[3]);
      mode_ = MODE_MENU;
      break;
      
    default:
      break;
  }
}

void Ui::OnIncrement(const Event& e) {
  switch (mode_) {
    case MODE_MARQUEE_EDITOR:
      {
        char c = settings.marquee_text()[marquee_character_];
        c += e.data;
        if (c <= ' ') {
          c = ' ';
        } else if (c >= '\xA0') {
          c = '\xA0';
        }
        settings.mutable_marquee_text()[marquee_character_] = c;
        marquee_dirty_character_ = true;
      }
      break;

    case MODE_EDIT:
      {
        int16_t value = settings.GetValue(setting_);
        value = settings.metadata(setting_).Clip(value + e.data);
        settings.SetValue(setting_, value);
        display_.set_brightness(settings.GetValue(SETTING_BRIGHTNESS) + 1);
      }
      break;
      
    case MODE_MENU:
      {
        setting_index_ += e.data;
        if (setting_index_ < 0) {
          setting_index_ = 0;
        } else if (setting_index_ >= SETTING_LAST) {
          setting_index_ = SETTING_LAST - 1;
        }
        setting_ = settings.setting_at_index(setting_index_);
        marquee_step_ = 0;
      }
      break;
      
    default:
      break;
  }
}

void Ui::DoEvents() {
  bool refresh_display_ = false;
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_ENCODER_CLICK) {
      OnClick();
    } else if (e.control_type == CONTROL_ENCODER_LONG_CLICK) {
      OnLongClick();
    } else if (e.control_type == CONTROL_ENCODER) {
      OnIncrement(e);
    }
    refresh_display_ = true;
  }
  if (queue_.idle_time() > 1000) {
    refresh_display_ = true;
  }
  if (queue_.idle_time() >= 50 && mode_ == MODE_SPLASH) {
    ++splash_frame_;
    if (splash_frame_ == 8) {
      splash_frame_ = 0;
      mode_ = MODE_EDIT;
      setting_ = SETTING_OSCILLATOR_SHAPE;
    }
    refresh_display_ = true;
  }
  if (queue_.idle_time() >= 50 &&
      (setting_ == SETTING_CV_TESTER ||
      setting_ == SETTING_MARQUEE)) {
    refresh_display_ = true;
  }
  if (queue_.idle_time() >= 50 &&
      setting_ == SETTING_OSCILLATOR_SHAPE &&
      mode_ == MODE_EDIT &&
      settings.meta_modulation()) {
    refresh_display_ = true;
  }
  if (refresh_display_) {
    queue_.Touch();
    RefreshDisplay();
    blink_ = false;
  }
}

}  // namespace braids
