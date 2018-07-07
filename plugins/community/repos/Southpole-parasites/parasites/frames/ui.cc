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
#include "frames/euclidean.h"

namespace frames {

using namespace std;
using namespace stmlib;

const uint16_t kAdcThreshold = 1 << (16 - 10);  // 10 bits
const int32_t kLongPressDuration = 800;
const int32_t kVeryLongPressDuration = 2000;
const uint16_t kKeyframeGridTolerance = 2048;
const uint8_t kDividersSteps = 7;
const uint16_t kDeadZone = 2048; // 0..32767

void Ui::Init(Keyframer* keyframer, PolyLfo* poly_lfo, Euclidean euclidean[kNumChannels]) {
  factory_testing_switch_.Init();
  channel_leds_.Init();
  keyframe_led_.Init();
  rgb_led_.Init();
  switches_.Init();
  adc_.Init(false);
  
  fill(&adc_value_[0], &adc_value_[kNumAdcChannels], 0);
        
  keyframer_ = keyframer;
  poly_lfo_ = poly_lfo;
  euclidean_ = euclidean;
  mode_ = factory_testing_switch_.Read()
      ? UI_MODE_SPLASH
      : UI_MODE_FACTORY_TESTING;
  animation_counter_ = 0;
  ignore_releases_ = 0;
  active_slot_ = 0;

  FindNearestKeyframe();
  active_keyframe_lock_ = false;
  
  feature_mode_ = static_cast<FeatureMode>(keyframer_->feature_mode_);
  for (int i=0; i<kNumChannels; i++) {
    euclidean_[i].set_length(keyframer_->euclidean_length_[i]);
    euclidean_[i].set_shape(keyframer_->euclidean_shape_[i]);
  }

  sequencer_step = 0;
  step_divider = 1;
  step_random = 0;
  shift_divider = 1;
  shift_random = 0;
  feedback_random = 0;
  active_registers = kMaxRegisters;
  for (int i=0; i<kMaxRegisters; i++)
    shift_register[i] = 0;
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

  case UI_MODE_FEATURE_SWITCH:
      animation_counter_ += 1024;
      rgb_led_.set_color(255, 255, 255);
      rgb_led_.Dim(animation_counter_);

      for (int i=0; i<4; i++)
        channel_leds_.set_channel(i, 0);

      switch (feature_mode_) {
      case FEAT_MODE_KEYFRAMER:
        break;
      case FEAT_MODE_KEYFRAME_LOOPER:
        rgb_led_.set_color(255, 0, 0);
        rgb_led_.Dim(animation_counter_);
        break;
      case FEAT_MODE_SEQ_MAIN:
        channel_leds_.set_channel(0, 255); break;
      case FEAT_MODE_SEQ_SHIFT_REGISTER:
        channel_leds_.set_channel(1, 255); break;
      case FEAT_MODE_SEQ_STEP_EDIT:
        break;
      case FEAT_MODE_POLY_LFO:
        channel_leds_.set_channel(2, 255); break;
      case FEAT_MODE_EUCLIDEAN:
        channel_leds_.set_channel(3, 255); break;
      }

      break;
    
    case UI_MODE_SAVE_CONFIRMATION:
      animation_counter_ -= 256;
      if (active_slot_ == 0) {
        channel_leds_.set_channel(0, (animation_counter_ + 0) >> 8);
        channel_leds_.set_channel(1, (animation_counter_ + 16384) >> 8);
        channel_leds_.set_channel(2, (animation_counter_ + 32768) >> 8);
        channel_leds_.set_channel(3, (animation_counter_ + 49152) >> 8);
      } else {
        for (int i=0; i<4; i++)
          channel_leds_.set_channel(i, active_slot_ == i+1 ?
                                    animation_counter_ >> 8 : 0);
      }
      rgb_led_.set_color(0, 255, 0);
      break;

    case UI_MODE_ERASE_CONFIRMATION:
      animation_counter_ -= 256;
      if (active_slot_ == 0) {
        channel_leds_.set_channel(0, (animation_counter_ + 0) >> 8);
        channel_leds_.set_channel(1, (animation_counter_ + 16384) >> 8);
        channel_leds_.set_channel(2, (animation_counter_ + 32768) >> 8);
        channel_leds_.set_channel(3, (animation_counter_ + 49152) >> 8);
      } else {
        for (int i=0; i<4; i++)
          channel_leds_.set_channel(i, active_slot_ == i+1 ?
                                    animation_counter_ >> 8 : 0);
      }
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
      
      if (feature_mode_ == FEAT_MODE_POLY_LFO) {
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
      } else if (feature_mode_ == FEAT_MODE_EUCLIDEAN) {
        channel_leds_.set_channel(0, euclidean_[0].level());
        channel_leds_.set_channel(1, euclidean_[1].level());
        channel_leds_.set_channel(2, euclidean_[2].level());
        channel_leds_.set_channel(3, euclidean_[3].level());
      } else if (feature_mode_ == FEAT_MODE_SEQ_STEP_EDIT) {
        channel_leds_.set_channel(0, keyframer_->level(0) >> 8);
        channel_leds_.set_channel(1, keyframer_->level(1) >> 8);
        channel_leds_.set_channel(2, keyframer_->level(2) >> 8);
        channel_leds_.set_channel(3, keyframer_->level(3) >> 8);
        rgb_led_.set_color(keyframer_->color());

        ++keyframe_led_pwm_counter_;
        if ((keyframe_led_pwm_counter_ & 15) >= 15) {
          keyframe_led_.High();
        } else {
          keyframe_led_.Low();
        }
        break;
      } else if (feature_mode_ == FEAT_MODE_SEQ_SHIFT_REGISTER) {
        channel_leds_.set_channel(0, shift_register[0] >> 8);
        channel_leds_.set_channel(1, shift_register[1] >> 8);
        channel_leds_.set_channel(2, shift_register[2] >> 8);
        channel_leds_.set_channel(3, shift_register[3] >> 8);
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
      } else {
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
      }
      break;

    case UI_MODE_EDIT_LENGTH:
      channel_leds_.set_channel(0, euclidean_[0].level());
      channel_leds_.set_channel(1, euclidean_[1].level());
      channel_leds_.set_channel(2, euclidean_[2].level());
      channel_leds_.set_channel(3, euclidean_[3].level());
      rgb_led_.set_color(255, 16, 32);
    break;
    case UI_MODE_EDIT_SHAPE:
      channel_leds_.set_channel(0, euclidean_[0].level());
      channel_leds_.set_channel(1, euclidean_[1].level());
      channel_leds_.set_channel(2, euclidean_[2].level());
      channel_leds_.set_channel(3, euclidean_[3].level());
      rgb_led_.set_color(16, 192, 32);
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

  // double press -> feature switch mode
  if ((e.control_id == 0 && switches_.pressed_immediate(1)) ||
      (e.control_id == 1 && switches_.pressed_immediate(0))) {
    mode_ = UI_MODE_FEATURE_SWITCH;
    ignore_releases_ = 2;
  }
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

    // hack for double presses
    if (ignore_releases_ > 0) {
      ignore_releases_--;
      return;
    }

    switch (e.control_id) {
      case SWITCH_ADD_FRAME:
        if (e.data > kVeryLongPressDuration) {
          mode_ = UI_MODE_SAVE_CONFIRMATION;
        } else if (e.data > kLongPressDuration) {
          if (feature_mode_ == FEAT_MODE_KEYFRAMER ||
              feature_mode_ == FEAT_MODE_KEYFRAME_LOOPER) {
            mode_ = UI_MODE_EDIT_EASING;
            active_channel_ = -1;
          } else if (feature_mode_ == FEAT_MODE_EUCLIDEAN) {
            mode_ = UI_MODE_EDIT_LENGTH;
          } else if (feature_mode_ == FEAT_MODE_SEQ_MAIN) {
            keyframer_->Randomize();
          }
        } else {
          if (mode_ == UI_MODE_NORMAL &&
              (feature_mode_ == FEAT_MODE_KEYFRAMER ||
               feature_mode_ == FEAT_MODE_KEYFRAME_LOOPER ||
               feature_mode_ == FEAT_MODE_SEQ_MAIN ||
               feature_mode_ == FEAT_MODE_SEQ_SHIFT_REGISTER)) {
            if (active_keyframe_ == -1) {
              keyframer_->AddKeyframe(frame(), &adc_value_[0]);
            } else {
              // This abandoned feature allowed to select and continue editing
              // a keyframe with the 4 knobs on the top even when the big
              // frame knob is being played. For that, we select the keyframe
              // we are interested in, we press "add", and this "locks" the
              // 4 pots at the top of the module for editing this keyframe.
              //active_keyframe_lock_ = true;
            }
            FindNearestKeyframe();
          } else if (mode_ == UI_MODE_SAVE_CONFIRMATION) {
            // confirming save -> write to active slot
            keyframer_->feature_mode_ = feature_mode_;
            keyframer_->Save(active_slot_);
            mode_ = UI_MODE_SPLASH;
          } else if (mode_ == UI_MODE_FEATURE_SWITCH) {
            mode_ = UI_MODE_NORMAL;
          } else if (feature_mode_ == FEAT_MODE_POLY_LFO) {
            poly_lfo_->Reset();
          } else if (feature_mode_ == FEAT_MODE_SEQ_STEP_EDIT) {
            feature_mode_ = FEAT_MODE_SEQ_MAIN;
          } else {
            mode_ = UI_MODE_NORMAL;
          }
        }
        break;

      case SWITCH_DELETE_FRAME:
        if (e.data > kVeryLongPressDuration) {
          mode_ = UI_MODE_ERASE_CONFIRMATION;
        } else if (e.data > kLongPressDuration) {
          if (feature_mode_ == FEAT_MODE_SEQ_MAIN) {
            feature_mode_ = FEAT_MODE_SEQ_STEP_EDIT;
          } else if (feature_mode_ == FEAT_MODE_KEYFRAMER ||
                     feature_mode_ == FEAT_MODE_KEYFRAME_LOOPER) {
            mode_ = UI_MODE_EDIT_RESPONSE;
            active_channel_ = -1;
          } else if (feature_mode_ == FEAT_MODE_EUCLIDEAN) {
            mode_ = UI_MODE_EDIT_SHAPE;
          } 
        } else if (mode_ == UI_MODE_ERASE_CONFIRMATION) {
          if (active_slot_ == 0) {
            keyframer_->Clear();
            FindNearestKeyframe();
            SyncWithPots();
            feature_mode_ = FEAT_MODE_KEYFRAMER;
          } else {
            keyframer_->Load(active_slot_);
            feature_mode_ = static_cast<FeatureMode>(keyframer_->feature_mode_);
            for (int i=0; i<kNumChannels; i++) {
              euclidean_[i].set_length(keyframer_->euclidean_length_[i]);
              euclidean_[i].set_shape(keyframer_->euclidean_shape_[i]);
            }
          }
          mode_ = UI_MODE_SPLASH;
        } else {
          if (mode_ == UI_MODE_NORMAL &&
              (feature_mode_ == FEAT_MODE_KEYFRAMER ||
               feature_mode_ == FEAT_MODE_KEYFRAME_LOOPER ||
               feature_mode_ == FEAT_MODE_SEQ_MAIN ||
               feature_mode_ == FEAT_MODE_SEQ_SHIFT_REGISTER)) {
            if (active_keyframe_ != -1) {
              keyframer_->RemoveKeyframe(
                  keyframer_->keyframe(active_keyframe_).timestamp);
            }
            FindNearestKeyframe();
            SyncWithPots();
          } else if (mode_ == UI_MODE_FEATURE_SWITCH) {
            mode_ = UI_MODE_NORMAL;
          } else if (feature_mode_ == FEAT_MODE_POLY_LFO) {
            poly_lfo_->Randomize();
          } else if (feature_mode_ == FEAT_MODE_SEQ_STEP_EDIT) {
            feature_mode_ = FEAT_MODE_SEQ_MAIN;
          } else {
            mode_ = UI_MODE_NORMAL;
          }
        }
        break;
    }
  }
}

  void Ui::ParseShiftSequencer(uint16_t control_id, int32_t data) {
  // knob 1 is sequence order randomization, knobs 2 and 3
  // are divisors for the shift register, knob 4 randomizes
  // feedback value
  switch (control_id) {
  case 0:
    if (active_keyframe_ != -1) {
      Keyframe* k = keyframer_->mutable_keyframe(active_keyframe_);
      k->values[0] = data;
    } else {
      keyframer_->set_immediate(0, data);
    }
    break;
  case 1:
    if (data < 32768) {
      step_random = 0;
      step_divider = (((32768 - data) * kDividersSteps >> 15) + 1) % kDividersSteps;
    } else {
      int16_t rnd = data - 32768;
      rnd = (rnd * rnd) >> 15;
      step_random = rnd >> 7;
      step_divider = 1;
    }
    break;
  case 2:
    if (data < 32768) {
      shift_random = 0;
      shift_divider = (((32768 - data) * kDividersSteps >> 15) + 1) % kDividersSteps;
    } else {
      int16_t rnd = data - 32768;
      rnd = (rnd * rnd) >> 15;
      shift_random = rnd >> 7;
      shift_divider = 1;
    }
    break;
  case 3:
    if (data < 32768) {
      feedback_random = 0;
      int16_t rnd = 32768 - data;
      rnd = (rnd * rnd) >> 15;
      sequencer_random = rnd >> 7;
    } else {
      int16_t rnd = data - 32768;
      rnd = (rnd * rnd) >> 15;
      feedback_random = rnd >> 7;
      sequencer_random = 0;
    }
    break;
    feedback_random = data >> 8;
    break;
  case kFrameAdcChannel:
    if (!active_keyframe_lock_) {
      FindNearestKeyframe();
    }
    // big knob also sets number of registers
    active_registers = (data * kMaxRegisters >> 16) + 1;
  }
}


