// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (olivier@mutable-instruments.net)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// User interface handling.

#include "edges/ui.h"

#include <string.h>

#include "edges/midi_handler.h"
#include "edges/settings.h"

namespace edges {

const uint16_t kLongPressTime = 600;  // ms

using namespace avrlibx;

/* <static> */
Mode Ui::mode_;
uint8_t Ui::gate_;
uint8_t Ui::edited_channel_;
uint16_t Ui::leds_pwm_counter_;
uint16_t Ui::switch_time_counter_;
uint16_t Ui::calibration_cv_[2];
uint16_t Ui::cv_[2 * kNumChannels];
uint16_t Ui::root_cv_;

Leds Ui::leds_;
Switches Ui::switches_;
Gpio<PortC, 1> Ui::midi_learn_switch_;
Gpio<PortC, 3> Ui::midi_mode_switch_;

uint8_t Ui::debounce_history_[kNumSwitches];
/* </static> */

/* static */
void Ui::Init() {
  leds_.set_direction(OUTPUT);
  switches_.set_direction(INPUT);
  midi_learn_switch_.set_direction(INPUT);
  midi_mode_switch_.set_direction(INPUT);

  switches_.set_mode(PORT_MODE_PULL_UP);
  midi_learn_switch_.set_mode(PORT_MODE_PULL_UP);
  midi_mode_switch_.set_mode(PORT_MODE_PULL_UP);
  mode_ = MODE_NORMAL;
  leds_pwm_counter_ = 0;
  edited_channel_ = 0;
  memset(debounce_history_, 0xff, sizeof(debounce_history_));
  memset(cv_, 0, sizeof(cv_));
}

/* static */
void Ui::OnSwitchHeld(uint8_t index) {
  if (index < kNumChannels) {
    if (mode_ == MODE_MENU) {
      mode_ = MODE_NORMAL;
      settings.Save();
    } else {
      mode_ = MODE_MENU;
      edited_channel_ = index;
    }
  } else if (index == kNumChannels + 1) {
    midi_handler.DisableMidiCoupling();
  }
}

/* static */
void Ui::OnSwitchReleased(uint8_t index) {
  if (index < kNumChannels) {
    switch (mode_) {
      case MODE_MENU:
        switch (index) {
          case 0:
            settings.ToggleQuantizer(edited_channel_);
            break;
            
          case 1:
            settings.ToggleArpeggio(edited_channel_);
            break;
            
          case 2:
            mode_ = MODE_RECORDING;
            root_cv_ = cv_[edited_channel_];
            settings.mutable_channel_data(edited_channel_)->StartRecording();
            break;
            
          case 3:
            mode_ = MODE_CALIBRATE_1;
            break;
        }
        break;

      case MODE_NORMAL:
        settings.StepPW(index);
        break;
        
      case MODE_CALIBRATE_1:
        mode_ = MODE_CALIBRATE_2;
        break;
        
      case MODE_CALIBRATE_2:
        settings.Calibrate(
            edited_channel_,
            calibration_cv_[0],
            calibration_cv_[1],
            cv_[edited_channel_ + 4]);
        mode_ = MODE_NORMAL;
        break;
        
      case MODE_RECORDING:
        {
          ChannelData* channel = settings.mutable_channel_data(edited_channel_);
          uint8_t current_step = channel->num_arpeggio_steps - 1;
          if (index == (current_step & 3)) {
            channel->StopRecording();
            settings.Save();
            mode_ = MODE_NORMAL;
          } else if (index == ((current_step + 1) & 3)) {
            if (!channel->NextArpeggiatorStep()) {
              settings.Save();
              mode_ = MODE_NORMAL;
            }
          }
        }
        break;
    }
  } else if (index == kNumChannels) {
    mode_ = MODE_LEARNING_MIDI_CHANNEL;
    midi_handler.Learn();
  } else {
    midi_handler.ToggleMidiMode();
  }
}

static const uint8_t bit_reverse[] = {
  0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};

/* static */
void Ui::Poll() {
  ++leds_pwm_counter_;
  ++switch_time_counter_;
  
  // Refresh leds.
  uint8_t leds_value = 0;
  switch (mode_) {
    case MODE_NORMAL:
      leds_value = gate_;
      break;
      
    case MODE_MENU:
      if (settings.quantized(edited_channel_)) {
        leds_value |= 0x1;
      }
      if (settings.arpeggio(edited_channel_)) {
        leds_value |= 0x2;
      }
      if (leds_pwm_counter_ & 128) {
        leds_value |= 0x4;
      } else {
        leds_value |= 0x8;
      }
      break;
      
    case MODE_LEARNING_MIDI_CHANNEL:
      if (leds_pwm_counter_ & 128) {
        leds_value = 0xff;
      }
      if (!midi_handler.learning()) {
        mode_ = MODE_NORMAL;
      }
      break;
      
    case MODE_CALIBRATE_1:
      leds_value |= 0x3;
      break;
      
    case MODE_CALIBRATE_2:
      leds_value |= 0xf;
      break;
      
    case MODE_RECORDING:
      leds_value |= 1 << ((settings.num_steps(edited_channel_) - 1) & 0x03);
      break;
  }
  leds_.set_value(bit_reverse[leds_value & 0xf]);
  
  // Scan switches.
  uint8_t switches_value = bit_reverse[switches_.Read()];
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    debounce_history_[i] = (debounce_history_[i] << 1) | (switches_value & 1);
    switches_value >>= 1;
  }
  debounce_history_[kNumChannels] = \
      (debounce_history_[kNumChannels] << 1) | midi_learn_switch_.value();
  debounce_history_[kNumChannels + 1] = \
      (debounce_history_[kNumChannels + 1] << 1) | midi_mode_switch_.value();
    
  // Trigger switch events.
  for (uint8_t i = 0; i < kNumSwitches; ++i) {
    // When a switch is pressed, start the time counter.
    if (debounce_history_[i] == 0xfe) {
      switch_time_counter_ = 0;
    }
    
    // When a switch is held, enable calibration mode.
    if (debounce_history_[i] == 0x00 &&
        switch_time_counter_ == kLongPressTime) {
      OnSwitchHeld(i);
    }
    
    // When a switch is released, do something depending on the event.
    if (debounce_history_[i] == 0x01 &&
        switch_time_counter_ < kLongPressTime) {
      OnSwitchReleased(i);
    }
  }
  
  // Update arpeggiator pattern.
  if (mode_ == MODE_RECORDING) {
    if (settings.num_steps(edited_channel_) == 1) {
      root_cv_ = cv_[edited_channel_];
    }
    settings.mutable_channel_data(edited_channel_)->UpdateArpeggiatorStep(
        root_cv_,
        cv_[edited_channel_]
    );
  }
}

/* extern */
Ui ui;

}  // namespace edges
