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

#include "peaks/ui.h"

#include "stmlib/system/storage.h"

#include <algorithm>

namespace peaks {

using namespace std;
using namespace stmlib;

const uint16_t kAdcThresholdUnlocked = 1 << (16 - 10);  // 10 bits
const uint16_t kAdcThresholdLocked = 1 << (16 - 8);  // 8 bits
const int32_t kLongPressDuration = 600;

/* static */
const ProcessorFunction Ui::function_table_[FUNCTION_LAST][2] = {
  { PROCESSOR_FUNCTION_ENVELOPE, PROCESSOR_FUNCTION_ENVELOPE },
  { PROCESSOR_FUNCTION_LFO, PROCESSOR_FUNCTION_LFO },
  { PROCESSOR_FUNCTION_TAP_LFO, PROCESSOR_FUNCTION_TAP_LFO },
  { PROCESSOR_FUNCTION_BASS_DRUM, PROCESSOR_FUNCTION_SNARE_DRUM },

  { PROCESSOR_FUNCTION_MINI_SEQUENCER, PROCESSOR_FUNCTION_MINI_SEQUENCER },
  { PROCESSOR_FUNCTION_PULSE_SHAPER, PROCESSOR_FUNCTION_PULSE_SHAPER },
  { PROCESSOR_FUNCTION_PULSE_RANDOMIZER, PROCESSOR_FUNCTION_PULSE_RANDOMIZER },
  { PROCESSOR_FUNCTION_FM_DRUM, PROCESSOR_FUNCTION_FM_DRUM },
};

Storage<0x8020000, 16> storage;

void Ui::Init() {
  leds_.Init();
  switches_.Init();
  adc_.Init();
  system_clock.Tick();

  fill(&adc_value_[0], &adc_value_[kNumAdcChannels], 0);
  fill(&adc_threshold_[0], &adc_threshold_[kNumAdcChannels], 0);
  fill(&snapped_[0], &snapped_[kNumAdcChannels], false);
  panel_gate_state_ = 0;
  
  if (!storage.ParsimoniousLoad(&settings_, &version_token_)) {
    edit_mode_ = EDIT_MODE_TWIN;
    function_[0] = FUNCTION_ENVELOPE;
    function_[1] = FUNCTION_ENVELOPE;
    settings_.snap_mode = false;
  } else {
    edit_mode_ = static_cast<EditMode>(settings_.edit_mode);
    function_[0] = static_cast<Function>(settings_.function[0]);
    function_[1] = static_cast<Function>(settings_.function[1]);
    copy(&settings_.pot_value[0], &settings_.pot_value[8], &pot_value_[0]);
    
    if (edit_mode_ == EDIT_MODE_FIRST || edit_mode_ == EDIT_MODE_SECOND) {
      LockPots();
      for (uint8_t i = 0; i < 4; ++i) {
        processors[0].set_parameter(
            i,
            static_cast<uint16_t>(pot_value_[i]) << 8);
        processors[1].set_parameter(
            i,
            static_cast<uint16_t>(pot_value_[i + 4]) << 8);
      }
    }
  }
  
  if (switches_.pressed_immediate(SWITCH_TWIN_MODE)) {
    settings_.snap_mode = !settings_.snap_mode;
    SaveState();
  }
  
  ChangeControlMode();
  SetFunction(0, function_[0]);
  SetFunction(1, function_[1]);
  double_press_counter_ = 0;
}

void Ui::LockPots() {
  fill(
      &adc_threshold_[0],
      &adc_threshold_[kNumAdcChannels],
      kAdcThresholdLocked);
  fill(&snapped_[0], &snapped_[kNumAdcChannels], false);
}

void Ui::SaveState() {
  settings_.edit_mode = edit_mode_;
  settings_.function[0] = function_[0];
  settings_.function[1] = function_[1];
  copy(&pot_value_[0], &pot_value_[8], &settings_.pot_value[0]);
  storage.ParsimoniousSave(settings_, &version_token_);
}

inline void Ui::RefreshLeds() {
  uint8_t flash = (system_clock.milliseconds() >> 7) & 7;
  switch (edit_mode_) {
    case EDIT_MODE_FIRST:
      leds_.set_twin_mode(flash == 1);
      break;
    case EDIT_MODE_SECOND:
      leds_.set_twin_mode(flash == 1 || flash == 3);
      break;
    default:
      leds_.set_twin_mode(edit_mode_ & 1);
      break;
  }
  if ((system_clock.milliseconds() & 256) &&
      function() >= FUNCTION_FIRST_ALTERNATE_FUNCTION) {
    leds_.set_function(4);
  } else {
    leds_.set_function(function() & 3);
  }
  
  uint8_t b[2];
  for (uint8_t i = 0; i < 2; ++i) {
    switch (function_[i]) {
      case FUNCTION_DRUM_GENERATOR:
      case FUNCTION_FM_DRUM_GENERATOR:
        b[i] = abs(brightness_[i]) >> 8;
        b[i] = b[i] > 255 ? 255 : b[i];
        break;
      case FUNCTION_LFO:
      case FUNCTION_TAP_LFO:
      case FUNCTION_MINI_SEQUENCER:
        b[i] = static_cast<uint16_t>(brightness_[i] + 32768) >> 8;
        break;
      default:
        b[i] = brightness_[i] >> 7;
        break;
    }
  }
  
  if (processors[0].function() == PROCESSOR_FUNCTION_NUMBER_STATION) {
    leds_.set_pattern(
        processors[0].number_station().digit() ^ \
        processors[1].number_station().digit());
    b[0] = processors[0].number_station().gate() ? 255 : 0;
    b[1] = processors[1].number_station().gate() ? 255 : 0;
  }
  
  leds_.set_levels(b[0], b[1]);
}

void Ui::PollPots() {
  for (uint8_t i = 0; i < kNumAdcChannels; ++i) {
    int32_t value = adc_.value(i);
    int32_t current_value = static_cast<int32_t>(adc_value_[i]);
    if (value >= current_value + adc_threshold_[i] ||
        value <= current_value - adc_threshold_[i] ||
        !adc_threshold_[i]) {
      Event e;
      e.control_id = i;
      e.data = value;
      OnPotChanged(e);
      adc_value_[i] = value;
      adc_threshold_[i] = kAdcThresholdUnlocked;
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
    if (switches_.pressed(i) && press_time_[i] != 0 && i < SWITCH_GATE_TRIG_1) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];
      if (pressed_time > kLongPressDuration) {
        if (switches_.pressed(1 - i)) {
          ++double_press_counter_;
          press_time_[0] = press_time_[1] = 0;
          if (double_press_counter_ == 3) {
            double_press_counter_ = 0;
            processors[0].set_function(PROCESSOR_FUNCTION_NUMBER_STATION);
            processors[1].set_function(PROCESSOR_FUNCTION_NUMBER_STATION);
          }
        } else {
          queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
          press_time_[i] = 0;  // Inhibit next release event
        }
      }
    }
    if (switches_.released(i) && press_time_[i] != 0) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
    }
  }
  
  RefreshLeds();
  leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {
  switch (e.control_id) {
    case SWITCH_TWIN_MODE:
      break;
      
    case SWITCH_FUNCTION:
      break;
      
    case SWITCH_GATE_TRIG_1:
      panel_gate_control_[0] = true;
      break;
    
    case SWITCH_GATE_TRIG_2:
      panel_gate_control_[1] = true;
      break;
  }
}