void Ui::ParsePolyLFO(uint16_t control_id, int32_t data) {
  switch (control_id) {
  case 0:
    poly_lfo_->set_shape(data);
    break;
  case 1:
    poly_lfo_->set_shape_spread(data);
    break;
  case 2:
    poly_lfo_->set_spread(data);
    break;
  case 3:
    poly_lfo_->set_coupling(data);
    break;
  }
}

void Ui::ParseEuclidean(uint16_t control_id, int32_t data) {
  if (mode_ == UI_MODE_EDIT_LENGTH) {
    switch (control_id) {
    case 0:
    case 1:
    case 2:
    case 3: 
    {
      uint8_t len = (data >> 12) + 1;
      euclidean_[control_id].set_length(len);
      keyframer_->euclidean_length_[control_id] = len;
    }
    break;
    case kFrameAdcChannel:
      for (int i=0; i<kNumChannels; i++)
        euclidean_[i].set_rotate(data * i);
      break;
    }
  } else if (mode_ == UI_MODE_EDIT_SHAPE) {
    switch (control_id) {
    case 0:
    case 1:
    case 2:
    case 3:
    {
      euclidean_[control_id].set_shape(data);
      keyframer_->euclidean_shape_[control_id] = data;
    }
    break;
    case kFrameAdcChannel:
      for (int i=0; i<kNumChannels; i++)
        euclidean_[i].set_rotate(data * i);
      break;
    }
  } else {
    // normal mode or scratch or feature switch
    switch (control_id) {
    case 0:
    case 1:
    case 2:
    case 3:
      euclidean_[control_id].set_fill(data);
      break;
    case kFrameAdcChannel:
      for (int i=0; i<kNumChannels; i++)
        euclidean_[i].set_rotate(data * i);
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
  } else if (mode_ == UI_MODE_FEATURE_SWITCH) {
    switch (e.control_id) {
      case kFrameAdcChannel:
        feature_mode_ = e.data > 60000 ?
          FEAT_MODE_KEYFRAME_LOOPER :
          FEAT_MODE_KEYFRAMER;
        SyncWithPots();
        break;
      case 0:
        feature_mode_ = FEAT_MODE_SEQ_MAIN;
        SyncWithPots();
        break;
      case 1:
        feature_mode_ = FEAT_MODE_SEQ_SHIFT_REGISTER;
        SyncWithPotsShiftSequencer();
        break;
      case 2:
        feature_mode_ = FEAT_MODE_POLY_LFO;
        SyncWithPotsPolyLFO();
        break;
      case 3:
        feature_mode_ = FEAT_MODE_EUCLIDEAN;
        // TODO marche pas
        SyncWithPotsEuclidean();
        break;
    }
  } else if (mode_ == UI_MODE_SAVE_CONFIRMATION ||
             mode_ == UI_MODE_ERASE_CONFIRMATION) {
    switch (e.control_id) {
    case 0:
    case 1:
    case 2:
    case 3:
      active_slot_ = e.control_id + 1;
      break;
    case kFrameAdcChannel:
      active_slot_ = 0;
    }
  } else if (feature_mode_ == FEAT_MODE_POLY_LFO) {
    ParsePolyLFO(e.control_id, e.data);
  } else if (feature_mode_ == FEAT_MODE_SEQ_STEP_EDIT) {
    switch (e.control_id) {
      case 0:
      case 1:
      case 2:
      case 3:
        // the controls directly edit the value of the current step
        Keyframe* k = keyframer_->mutable_keyframe(sequencer_step);
        k->values[e.control_id] = e.data;
    }

  } else if (feature_mode_ == FEAT_MODE_EUCLIDEAN) {
    ParseEuclidean(e.control_id, e.data);

  } else if (feature_mode_ == FEAT_MODE_SEQ_SHIFT_REGISTER) {
    ParseShiftSequencer(e.control_id, e.data);
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
          keyframer_->mutable_settings(e.control_id)->easing_curve =    \
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

void Ui::SyncWithPotsShiftSequencer() {
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    ParseShiftSequencer(i, adc_value_[i]);
  }
}

void Ui::SyncWithPotsEuclidean() {
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    ParseEuclidean(i, adc_value_[i]);
  }
}

void Ui::SyncWithPotsPolyLFO() {
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    ParsePolyLFO(i, adc_value_[i]);
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
    if (mode_ == UI_MODE_SPLASH) {
      mode_ = UI_MODE_NORMAL;
    }
  }
}

}  // namespace frames
