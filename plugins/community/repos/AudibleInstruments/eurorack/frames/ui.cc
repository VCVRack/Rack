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

#include "frames/ui.h"

#include <algorithm>

#include "frames/keyframer.h"
#include "frames/poly_lfo.h"

namespace frames {

using namespace std;
using namespace stmlib;

const uint16_t kAdcThreshold = 1 << (16 - 10);  // 10 bits
const int32_t kLongPressDuration = 800;
const int32_t kVeryLongPressDuration = 3000;
const uint16_t kKeyframeGridTolerance = 2048;

void Ui::Init(Keyframer* keyframer, PolyLfo* poly_lfo) {
  factory_testing_switch_.Init();
  channel_leds_.Init();
  keyframe_led_.Init();
  rgb_led_.Init();
  switches_.Init();
  adc_.Init(false);
  
  fill(&adc_value_[0], &adc_value_[kNumAdcChannels], 0);
        
  keyframer_ = keyframer;
  poly_lfo_ = poly_lfo;
  mode_ = factory_testing_switch_.Read()
      ? UI_MODE_SPLASH
      : UI_MODE_FACTORY_TESTING;
  animation_counter_ = 0;
  
  FindNearestKeyframe();
  active_keyframe_lock_ = false;
  
  uint32_t ui_flags = keyframer_->extra_settings();
  poly_lfo_mode_ = ui_flags & 1;
  sequencer_mode_= ui_flags & 2;
  secret_handshake_counter_ = 0;
}

void Ui::TryCalibration() {
  if (switches_.pressed_immediate(1)) {
    keyframer_->Calibrate(frame_modulation());
    FlushEvents();
  }
}

void Ui::Poll() {
  switches_.Debounce();
  
  for (uint8_t i = 0; i < kNumSwitches; ++i) {
    if (switches_.just_pressed(i)) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
      detect_very_long_press_[i] = false;
    }
    if (switches_.pressed(i)) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];

