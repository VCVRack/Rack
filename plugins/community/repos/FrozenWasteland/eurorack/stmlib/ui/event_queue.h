// Copyright 2011 Olivier Gillet.
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
// Event queue.

#ifndef STMLIB_UI_EVENT_QUEUE_H_
#define STMLIB_UI_EVENT_QUEUE_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"
#include "stmlib/system/system_clock.h"

namespace stmlib {

enum ControlType {
  CONTROL_POT = 0,
  CONTROL_ENCODER = 1,
  CONTROL_ENCODER_CLICK = 2,
  CONTROL_ENCODER_LONG_CLICK = 3,
  CONTROL_SWITCH = 4,
  CONTROL_SWITCH_HOLD = 5,
  CONTROL_REFRESH = 0xff
};

struct Event {
  ControlType control_type;
  uint16_t control_id;
  int32_t data;
};

template<uint16_t size = 32>
class EventQueue {
 public:
  EventQueue() { }
  
  void Init() {
    events_.Init();
  }
  
  void Flush() {
    events_.Flush();
  };
  
  void AddEvent(ControlType control_type, uint16_t id, int32_t data) {
    Event e;
    e.control_type = control_type;
    e.control_id = id;
    e.data = data;
    events_.Overwrite(e);
    Touch();
  }
  
  void Touch() {
    last_event_time_ = system_clock.milliseconds();
  }
  
  size_t available() {
    return events_.readable();
  }
  
  uint32_t idle_time() {
    uint32_t now = system_clock.milliseconds();
    return now - last_event_time_;
  }
  
  Event PullEvent() {
    return events_.ImmediateRead();
  }
  
 private:
  uint32_t last_event_time_;
  RingBuffer<Event, size> events_;
};

}  // namespace stmlib

#endif  // STMLIB_UI_EVENT_QUEUE_H_
