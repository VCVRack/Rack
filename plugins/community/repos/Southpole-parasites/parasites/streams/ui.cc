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

#include "streams/ui.h"

#include <algorithm>

#include "stmlib/system/storage.h"
#include "stmlib/system/system_clock.h"

#include "streams/drivers/adc.h"
#include "streams/processor.h"

namespace streams {

const int32_t kLongPressDuration = 1000;

using namespace std;
using namespace stmlib;

Storage<0x801fc00, 4> ui_settings_storage;

void Ui::Init(Adc* adc, CvScaler* cv_scaler, Processor* processor) {
  leds_.Init();
  switches_.Init();
  adc_ = adc;
  cv_scaler_ = cv_scaler;
  processor_ = processor;
  
  fill(&pot_value_[0], &pot_value_[kNumPots], 0);
  fill(&pot_threshold_[0], &pot_threshold_[kNumPots], 0);
  
  if (!ui_settings_storage.ParsimoniousLoad(&ui_settings_, &version_token_)) {
    // Flash is not formatted. Initialize.
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      ui_settings_.function[i] = PROCESSOR_FUNCTION_ENVELOPE;
      ui_settings_.alternate[i] = false;
    }
    ui_settings_.monitor_mode = MONITOR_MODE_OUTPUT;
    ui_settings_.linked = false;
  }
  
  // Initialize from settings in flash.
  monitor_mode_ = static_cast<MonitorMode>(ui_settings_.monitor_mode);
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    meter_[i].Init();
    processor_[i].set_alternate(ui_settings_.alternate[i]);
    processor_[i].set_linked(ui_settings_.linked);
    processor_[i].set_function(
        static_cast<ProcessorFunction>(ui_settings_.function[i]));
    display_mode_[i] = DISPLAY_MODE_MONITOR;
  }
  
  secret_handshake_counter_ = 0;
  factory_testing_ = switches_.pressed_immediate(SWITCH_MONITOR);
}

void Ui::SaveState() {
  ui_settings_.monitor_mode = monitor_mode_;
  ui_settings_.linked = processor_[0].linked();
  ui_settings_.function[0] = processor_[0].function();
  ui_settings_.function[1] = processor_[1].function();
  ui_settings_.alternate[0] = processor_[0].alternate();
  ui_settings_.alternate[1] = processor_[1].alternate();
  ui_settings_storage.ParsimoniousSave(ui_settings_, &version_token_);
}

void Ui::PaintAdaptive(uint8_t channel, int32_t sample, int32_t gain) {
  meter_[channel].Process(sample);
  if (meter_[channel].cv()) {
    sample = sample * lut_2164_gain[-gain >> 9] >> 15;
    leds_.PaintCv(channel, sample * 5 >> 2);
  } else {
    leds_.PaintPositiveBar(channel, wav_db[meter_[channel].peak() >> 7] + gain);
  }
}

void Ui::PaintMonitor(uint8_t channel) {
  switch (monitor_mode_) {
    case MONITOR_MODE_EXCITE_IN:
      PaintAdaptive(channel, cv_scaler_->excite_sample(channel), 0);
      break;
      
    case MONITOR_MODE_AUDIO_IN:
      PaintAdaptive(channel, cv_scaler_->audio_sample(channel), 0);
      break;
      
    case MONITOR_MODE_VCA_CV:
      leds_.PaintPositiveBar(channel, 32768 + cv_scaler_->gain_sample(channel));
      break;
    
    case MONITOR_MODE_OUTPUT:
      if (processor_[channel].function() == PROCESSOR_FUNCTION_COMPRESSOR) {
        leds_.PaintNegativeBar(channel, processor_[channel].gain_reduction());
      } else {
        PaintAdaptive(
            channel,
            cv_scaler_->audio_sample(channel),
            cv_scaler_->gain_sample(channel));
      }
      break;
      
    default:
      break;
  }
}

void Ui::PaintTestStatus() {
  int32_t value = 0;
  value += pot_value_[0] - 32768;
  value += pot_value_[1] - 32768;
  value += pot_value_[2] - 32768;
  value += pot_value_[3] - 32768;
  value -= cv_scaler_->excite_sample(0) > 0 ? 0 : cv_scaler_->excite_sample(0);
  value -= cv_scaler_->excite_sample(1) > 0 ? 0 : cv_scaler_->excite_sample(1);
  value -= cv_scaler_->gain_sample(0);
  value -= cv_scaler_->gain_sample(1);
  value -= cv_scaler_->audio_sample(0) - 12000;
  value -= cv_scaler_->audio_sample(1) - 12000;
  if (switches_.pressed(0)) value = -32767;
  if (switches_.pressed(1)) value = 32767;
  CONSTRAIN(value, -32767, 32767);
  if (value < 12288 && value > -12288) value = 0;
  uint8_t r = value >= 0 ? 0 : ((-1 - value) >> 7);
  uint8_t g = value > 0 ? (value >> 7) : 0;
  for (uint8_t i = 0; i < 8; ++i) {
    leds_.set(i, r, g);
  }
}