void Ui::ChangeControlMode() {
  if (edit_mode_ == EDIT_MODE_SPLIT) {
    processors[0].CopyParameters(&adc_value_[0], 2);
    processors[1].CopyParameters(&adc_value_[2], 2);
    processors[0].set_control_mode(CONTROL_MODE_HALF);
    processors[1].set_control_mode(CONTROL_MODE_HALF);
  } else if (edit_mode_ == EDIT_MODE_TWIN) {
    processors[0].CopyParameters(&adc_value_[0], 4);
    processors[1].CopyParameters(&adc_value_[0], 4);
    processors[0].set_control_mode(CONTROL_MODE_FULL);
    processors[1].set_control_mode(CONTROL_MODE_FULL);
  } else {
    processors[0].set_control_mode(CONTROL_MODE_FULL);
    processors[1].set_control_mode(CONTROL_MODE_FULL);
  }
}

void Ui::SetFunction(uint8_t index, Function f) {
  if (edit_mode_ == EDIT_MODE_SPLIT || edit_mode_ == EDIT_MODE_TWIN) {
    function_[0] = function_[1] = f;
    processors[0].set_function(function_table_[f][0]);
    processors[1].set_function(function_table_[f][1]);
  } else {
    function_[index] = f;
    processors[index].set_function(function_table_[f][index]);
  }
}

