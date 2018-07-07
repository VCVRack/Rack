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

#include "clouds/ui.h"

#include "stmlib/system/system_clock.h"

#include "clouds/dsp/granular_processor.h"
#include "clouds/cv_scaler.h"
#include "clouds/meter.h"

namespace clouds {

const int32_t kLongPressDuration = 1000;
const int32_t kVeryLongPressDuration = 2500;

using namespace stmlib;

void Ui::Init(
    Settings* settings,
    CvScaler* cv_scaler,
    GranularProcessor* processor,
    Meter* meter) {
  settings_ = settings;
  cv_scaler_ = cv_scaler;
  leds_.Init();
  switches_.Init();
  
  processor_ = processor;
  meter_ = meter;
  mode_ = UI_MODE_SPLASH;
  ignore_releases_ = 0;
  
  const State& state = settings_->state();
  
  // Sanitize saved settings.
  cv_scaler_->set_blend_parameter(
      static_cast<BlendParameter>(state.blend_parameter & 3));
  processor_->set_quality(state.quality & 3);
  processor_->set_playback_mode(
      static_cast<PlaybackMode>(state.playback_mode % PLAYBACK_MODE_LAST));
  for (int32_t i = 0; i < BLEND_PARAMETER_LAST; ++i) {
    cv_scaler_->set_blend_value(
        static_cast<BlendParameter>(i),
        static_cast<float>(state.blend_value[i]) / 255.0f);
  }
  cv_scaler_->UnlockBlendKnob();

  if (switches_.pressed_immediate(SWITCH_WRITE)) {
    mode_ = UI_MODE_CALIBRATION_1;
    ignore_releases_ = 1;
  }  
}

void Ui::SaveState() {
  State* state = settings_->mutable_state();
  state->blend_parameter = cv_scaler_->blend_parameter();
  state->quality = processor_->quality();
  state->playback_mode = processor_->playback_mode();
  for (int32_t i = 0; i < BLEND_PARAMETER_LAST; ++i) {
    state->blend_value[i] = static_cast<uint8_t>(
        cv_scaler_->blend_value(static_cast<BlendParameter>(i)) * 255.0f);
  }
  settings_->Save();
}

void Ui::Poll() {
  system_clock.Tick();
  switches_.Debounce();
  
  for (uint8_t i = 0; i < kNumSwitches; ++i) {
    if (switches_.just_pressed(i)) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
      long_press_time_[i] = system_clock.milliseconds();
    }
    if (switches_.pressed(i) && press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];
      if (pressed_time > kLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        press_time_[i] = 0;
      }
    }
    if (switches_.pressed(i) && long_press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - long_press_time_[i];
      if (pressed_time > kVeryLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        long_press_time_[i] = 0;
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
  PaintLeds();
}

void Ui::PaintLeds() {
  leds_.Clear();
  uint32_t clock = system_clock.milliseconds();
  bool blink = (clock & 127) > 64;
  bool flash = (clock & 511) < 16;
  uint8_t fade = clock >> 1;
  fade = fade <= 127 ? (fade << 1) : 255 - (fade << 1);
  fade = static_cast<uint16_t>(fade) * fade >> 8;
  switch (mode_) {
    case UI_MODE_SPLASH:
      {
        uint8_t index = ((clock >> 8) + 1) & 3;
        uint8_t fade = (clock >> 2);
        fade = fade <= 127 ? (fade << 1) : 255 - (fade << 1);
        leds_.set_intensity(3 - index, fade);
      }
      break;
      
    case UI_MODE_VU_METER:
      leds_.PaintBar(lut_db[meter_->peak() >> 7]);
      break;
    
    case UI_MODE_BLEND_METER:
      for (int32_t i = 0; i < 4; ++i) {
        leds_.set_intensity(
            i,
            cv_scaler_->blend_value(static_cast<BlendParameter>(i)) * 255.0f);
      }
      break;
    
    case UI_MODE_QUALITY:
      leds_.set_status(processor_->quality(), 255, 0);
      break;
      
    case UI_MODE_BLENDING:
      leds_.set_status(cv_scaler_->blend_parameter(), 0, 255);
      break;
      
    case UI_MODE_PLAYBACK_MODE:
      if (blink) {
        for (int i=0; i<4; i++)
          leds_.set_status(i, 0, 0);
      } else if (processor_->playback_mode() < 4) {      
        leds_.set_status(processor_->playback_mode(),
                         128 + (fade >> 1),
                         255 - (fade >> 1));
      } else {
        for (int i=0; i<4; i++)
          leds_.set_status(i, 128 + (fade >> 1), 255 - (fade >> 1));
        leds_.set_status(processor_->playback_mode() & 3, 0, 0);
      }
      
      break;
      
    case UI_MODE_LOAD:
      leds_.set_status(load_save_location_, 0, blink ? 255 : 0);
      break;

    case UI_MODE_SAVE:
      leds_.set_status(load_save_location_, blink ? 255 : 0, 0);
      break;
    
    case UI_MODE_SAVING:
      leds_.set_status(load_save_location_, 255, 0);
      break;
      
    case UI_MODE_CALIBRATION_1:
      leds_.set_status(0, blink ? 255 : 0, blink ? 255 : 0);
      leds_.set_status(1, blink ? 255 : 0, blink ? 255 : 0);
      leds_.set_status(2, 0, 0);
      leds_.set_status(3, 0, 0);
      break;

    case UI_MODE_CALIBRATION_2:
      leds_.set_status(0, blink ? 255 : 0, blink ? 255 : 0);
      leds_.set_status(1, blink ? 255 : 0, blink ? 255 : 0);
      leds_.set_status(2, blink ? 255 : 0, blink ? 255 : 0);
      leds_.set_status(3, blink ? 255 : 0, blink ? 255 : 0);
      break;
      
    case UI_MODE_PANIC:
      leds_.set_status(0, 255, 0);
      leds_.set_status(1, 255, 0);
      leds_.set_status(2, 255, 0);
      leds_.set_status(3, 255, 0);
      break;
      
    default:
      break;
  }

  bool freeze = processor_->frozen();
  if (processor_->reversed()) {
    freeze ^= flash;
  }
  leds_.set_freeze(freeze);
  if (processor_->bypass()) {
    leds_.PaintBar(lut_db[meter_->peak() >> 7]);
    leds_.set_freeze(true);
  }
  
  leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {
  // double press -> feature switch mode
  if ((e.control_id == SWITCH_MODE && switches_.pressed_immediate(SWITCH_WRITE)) ||
      (e.control_id == SWITCH_WRITE && switches_.pressed_immediate(SWITCH_MODE))) {
    mode_ = UI_MODE_PLAYBACK_MODE;
    ignore_releases_ = 2;
  }
}

void Ui::CalibrateC1() {
  cv_scaler_->CalibrateC1();
  cv_scaler_->CalibrateOffsets();
  mode_ = UI_MODE_CALIBRATION_2;
}

void Ui::CalibrateC3() {
  bool success = cv_scaler_->CalibrateC3();
  if (success) {
    settings_->Save();
    mode_ = UI_MODE_VU_METER;
  } else {
    mode_ = UI_MODE_PANIC;
  }
}

void Ui::OnSecretHandshake() {
  mode_ = UI_MODE_PLAYBACK_MODE;
}

void Ui::OnSwitchReleased(const Event& e) {

  // hack for double presses
  if (ignore_releases_ > 0) {
    ignore_releases_--;
    return;
  }

  switch (e.control_id) {
    case SWITCH_FREEZE:
      if (e.data >= kVeryLongPressDuration) {
      } else if (e.data >= kLongPressDuration) {
        processor_->ToggleReverse();
      } else {
        processor_->ToggleFreeze();
      }
      break;

    case SWITCH_MODE:
      if (e.data >= kVeryLongPressDuration) {
        mode_ = UI_MODE_PLAYBACK_MODE;
      } else if (e.data >= kLongPressDuration) {
        if (mode_ == UI_MODE_QUALITY) {
          mode_ = UI_MODE_VU_METER;
        } else {
          mode_ = UI_MODE_QUALITY;
        }
      } else if (mode_ == UI_MODE_VU_METER || mode_ == UI_MODE_BLEND_METER) {
        mode_ = UI_MODE_BLENDING;
      } else if (mode_ == UI_MODE_BLENDING) {
        uint8_t parameter = (cv_scaler_->blend_parameter() + 1) & 3;
        cv_scaler_->set_blend_parameter(static_cast<BlendParameter>(parameter));
        SaveState();
      } else if (mode_ == UI_MODE_QUALITY) {
        processor_->set_quality((processor_->quality() + 1) & 3);
        SaveState();
      } else if (mode_ == UI_MODE_PLAYBACK_MODE) {
        uint8_t mode = processor_->playback_mode() == 0 ?
          PLAYBACK_MODE_LAST-1 :
          processor_->playback_mode() - 1;
        processor_->set_playback_mode(static_cast<PlaybackMode>(mode));
        SaveState();
      } else if (mode_ == UI_MODE_SAVE || mode_ == UI_MODE_LOAD) {
        load_save_location_ = (load_save_location_ + 1) & 3;
      } else {
        mode_ = UI_MODE_VU_METER;
      }
      break;

    case SWITCH_WRITE:
      if (mode_ == UI_MODE_CALIBRATION_1) {
        CalibrateC1();
      } else if (mode_ == UI_MODE_CALIBRATION_2) {
        CalibrateC3();
      } else if (mode_ == UI_MODE_SAVE) {
        // Get pointers on data chunks to save.
        PersistentBlock blocks[4];
        size_t num_blocks = 0;
        
        mode_ = UI_MODE_SAVING;
        // Silence the processor during the long erase/write.
        processor_->set_silence(true);
        system_clock.Delay(5);
        processor_->PreparePersistentData();
        processor_->GetPersistentData(blocks, &num_blocks);
        settings_->SaveSampleMemory(load_save_location_, blocks, num_blocks);
        processor_->set_silence(false);
        load_save_location_ = (load_save_location_ + 1) & 3;
        mode_ = UI_MODE_VU_METER;
      } else if (mode_ == UI_MODE_LOAD) {
        processor_->LoadPersistentData(settings_->sample_flash_data(
            load_save_location_));
        load_save_location_ = (load_save_location_ + 1) & 3;
        mode_ = UI_MODE_VU_METER;
      } else if (mode_ == UI_MODE_PLAYBACK_MODE) {
        uint8_t mode = (processor_->playback_mode() + 1) % PLAYBACK_MODE_LAST;
        processor_->set_playback_mode(static_cast<PlaybackMode>(mode));
        SaveState();
      } else if (e.data >= kLongPressDuration) {
        mode_ = UI_MODE_SAVE;
      } else {
        mode_ = UI_MODE_LOAD;
      }
      break;
  }
}

void Ui::DoEvents() {
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_SWITCH) {
      if (e.data == 0) {
        OnSwitchPressed(e);
      } else {
        if (e.data >= kLongPressDuration &&
            e.control_id == SWITCH_MODE && 
            switches_.pressed(SWITCH_WRITE)) {
          press_time_[SWITCH_WRITE] = 0;
          OnSecretHandshake();
        } else {
          OnSwitchReleased(e);
        }
      }
    }
  }
  
  if (queue_.idle_time() > 1000 && mode_ == UI_MODE_PANIC) {
    queue_.Touch();
    mode_ = UI_MODE_VU_METER;
  }
  
  if ((mode_ == UI_MODE_VU_METER || mode_ == UI_MODE_BLEND_METER ||
       mode_ == UI_MODE_BLENDING) && \
      cv_scaler_->blend_knob_touched()) {
    queue_.Touch();
    mode_ = UI_MODE_BLEND_METER;
  }
  
  if (queue_.idle_time() > 3000) {
    queue_.Touch();
    if (mode_ == UI_MODE_BLENDING || mode_ == UI_MODE_QUALITY ||
        mode_ == UI_MODE_PLAYBACK_MODE || mode_ == UI_MODE_SAVE ||
        mode_ == UI_MODE_LOAD || mode_ == UI_MODE_BLEND_METER ||
        mode_ == UI_MODE_SPLASH) {
      mode_ = UI_MODE_VU_METER;
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
      
    case FACTORY_TESTING_READ_GATE:
      if (argument <= 2) {
        return switches_.pressed(argument);
      } else {
        return cv_scaler_->gate(argument - 3);
      }
      break;
      
    case FACTORY_TESTING_SET_BYPASS:
      processor_->set_bypass(argument);
      break;
      
    case FACTORY_TESTING_CALIBRATE:
      if (argument == 0) {
        mode_ = UI_MODE_CALIBRATION_1;
      } else if (argument == 1) {
        CalibrateC1();
      } else {
        CalibrateC3();
        cv_scaler_->set_blend_parameter(static_cast<BlendParameter>(0));
        SaveState();
      }
      break;
  }
  return reply;
}

}  // namespace clouds