void Ui::PaintLeds() {
  leds_.Clear();
  
  if (calibrating_) {
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      uint8_t red, green;
      if (show_offset_level_ & (1 << i)) {
        int32_t gain_sample = cv_scaler_->raw_gain_sample(i);
        bool nulled = gain_sample > 59000;
        green = nulled ? 255 : 0;
        red = nulled ? 0 : 255;
      } else {
        red = 0;
        green = 255;
      }
      uint8_t pattern = i == 0 ? 255 : 9;
      for (uint8_t j = 0; j < 4; ++j) {
        bool on = (pattern & (1 << j)) != 0;
        leds_.set(i * 4 + j, on ? red : 0, on ? green : 0);
      }
    }
    return;
  }
  
  if (factory_testing_) {
    PaintTestStatus();
    return;
  }
  
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    uint8_t bank = i * 4;
    switch (display_mode_[i]) {
      case DISPLAY_MODE_FUNCTION:
        {
          bool alternate = processor_[i].alternate();
          uint8_t intensity = 255;
          if (processor_[i].linked()) {
            uint8_t phase = system_clock.milliseconds() >> 1;
            phase += i * 128;
            phase = phase < 128 ? phase : (255 - phase);
            intensity = (phase * 224 >> 7) + 32;
            intensity = intensity * intensity >> 8;
          }
          uint8_t function = processor_[i].function();
          if (function == PROCESSOR_FUNCTION_FILTER_CONTROLLER) {
            for (uint8_t j = 0; j < 4; ++j) {
              leds_.set(bank + j,
                  alternate ? intensity : 0,
                  alternate ? 0 : intensity);
            }
          } else if (function < PROCESSOR_FUNCTION_LORENZ_GENERATOR) {
            leds_.set(
                bank + function,
                alternate ? intensity : 0,
                alternate ? 0 : intensity);
          } else {
            uint8_t index = (processor_[i].last_gain() >> 4) * 5 >> 4;
            if (index > 3) index = 3;
            int16_t color = processor_[i].last_frequency();
            color = color - 128;
            color *= 2;
            if (color < 0) {
              if (color < -127) color = -127;
              leds_.set(bank + index, 255 + (color * 2), 255);
            } else {
              if (color > 127) color = 127;
              leds_.set(bank + index, 255, 255 - (color * 2));
            }
          }
        }
        break;
        
      case DISPLAY_MODE_MONITOR_FUNCTION:
        {
          uint8_t position = static_cast<uint8_t>(monitor_mode_);
          leds_.set(position * 2, 255, 0);
          leds_.set(position * 2 + 1, 255, 0);
        }
        break;
      
      case DISPLAY_MODE_MONITOR:
        PaintMonitor(i);
        break;
    }
  }
}

void Ui::Poll() {
  // SysTick is at 4kHz to get a fast bargraph refresh.
  ++divider_;
  if ((divider_ & 3) == 0) {
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
  
    adc_->ScanPots();
    for (uint8_t i = 0; i < kNumPots; ++i) {
      int32_t value = adc_->pot(i);
      int32_t current_value = pot_value_[i];
      if (value >= current_value + pot_threshold_[i] ||
          value <= current_value - pot_threshold_[i] ||
          !pot_threshold_[i]) {
        Event e;
        e.control_id = i;
        e.data = value;
        queue_.AddEvent(CONTROL_POT, i, e.data);
        pot_value_[i] = value;
        pot_threshold_[i] = 256;
      }
    }
  }
  PaintLeds();
  leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::Link(uint8_t index) {
  if (processor_[0].linked()) {
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      if (i != index) {
        display_mode_[i] = display_mode_[index];
        processor_[i].set_function(processor_[index].function());
        processor_[i].set_alternate(processor_[index].alternate());
      }
    }
  }
}

void Ui::OnPotMoved(const Event& e) {
  if (calibrating_) {
    if ((e.control_id & 1) == 0) {
      int32_t min = kDefaultOffset >> 1;
      int32_t max = 3 * min + 256;
      int32_t value = min + ((max - min) * e.data >> 16);
      cv_scaler_->set_dac_offset(e.control_id >> 1, value);
      show_offset_level_ |= (1 << (e.control_id >> 1));
    }
  } else {
    processor_[0].set_global(e.control_id, e.data);
    processor_[1].set_global(e.control_id, e.data);
    processor_[e.control_id >> 1].set_parameter(e.control_id & 1, e.data);
  }
}

