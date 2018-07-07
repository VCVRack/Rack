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

#ifndef EDGES_UI_H_
#define EDGES_UI_H_

#include "avrlibx/avrlibx.h"

#include "edges/hardware_config.h"

namespace edges {

enum Mode {
  MODE_NORMAL,
  MODE_MENU,
  MODE_CALIBRATE_1,
  MODE_CALIBRATE_2,
  MODE_RECORDING,
  MODE_LEARNING_MIDI_CHANNEL
};

static const uint8_t kNumSwitches = 6;

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init();
  
  static void set_gate(uint8_t gate) { gate_ = gate; }
  static void set_cv(uint8_t channel, uint16_t cv) {
    if (mode_ >= MODE_CALIBRATE_1 &&
        mode_ <= MODE_CALIBRATE_2 &&
        channel == edited_channel_) {
      calibration_cv_[mode_ - MODE_CALIBRATE_1] = cv;
    }
    cv_[channel] = cv;
  }
  
  static uint16_t cv(uint8_t channel) {
    if (mode_ == MODE_RECORDING && channel == edited_channel_) {
      return root_cv_;
    } else {
      return cv_[channel];
    }
  }
  
  static void Poll();
  static uint8_t gate() { return gate_; }
  static Mode mode() { return mode_; }
  
 private:
  static void OnSwitchHeld(uint8_t index);
  static void OnSwitchReleased(uint8_t index);
   
  static uint8_t edited_channel_;
  static Mode mode_;
  static uint8_t gate_;
  
  // Used for dimming LEDs.
  static uint16_t leds_pwm_counter_;
  
  // Used for detecting long switch pressed.
  static uint16_t switch_time_counter_;
  
  static uint16_t calibration_cv_[2];
  static uint16_t root_cv_;
  static uint16_t cv_[2 * kNumChannels];
  
  static Leds leds_;
  static Switches switches_;
  static Gpio<PortC, 1> midi_learn_switch_;
  static Gpio<PortC, 3> midi_mode_switch_;

  static uint8_t debounce_history_[kNumSwitches];
  
  DISALLOW_COPY_AND_ASSIGN(Ui);
};

extern Ui ui;

}  // namespace edges

#endif  // EDGES_UI_H_
