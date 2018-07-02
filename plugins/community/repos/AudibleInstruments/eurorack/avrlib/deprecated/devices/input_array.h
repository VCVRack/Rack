// Copyright 2009 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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
// -----------------------------------------------------------------------------
//
// Controller for an array of analog or digital inputs behind a multiplexer
// controlled by a shift register. Stores an array reflecting the current value
// of each input. A readout returns an "Event" object with the latest value of
// the controller, a flag indicating how it changed with respect to the
// previous value, and a timestamp.
//
// When event = EVENT_NONE, the time is the idle time (how many ms since an
// event occurred).
// When event != EVENT_NONE, the time is the time spent in the previous state.

#ifndef AVRLIB_DEVICES_INPUT_ARRAY_H_
#define AVRLIB_DEVICES_INPUT_ARRAY_H_

#include "avrlib/size_to_type.h"
#include "avrlib/time.h"

namespace avrlib {

enum InputEvent {
  EVENT_NONE = 0,
  EVENT_CHANGED = 1,
};

template<typename Input, uint8_t num_inputs, uint8_t low_threshold = 8>
class InputArray {
 public:
  InputArray() { }
  typedef typename DataTypeForSize<Input::data_size>::Type T;
  typedef struct {
    uint8_t id;
    uint8_t event;  // Could have been InputEvent but I want the extra byte.
    T value;
    uint32_t time;
  } Event;
  static void Init() {
    // No need to initialize anything - the first cycle of readouts will take
    // care of this.
    active_input_ = 0;
    starting_up_ = num_inputs * 2;
    Input::Init();
  }
  static void Lock(uint16_t threshold) {
    for (uint8_t i = 0; i < num_inputs; ++i) {
      thresholds_[i] = threshold;
    }
  }
  static void Touch() {
    last_event_time_ = milliseconds();
  }
  static Event Read() {
    Event e;
    e.id = active_input_;

    // Read a value from the ADC and check if something occurred.
    e.value = Input::Read(active_input_);
    uint8_t same;
    int16_t delta = static_cast<int16_t>(values_[active_input_]) - 
        static_cast<int16_t>(e.value);
    if (delta < 0) {
      delta = -delta;
    }
    same = delta < thresholds_[active_input_];
    uint32_t now = milliseconds();
    e.time = now - last_event_time_;
    if (same) {
      e.event = EVENT_NONE;
    } else {
      // Since the input has been touched and the event has been recorded,
      // lower the threshold.
      thresholds_[active_input_] = low_threshold;
      values_[active_input_] = e.value;
      last_event_time_ = now;
      e.event = EVENT_CHANGED;
    }

    // The next call to Read() will read the next input.
    ++active_input_;
    if (active_input_ == num_inputs) {
      active_input_ = 0;
    }

    // During the first cycle, do not raise any event - just record the values.
    if (starting_up_) {
      --starting_up_;
      e.event = EVENT_NONE;
      e.time = 0;
    }
    return e;
  }
  static uint8_t active_input() { return active_input_; }

 private:
  static T values_[num_inputs];
  static T thresholds_[num_inputs];
  static uint8_t active_input_;
  static uint8_t starting_up_;
  static uint32_t last_event_time_;

  DISALLOW_COPY_AND_ASSIGN(InputArray);
};

template<typename Input, uint8_t num_inputs, uint8_t low_threshold>
typename InputArray<Input, num_inputs, low_threshold>::T
InputArray<Input, num_inputs, low_threshold>::values_[num_inputs];

template<typename Input, uint8_t num_inputs, uint8_t low_threshold>
typename InputArray<Input, num_inputs, low_threshold>::T
InputArray<Input, num_inputs, low_threshold>::thresholds_[num_inputs];

template<typename Input, uint8_t num_inputs, uint8_t low_threshold>
uint8_t InputArray<Input, num_inputs, low_threshold>::active_input_;

template<typename Input, uint8_t num_inputs, uint8_t low_threshold>
uint32_t InputArray<Input, num_inputs, low_threshold>::last_event_time_;

template<typename Input, uint8_t num_inputs, uint8_t low_threshold>
uint8_t InputArray<Input, num_inputs, low_threshold>::starting_up_;

}  // namespace avrlib

#endif   // AVRLIB_DEVICES_INPUT_ARRAY_H_