void Ui::OnSwitchReleased(const Event& e) {
  switch (e.control_id) {
    case SWITCH_TWIN_MODE:
      if (e.data > kLongPressDuration) {
        edit_mode_ = static_cast<EditMode>(
            (edit_mode_ + EDIT_MODE_FIRST) % EDIT_MODE_LAST);
        function_[0] = function_[1];
        processors[0].set_function(function_table_[function_[0]][0]);
        processors[1].set_function(function_table_[function_[0]][1]);
        LockPots();
      } else {
        if (edit_mode_ <= EDIT_MODE_SPLIT) {
          edit_mode_ = static_cast<EditMode>(EDIT_MODE_SPLIT - edit_mode_);
        } else {
          edit_mode_ = static_cast<EditMode>(EDIT_MODE_SECOND - (edit_mode_ & 1));
          LockPots();
        }
      }
      
      ChangeControlMode();
      SaveState();
      break;
      
    case SWITCH_FUNCTION:
      {
        Function f = function();
        if (e.data > kLongPressDuration) {
          f = static_cast<Function>((f + FUNCTION_FIRST_ALTERNATE_FUNCTION) % FUNCTION_LAST);
        } else {
          if (f <= FUNCTION_DRUM_GENERATOR) {
            f = static_cast<Function>((f + 1) & 3);
          } else {
            f = static_cast<Function>(((f + 1) & 3) + FUNCTION_FIRST_ALTERNATE_FUNCTION);
          }
        }
        SetFunction(edit_mode_ - EDIT_MODE_FIRST, f);
        SaveState();
      }
      break;
      
    case SWITCH_GATE_TRIG_1:
      panel_gate_control_[0] = false;
      break;

    case SWITCH_GATE_TRIG_2:
      panel_gate_control_[1] = false;
      break;
  }
}

void Ui::OnPotChanged(const Event& e) {
  switch (edit_mode_) {
    case EDIT_MODE_TWIN:
      processors[0].set_parameter(e.control_id, e.data);
      processors[1].set_parameter(e.control_id, e.data);
      pot_value_[e.control_id] = e.data >> 8;
      break;
    case EDIT_MODE_SPLIT:
      if (e.control_id < 2) {
        processors[0].set_parameter(e.control_id, e.data);
      } else {
        processors[1].set_parameter(e.control_id - 2, e.data);
      }
      pot_value_[e.control_id] = e.data >> 8;
      break;
    case EDIT_MODE_FIRST:
    case EDIT_MODE_SECOND:
      {
        uint8_t index = e.control_id + (edit_mode_ - EDIT_MODE_FIRST) * 4;
        Processors* p = &processors[edit_mode_ - EDIT_MODE_FIRST];
        
        int16_t delta = static_cast<int16_t>(pot_value_[index]) - \
            static_cast<int16_t>(e.data >> 8);
        if (delta < 0) {
          delta = -delta;
        }
        
        if (!settings_.snap_mode || snapped_[e.control_id] || delta <= 2) {
          p->set_parameter(e.control_id, e.data);
          pot_value_[index] = e.data >> 8;
          snapped_[e.control_id] = true;
        }
      }
      break;
    case EDIT_MODE_LAST:
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
        OnSwitchReleased(e);
      }
    } else if (e.control_type == CONTROL_POT) {
      OnPotChanged(e);
    }
  }
  if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }
}

}  // namespace peaks