      if (!detect_very_long_press_[i]) {
        if (pressed_time > kLongPressDuration) {
          queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
          detect_very_long_press_[i] = true;
        }
      } else {
        if (pressed_time > kVeryLongPressDuration) {
          queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
          detect_very_long_press_[i] = false;
          press_time_[i] = 0;
        }
      }
    }
    
    if (switches_.released(i) &&
        press_time_[i] != 0 &&
        !detect_very_long_press_[i]) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
      press_time_[i] = 0;
      detect_very_long_press_[i] = false;
    }
  }
  
  for (uint8_t i = 0; i <= kFrameAdcChannel; ++i) {
    int32_t value = (31 * adc_filtered_value_[i] + adc_.value(i)) >> 5;
    adc_filtered_value_[i] = value;
    int32_t current_value = static_cast<int32_t>(adc_value_[i]);
    if (value >= current_value + kAdcThreshold ||
        value <= current_value - kAdcThreshold) {
      queue_.AddEvent(CONTROL_POT, i, value);
      adc_value_[i] = value;
    }
  }
  
  switch (mode_) {
    case UI_MODE_SPLASH:
      animation_counter_ += 128;
      channel_leds_.set_channel(0, (animation_counter_ + 49152) >> 8);
      channel_leds_.set_channel(1, (animation_counter_ + 32768) >> 8);
      channel_leds_.set_channel(2, (animation_counter_ + 16384) >> 8);
      channel_leds_.set_channel(3, (animation_counter_ + 0) >> 8);
      rgb_led_.set_color(255, 255, 255);
      break;
    
    case UI_MODE_SAVE_CONFIRMATION:
      animation_counter_ -= 256;
      channel_leds_.set_channel(0, (animation_counter_ + 0) >> 8);
      channel_leds_.set_channel(1, (animation_counter_ + 16384) >> 8);
      channel_leds_.set_channel(2, (animation_counter_ + 32768) >> 8);
      channel_leds_.set_channel(3, (animation_counter_ + 49152) >> 8);
      rgb_led_.set_color(0, 255, 0);
      break;

    case UI_MODE_ERASE_CONFIRMATION:
      animation_counter_ -= 256;
      channel_leds_.set_channel(0, (animation_counter_ + 0) >> 8);
      channel_leds_.set_channel(1, (animation_counter_ + 16384) >> 8);
      channel_leds_.set_channel(2, (animation_counter_ + 32768) >> 8);
      channel_leds_.set_channel(3, (animation_counter_ + 49152) >> 8);
      rgb_led_.set_color(255, 0, 0);
      break;

    case UI_MODE_FACTORY_TESTING:
      channel_leds_.set_channel(0, keyframer_->level(0) >> 8);
      channel_leds_.set_channel(1, keyframer_->level(1) >> 8);
      channel_leds_.set_channel(2, keyframer_->level(2) >> 8);
      channel_leds_.set_channel(3, keyframer_->level(3) >> 8);
      if (frame() < 4096) {
        rgb_led_.set_color(255, 0, 0);
      } else if (frame() > 61440) {
        rgb_led_.set_color(0, 255, 0);
      } else {
        uint8_t v = frame_modulation() >> 8;
        rgb_led_.set_color(0, 0, v);
      }
      if (test_led_) {
        keyframe_led_.High();
      } else {
        keyframe_led_.Low();
      }
      break;
      
    case UI_MODE_NORMAL:
      channel_leds_.set_channel(0, keyframer_->level(0) >> 8);
      channel_leds_.set_channel(1, keyframer_->level(1) >> 8);
      channel_leds_.set_channel(2, keyframer_->level(2) >> 8);
      channel_leds_.set_channel(3, keyframer_->level(3) >> 8);
      rgb_led_.set_color(keyframer_->color());
      if (active_keyframe_ == -1) {
        keyframe_led_.Low();
      } else {
        animation_counter_ += 256;
        int32_t distance = frame() - \
            keyframer_->keyframe(active_keyframe_).timestamp;
        distance = min(distance * distance >> 18, int32_t(15));
        ++keyframe_led_pwm_counter_;
        if ((keyframe_led_pwm_counter_ & 15) >= distance) {
          keyframe_led_.High();
        } else {
          keyframe_led_.Low();
        }
        if (active_keyframe_lock_) {
          if (animation_counter_ & 0x8000) {
            keyframe_led_.High();
          } else {
            keyframe_led_.Low();
          }
        }
      }
      
      if (poly_lfo_mode_) {
        channel_leds_.set_channel(0, poly_lfo_->level(0));
        channel_leds_.set_channel(1, poly_lfo_->level(1));
        channel_leds_.set_channel(2, poly_lfo_->level(2));
        channel_leds_.set_channel(3, poly_lfo_->level(3));
        rgb_led_.set_color(poly_lfo_->color());
        if (poly_lfo_->level(0) > 128) {
          keyframe_led_.High();
        } else {
          keyframe_led_.Low();
        }
      }
      break;
      
    case UI_MODE_EDIT_RESPONSE:
    case UI_MODE_EDIT_EASING:
      {
        animation_counter_ += 48;
        for (uint8_t i = 0; i < 4; ++i) {
          channel_leds_.set_channel(i, active_channel_ == i ? 255 : 0);
        }
        if (mode_ == UI_MODE_EDIT_EASING) {
          rgb_led_.set_color(255, 16, 32);
        } else {
          rgb_led_.set_color(16, 192, 32);
        }
        uint16_t brightness = active_channel_ == -1
            ? 65535
            : keyframer_->SampleAnimation(active_channel_,
                                          animation_counter_,
                                          mode_ == UI_MODE_EDIT_EASING);
        rgb_led_.Dim(brightness);
      }
      break;
  }
  
  rgb_led_.Write();
  channel_leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {
  test_led_ = true;
}