void Ui::OnSwitchPressed(const Event& e) {
  if (factory_testing_) {
    if (e.control_id == SWITCH_MONITOR) {
      ++secret_handshake_counter_;
      if (secret_handshake_counter_ == 4) {
        factory_testing_ = false;
      }
    }
    return;
  }
  if (calibrating_) {
    cv_scaler_->SaveCalibrationData();
    calibrating_ = false;
    show_offset_level_ = 0;
    return;
  }
  // Double press!
  if ((e.control_id == SWITCH_MODE_1 && press_time_[SWITCH_MODE_2]) ||
      (e.control_id == SWITCH_MODE_2 && press_time_[SWITCH_MODE_1])) {
    press_time_[SWITCH_MODE_1] = press_time_[SWITCH_MODE_2] = 0;
    bool linked = !processor_[0].linked();
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      display_mode_[i] = DISPLAY_MODE_FUNCTION;
      processor_[i].set_linked(linked);
    }
    Link(1 - e.control_id);
    return;
  }
  
  switch (e.control_id) {
    case SWITCH_MONITOR:
      {
        if (display_mode_[0] == DISPLAY_MODE_MONITOR &&
            display_mode_[1] == DISPLAY_MODE_MONITOR) {
          display_mode_[0] = display_mode_[1] = DISPLAY_MODE_MONITOR_FUNCTION;
        } else if (display_mode_[0] == DISPLAY_MODE_MONITOR_FUNCTION &&
                   display_mode_[1] == DISPLAY_MODE_MONITOR_FUNCTION) {
          monitor_mode_ = static_cast<MonitorMode>(monitor_mode_ + 1);
          if (monitor_mode_ == MONITOR_MODE_LAST) {
            monitor_mode_ = static_cast<MonitorMode>(0);
          }
          SaveState();
        } else {
          display_mode_[0] = display_mode_[1] = DISPLAY_MODE_MONITOR;
        }
      }
      break;
      
    default:
      break;
  }
}

void Ui::OnSwitchReleased(const Event& e) {
  if (factory_testing_) {
    return;
  }
  
  // Detect secret handshake for easter egg...
  uint8_t secret_handshake_code = e.control_id;
  secret_handshake_code |= e.data >= kLongPressDuration ? 2 : 0;
  if ((secret_handshake_counter_ & 3) == secret_handshake_code) {
    ++secret_handshake_counter_;
    if (secret_handshake_counter_ == 16) {
      for (uint8_t i = 0; i < kNumChannels; ++i) {
        processor_[i].set_alternate(false);
        processor_[i].set_function(PROCESSOR_FUNCTION_LORENZ_GENERATOR);
      }
      SaveState();
      secret_handshake_counter_ = 0;
      return;
    }
  } else {
    secret_handshake_counter_ = 0;
  }
  
  if (e.data >= kLongPressDuration) {
    // Handle long presses.
    switch (e.control_id) {
      case SWITCH_MONITOR:
        calibrating_ = cv_scaler_->can_calibrate();
        if (calibrating_) {
          cv_scaler_->CaptureAdcOffsets();
          show_offset_level_ = 0;
        }
        break;
        
      case SWITCH_MODE_1:
      case SWITCH_MODE_2:
        {
          processor_[e.control_id].set_alternate(
              !processor_[e.control_id].alternate());
          if (processor_[e.control_id].function() >
              PROCESSOR_FUNCTION_COMPRESSOR) {
            processor_[e.control_id].set_function(PROCESSOR_FUNCTION_ENVELOPE);
          }
          display_mode_[e.control_id] = DISPLAY_MODE_FUNCTION;
          int32_t other = 1 - e.control_id;
          if (display_mode_[other] == DISPLAY_MODE_MONITOR_FUNCTION) {
            display_mode_[other] = DISPLAY_MODE_MONITOR;
          }
          Link(e.control_id);
          SaveState();
        }
        break;
    }
  } else {
    switch (e.control_id) {
      case SWITCH_MODE_1:
      case SWITCH_MODE_2:
        {
          if (display_mode_[e.control_id] == DISPLAY_MODE_FUNCTION) {
            ProcessorFunction index = processor_[e.control_id].function();
            index = static_cast<ProcessorFunction>(index + 1);
            ProcessorFunction limit = processor_[e.control_id].alternate()
                ? PROCESSOR_FUNCTION_FILTER_CONTROLLER
                : PROCESSOR_FUNCTION_COMPRESSOR;
            if (index > limit) {
              index = static_cast<ProcessorFunction>(0);
            }
            processor_[e.control_id].set_function(index);
            SaveState();
          } else {
            display_mode_[e.control_id] = DISPLAY_MODE_FUNCTION;
            int32_t other = 1 - e.control_id;
            if (display_mode_[other] == DISPLAY_MODE_MONITOR_FUNCTION) {
              display_mode_[other] = DISPLAY_MODE_MONITOR;
            }
          }
          Link(e.control_id);
        }
        break;
      
      default:
        break;
    }
  }
}

void Ui::DoEvents() {
  bool refresh = false;
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_SWITCH) {
      if (e.data == 0) {
        OnSwitchPressed(e);
      } else {
        OnSwitchReleased(e);
      }
    } else if (e.control_type == CONTROL_POT) {
      OnPotMoved(e);
    }
    refresh = true;
  }
  if (queue_.idle_time() > 1000) {
    queue_.Touch();
    if (display_mode_[0] == DISPLAY_MODE_MONITOR_FUNCTION && 
        display_mode_[1] == DISPLAY_MODE_MONITOR_FUNCTION) {
       display_mode_[0] = display_mode_[1] = DISPLAY_MODE_MONITOR;   
    }
    refresh = true;
  }
  
  // Recompute processor parameters if necessary.
  if (refresh) {
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      processor_[i].Configure();
    }
  }
}

}  // namespace streams
