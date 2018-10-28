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

#include "marbles/ui.h"

#include <algorithm>

#include "stmlib/system/system_clock.h"

#include "marbles/drivers/clock_inputs.h"
#include "marbles/cv_reader.h"
#include "marbles/scale_recorder.h"
#include "marbles/settings.h"

namespace marbles {

const int32_t kLongPressDuration = 2000;

using namespace std;
using namespace stmlib;

/* static */
const LedColor Ui::palette_[4] = {
  LED_COLOR_GREEN,
  LED_COLOR_YELLOW,
  LED_COLOR_RED,
  LED_COLOR_OFF
};

/* static */
AlternateKnobMapping Ui::alternate_knob_mappings_[ADC_CHANNEL_LAST];

void Ui::Init(
    Settings* settings,
    CvReader* cv_reader,
    ScaleRecorder* scale_recorder,
    ClockInputs* clock_inputs) {
  settings_ = settings;
  cv_reader_ = cv_reader;
  scale_recorder_ = scale_recorder;
  clock_inputs_ = clock_inputs;
  
  leds_.Init();
  switches_.Init();
  queue_.Init();
  
  // Initialize generator from settings_->state();
  fill(&pot_value_[0], &pot_value_[ADC_CHANNEL_LAST], 0.0f);
  
  State* state = settings_->mutable_state();
  alternate_knob_mappings_[ADC_CHANNEL_T_BIAS].unlock_switch = SWITCH_T_MODEL;
  alternate_knob_mappings_[ADC_CHANNEL_T_BIAS].destination = &state->t_pulse_width_mean;
  alternate_knob_mappings_[ADC_CHANNEL_T_JITTER].unlock_switch = SWITCH_T_MODEL;
  alternate_knob_mappings_[ADC_CHANNEL_T_JITTER].destination = &state->t_pulse_width_std;
  alternate_knob_mappings_[ADC_CHANNEL_T_RATE].unlock_switch = SWITCH_X_MODE;
  alternate_knob_mappings_[ADC_CHANNEL_T_RATE].destination = &state->y_divider;
  alternate_knob_mappings_[ADC_CHANNEL_X_SPREAD].unlock_switch = SWITCH_X_MODE;
  alternate_knob_mappings_[ADC_CHANNEL_X_SPREAD].destination = &state->y_spread;
  alternate_knob_mappings_[ADC_CHANNEL_X_BIAS].unlock_switch = SWITCH_X_MODE;
  alternate_knob_mappings_[ADC_CHANNEL_X_BIAS].destination = &state->y_bias;
  alternate_knob_mappings_[ADC_CHANNEL_X_STEPS].unlock_switch = SWITCH_X_MODE;
  alternate_knob_mappings_[ADC_CHANNEL_X_STEPS].destination = &state->y_steps;
  
  setting_modification_flag_ = false;
  output_test_mode_ = false;
  
  if (switches_.pressed_immediate(SWITCH_X_MODE)) {
    if (state->color_blind == 1) {
      state->color_blind = 0;
    } else {
      state->color_blind = 1; 
    }
    settings_->SaveState();
  }
  
  deja_vu_lock_ = false;
}

void Ui::SaveState() {
  settings_->SaveState();
}

void Ui::Poll() {
  // 1kHz.
  system_clock.Tick();
  switches_.Debounce();
  
  for (int i = 0; i < SWITCH_LAST; ++i) {
    if (switches_.just_pressed(Switch(i))) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
      ignore_release_[i] = false;
    }
    if (switches_.pressed(Switch(i)) && !ignore_release_[i]) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];
      if (pressed_time > kLongPressDuration && !setting_modification_flag_) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        ignore_release_[i] = true;
      }
    }
    if (switches_.released(Switch(i)) && !ignore_release_[i]) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
      ignore_release_[i] = true;
    }
  }
  
  UpdateLEDs();
}

/* static */
LedColor Ui::MakeColor(uint8_t value, bool color_blind) {
  bool slow_blink = (system_clock.milliseconds() & 255) > 128;

  uint8_t bank = value >= 3 ? 1 : 0;
  value -= bank * 3;
  
  LedColor color = palette_[value];
  if (color_blind) {
    uint8_t pwm_counter = system_clock.milliseconds() & 15;
    uint8_t triangle = (system_clock.milliseconds() >> 5) & 31;
    triangle = triangle < 16 ? triangle : 31 - triangle;
  
    if (value == 0) {
      color = pwm_counter < (4 + (triangle >> 2))
          ? LED_COLOR_GREEN
          : LED_COLOR_OFF;
    } else if (value == 1) {
      color = LED_COLOR_YELLOW;
    } else {
      color = pwm_counter == 0 ? LED_COLOR_RED : LED_COLOR_OFF;
    }
  }

  return slow_blink || !bank ? color : LED_COLOR_OFF;
}