void Ui::OnSwitchReleased(const Event& e) {
  if (mode_ == UI_MODE_FACTORY_TESTING) {
    test_led_ = false;
  } else  {
    if (active_keyframe_lock_) {
      active_keyframe_lock_ = false;
      FindNearestKeyframe();
      return;
    }
    
    switch (e.control_id) {
      case SWITCH_ADD_FRAME:
        if (e.data > kVeryLongPressDuration) {
          uint32_t ui_flags = 0;
          ui_flags |= poly_lfo_mode_ ? 1 : 0;
          ui_flags |= sequencer_mode_ ? 2 : 0;
          keyframer_->Save(ui_flags);
          mode_ = UI_MODE_SAVE_CONFIRMATION;
        } else if (e.data > kLongPressDuration) {
          if (!poly_lfo_mode_) {
            mode_ = UI_MODE_EDIT_EASING;
            active_channel_ = -1;
          }
        } else {
          if (mode_ == UI_MODE_NORMAL && !poly_lfo_mode_) {
            if (active_keyframe_ == -1) {
              keyframer_->AddKeyframe(frame(), &adc_value_[0]);
            } else {
              ++secret_handshake_counter_;
              if (secret_handshake_counter_ >= 5) {
                sequencer_mode_ = !sequencer_mode_;
              }
              // This abandoned feature allowed to select and continue editing
              // a keyframe with the 4 knobs on the top even when the big
              // frame knob is being played. For that, we select the keyframe
              // we are interested in, we press "add", and this "locks" the
              // 4 pots at the top of the module for editing this keyframe.
              //active_keyframe_lock_ = true;
            }
            FindNearestKeyframe();
          } else {
            mode_ = UI_MODE_NORMAL;
          }
        }
        break;
      
      case SWITCH_DELETE_FRAME:
        if (frame() < 128) {
          --secret_handshake_counter_;
          if (secret_handshake_counter_ <= -10) {
            poly_lfo_mode_ = !poly_lfo_mode_;
            secret_handshake_counter_ = 0;
          }
        }
        if (e.data > kVeryLongPressDuration) {
          keyframer_->Clear();
          FindNearestKeyframe();
          SyncWithPots();
          poly_lfo_mode_ = false;
          mode_ = UI_MODE_ERASE_CONFIRMATION;
        } else if (e.data > kLongPressDuration) {
          if (!poly_lfo_mode_) {
            mode_ = UI_MODE_EDIT_RESPONSE;
            active_channel_ = -1;
          }
        } else {
          if (mode_ == UI_MODE_NORMAL && !poly_lfo_mode_) {
            if (active_keyframe_ != -1) {
              keyframer_->RemoveKeyframe(
                  keyframer_->keyframe(active_keyframe_).timestamp);
            }
            FindNearestKeyframe();
            SyncWithPots();
          } else {
            mode_ = UI_MODE_NORMAL;
          }
        }
        break;
    }
  }
}


void Ui::OnPotChanged(const Event& e) {
  if (mode_ == UI_MODE_FACTORY_TESTING) {
    switch (e.control_id) {
      case 0:
      case 1:
      case 2:
      case 3:
        keyframer_->set_immediate(e.control_id, e.data);
        break;
    }
  } else if (poly_lfo_mode_) {
    switch (e.control_id) {
      case 0:
        poly_lfo_->set_shape(e.data);
        break;
      case 1:
        poly_lfo_->set_shape_spread(e.data);
        break;
      case 2:
        poly_lfo_->set_spread(e.data);
        break;
      case 3:
        poly_lfo_->set_coupling(e.data);
        break;
    }
  } else {
    switch (e.control_id) {
      case 0:
      case 1:
      case 2:
      case 3:
        if (mode_ == UI_MODE_NORMAL || mode_ == UI_MODE_SPLASH) {
          if (active_keyframe_ != -1) {
            Keyframe* k = keyframer_->mutable_keyframe(active_keyframe_);
            k->values[e.control_id] = e.data;
          } else {
            keyframer_->set_immediate(e.control_id, e.data);
          }
        } else if (mode_ == UI_MODE_EDIT_RESPONSE) {
          active_channel_ = e.control_id;
          keyframer_->mutable_settings(e.control_id)->response = e.data >> 8;
        } else if (mode_ == UI_MODE_EDIT_EASING) {
          active_channel_ = e.control_id;
          keyframer_->mutable_settings(e.control_id)->easing_curve = \
              static_cast<EasingCurve>(e.data * 6 >> 16);
        }
        break;
      
      case kFrameAdcChannel:
        if (!active_keyframe_lock_) {
          FindNearestKeyframe();
        }
        break;
        
      case kFrameModulationAdcChannel:
        break;
    }
  } 
}

void Ui::FindNearestKeyframe() {
  active_keyframe_ = keyframer_->FindNearestKeyframe(
      frame(),
      kKeyframeGridTolerance);
}

void Ui::SyncWithPots() {
  if (!keyframer_->num_keyframes()) {
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      keyframer_->set_immediate(i, adc_filtered_value_[i]);
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
    } else if (e.control_type == CONTROL_POT) {
      OnPotChanged(e);
    }
  }
  if (queue_.idle_time() > 500) {
    queue_.Touch();
    if (mode_ == UI_MODE_SPLASH
        || mode_ == UI_MODE_SAVE_CONFIRMATION
        || mode_ == UI_MODE_ERASE_CONFIRMATION) {
      mode_ = UI_MODE_NORMAL;
    }
    secret_handshake_counter_ = 0;
  }
}

}  // namespace frames