void Ui::UpdateLEDs() {
  bool blink = (system_clock.milliseconds() & 127) > 64;
  bool slow_blink = (system_clock.milliseconds() & 255) > 128;
  bool fast_blink = (system_clock.milliseconds() & 63) > 32;
  const State& state = settings_->state();
  bool cb = state.color_blind == 1;
  
  LedColor scale_color = state.x_scale < 3
      ? (slow_blink ? palette_[state.x_scale] : LED_COLOR_OFF)
      : (fast_blink ? palette_[state.x_scale - 3] : LED_COLOR_OFF);
  
  if (cb) {
    int poly_counter = (system_clock.milliseconds() >> 6) % 12;
    if ((poly_counter >> 1) < (state.x_scale + 1) && (poly_counter & 1)) {
      scale_color = LED_COLOR_YELLOW;
    } else {
      scale_color = LED_COLOR_OFF;
    }
  }
  
  leds_.Clear();
  
  int slow_triangle = (system_clock.milliseconds() & 1023) >> 5;
  slow_triangle = slow_triangle >= 16 ? 31 - slow_triangle : slow_triangle;
  int pw = system_clock.milliseconds() & 15;
  bool deja_vu_glow = !deja_vu_lock_ || (slow_triangle >= pw);
  
  switch (mode_) {
    case UI_MODE_NORMAL:
    case UI_MODE_RECORD_SCALE:
      {
        leds_.set(LED_T_MODEL, MakeColor(state.t_model, cb));
        leds_.set(LED_T_RANGE, MakeColor(state.t_range, cb));
        leds_.set(LED_T_DEJA_VU,
                  state.t_deja_vu && deja_vu_glow ?
                      LED_COLOR_GREEN : LED_COLOR_OFF);
        leds_.set(LED_X_CONTROL_MODE, MakeColor(state.x_control_mode, cb));
        leds_.set(LED_X_DEJA_VU,
                  state.x_deja_vu && deja_vu_glow ?
                      LED_COLOR_GREEN : LED_COLOR_OFF);
                  
        if (mode_ == UI_MODE_NORMAL) {
          leds_.set(LED_X_RANGE,
                    state.x_register_mode
                        ? LED_COLOR_OFF
                        : MakeColor(state.x_range, cb));
          leds_.set(LED_X_EXT,
                    state.x_register_mode ? LED_COLOR_GREEN : LED_COLOR_OFF);
        } else {
          leds_.set(LED_X_RANGE, scale_color);
          leds_.set(LED_X_EXT, LED_COLOR_GREEN);
        }
      }
      break;

    case UI_MODE_SELECT_SCALE:
      leds_.set(LED_X_RANGE, scale_color);
      break;
    
    case UI_MODE_CALIBRATION_1:
      leds_.set(LED_T_RANGE, blink ? MakeColor(0, cb) : LED_COLOR_OFF);
      break;

    case UI_MODE_CALIBRATION_2:
      leds_.set(LED_T_RANGE, blink ? MakeColor(1, cb) : LED_COLOR_OFF);
      break;

    case UI_MODE_CALIBRATION_3:
      leds_.set(LED_X_RANGE, blink ? MakeColor(0, cb) : LED_COLOR_OFF);
      break;

    case UI_MODE_CALIBRATION_4:
      leds_.set(LED_X_RANGE, blink ? MakeColor(1, cb) : LED_COLOR_OFF);
      break;
    
    case UI_MODE_PANIC:
      leds_.set(LED_T_MODEL, blink ? LED_COLOR_RED : LED_COLOR_OFF);
      leds_.set(LED_T_RANGE, !blink ? LED_COLOR_RED : LED_COLOR_OFF);
      leds_.set(LED_X_CONTROL_MODE, !blink ? LED_COLOR_RED : LED_COLOR_OFF);
      leds_.set(LED_X_RANGE, blink ? LED_COLOR_RED : LED_COLOR_OFF);
      break;
  }
  leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {

}

void Ui::OnSwitchReleased(const Event& e) {
  if (setting_modification_flag_) {
    for (int i = 0; i < ADC_CHANNEL_LAST; ++i) {
      cv_reader_->mutable_channel(i)->UnlockPot();
    }
    setting_modification_flag_ = false;
    return;
  }
  
  // Check if the other switch is still pressed.
  if (e.control_id == SWITCH_T_RANGE && switches_.pressed(SWITCH_X_RANGE)) {
    mode_ = UI_MODE_CALIBRATION_1;
    ignore_release_[SWITCH_T_RANGE] = ignore_release_[SWITCH_X_RANGE] = true;
    return;
  }
  
  State* state = settings_->mutable_state();
  switch (e.control_id) {
    case SWITCH_T_DEJA_VU:
      state->t_deja_vu = !state->t_deja_vu;
      break;

    case SWITCH_X_DEJA_VU:
      state->x_deja_vu = !state->x_deja_vu;
      break;
    
    case SWITCH_T_MODEL:
      {
        uint8_t bank = state->t_model / 3;
        if (e.data >= kLongPressDuration) {
          if (!bank) {
            state->t_model += 3;
          }
        } else {
          if (bank) {
            state->t_model -= 3;
          } else {
            state->t_model = (state->t_model + 1) % 3;
          }
        }
        SaveState();
      }
      break;

    case SWITCH_T_RANGE:
      {
        if (mode_ >= UI_MODE_CALIBRATION_1 && mode_ <= UI_MODE_CALIBRATION_4) {
          NextCalibrationStep();
        } else {
          state->t_range = (state->t_range + 1) % 3;
        }
        SaveState();
      }
      break;
    
    case SWITCH_X_MODE:
      state->x_control_mode = (state->x_control_mode + 1) % 3;
      SaveState();
      break;
      
    case SWITCH_X_EXT:
      if (mode_ == UI_MODE_RECORD_SCALE) {
        int scale_index = settings_->state().x_scale;
        bool success = true;
        if (e.data >= kLongPressDuration) {
          settings_->ResetScale(scale_index);
        } else {
          success = scale_recorder_->ExtractScale(
              settings_->mutable_scale(scale_index));
        }
        if (success) {
          settings_->SavePersistentData();
          settings_->set_dirty_scale_index(scale_index);
        }
        mode_ = UI_MODE_NORMAL;
      } else if (e.data >= kLongPressDuration) {
        mode_ = UI_MODE_RECORD_SCALE;
        scale_recorder_->Clear();
      } else {
        state->x_register_mode = !state->x_register_mode;
        SaveState();
      }
      break;

    case SWITCH_X_RANGE:
      if (mode_ >= UI_MODE_CALIBRATION_1 && mode_ <= UI_MODE_CALIBRATION_4) {
        NextCalibrationStep();
      } else if (e.data >= kLongPressDuration) {
        if (mode_ == UI_MODE_NORMAL) {
          mode_ = UI_MODE_SELECT_SCALE;
        }
      } else if (mode_ == UI_MODE_SELECT_SCALE) {
        state->x_scale = (state->x_scale + 1) % kNumScales;
      } else {
        if (!state->x_register_mode) {
          state->x_range = (state->x_range + 1) % 3;
        }
      }
      SaveState();
      break;
  }
}

void Ui::TerminateScaleRecording() {
  for (int i = 0; i < ADC_CHANNEL_LAST; ++i) {
    cv_reader_->mutable_channel(i)->UnlockPot();
  }
  mode_ = UI_MODE_NORMAL;
}

void Ui::NextCalibrationStep() {
  switch (mode_) {
    case UI_MODE_CALIBRATION_1:
      cv_reader_->CalibrateOffsets();
      cv_reader_->CalibrateRateC1();
      mode_ = UI_MODE_CALIBRATION_2;
      break;

    case UI_MODE_CALIBRATION_2:
      cv_reader_->CalibrateRateC3();
      mode_ = UI_MODE_CALIBRATION_3;
      break;

    case UI_MODE_CALIBRATION_3:
      cv_reader_->CalibrateSpreadC1();
      mode_ = UI_MODE_CALIBRATION_4;
      break;
    
    case UI_MODE_CALIBRATION_4:
      if (cv_reader_->CalibrateSpreadC3()) {
        settings_->SavePersistentData();
        mode_ = UI_MODE_NORMAL;
      } else {
        mode_ = UI_MODE_PANIC;
      }
      break;
      
    default:
      break;
  }
}

void Ui::UpdateHiddenParameters() {
  // Check if some pots have been moved.
  for (int i = 0; i < ADC_CHANNEL_LAST; ++i) {
    float new_value = cv_reader_->channel(i).unscaled_pot();
    float old_value = pot_value_[i];
    bool changed = fabs(new_value - old_value) >= 0.008f;
    if (changed) {
      pot_value_[i] = new_value;
      AlternateKnobMapping mapping = alternate_knob_mappings_[i];
      if (switches_.pressed(mapping.unlock_switch)) {
        if (mapping.unlock_switch == SWITCH_T_RANGE && new_value < 0.1f) {
          new_value = 0.0f;
        }
        *mapping.destination = static_cast<uint8_t>(new_value * 255.0f);
        cv_reader_->mutable_channel(i)->LockPot();

        // The next time a switch is released, we unlock the pots.
        setting_modification_flag_ = true;
      }
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
  
  UpdateHiddenParameters();
  
  if (queue_.idle_time() > 800 && mode_ == UI_MODE_PANIC) {
    mode_ = UI_MODE_NORMAL;
  }
  if (mode_ == UI_MODE_SELECT_SCALE) {
    if (queue_.idle_time() > 4000) {
      mode_ = UI_MODE_NORMAL;
      queue_.Touch();
    }
  } else if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }
}

uint8_t Ui::HandleFactoryTestingRequest(uint8_t command) {
  uint8_t argument = command & 0x1f;
  command = command >> 5;
  uint8_t reply = 0;
  switch (command) {
    case FACTORY_TESTING_READ_POT:
    case FACTORY_TESTING_READ_CV:
      reply = cv_reader_->adc_value(argument);
      break;
    
    case FACTORY_TESTING_READ_NORMALIZATION:
      reply = clock_inputs_->is_normalized(ClockInput(argument)) ? 255 : 0;
      break;      
    
    case FACTORY_TESTING_READ_GATE:
      reply = argument >= SWITCH_LAST
          ? clock_inputs_->value(ClockInput(argument - SWITCH_LAST))
          : switches_.pressed(Switch(argument));
      break;
      
    case FACTORY_TESTING_GENERATE_TEST_SIGNALS:
      output_test_mode_ = static_cast<bool>(argument);
      fill(
          &output_test_forced_dac_code_[0],
          &output_test_forced_dac_code_[4],
          0);
      break;
      
    case FACTORY_TESTING_CALIBRATE:
      if (argument == 0) {
        // Revert all settings before getting into calibration mode.
        settings_->mutable_state()->t_deja_vu = 0;
        settings_->mutable_state()->x_deja_vu = 0;
        settings_->mutable_state()->t_model = 0;
        settings_->mutable_state()->t_range = 1;
        settings_->mutable_state()->x_control_mode = 0;
        settings_->mutable_state()->x_range = 2;
        settings_->mutable_state()->x_register_mode = 0;
        settings_->SavePersistentData();
        
        mode_ = UI_MODE_CALIBRATION_1;
      } else {
        NextCalibrationStep();
      }
      {
        const CalibrationData& cal = settings_->calibration_data();
        float voltage = (argument & 1) == 0 ? 1.0f : 3.0f;
        for (int i = 0; i < 4; ++i) {
          output_test_forced_dac_code_[i] =  static_cast<uint16_t>(
              voltage * cal.dac_scale[i] + cal.dac_offset[i]);
        }
      }
      queue_.Touch();
      break;
      
    case FACTORY_TESTING_FORCE_DAC_CODE:
      {
        int channel = argument >> 2;
        int step = argument & 0x3;
        if (step == 0) {
          output_test_forced_dac_code_[channel] = 0xaf35;
        } else if (step == 1) {
          output_test_forced_dac_code_[channel] = 0x1d98;
        } else {
          CalibrationData* cal = settings_->mutable_calibration_data();
          cal->dac_offset[channel] = static_cast<float>(
              calibration_data_ & 0xffff);
          cal->dac_scale[channel] = static_cast<float>(
              calibration_data_ >> 16) * -0.125f;
          output_test_forced_dac_code_[channel] = static_cast<uint16_t>(cal->dac_scale[channel] + cal->dac_offset[channel]);
          settings_->SavePersistentData();
        }
      }
      break;
      
    case FACTORY_TESTING_WRITE_CALIBRATION_DATA_NIBBLE:
      calibration_data_ <<= 4;
      calibration_data_ |= argument & 0xf;
      break;
  }
  return reply;
}

}  // namespace marbles
