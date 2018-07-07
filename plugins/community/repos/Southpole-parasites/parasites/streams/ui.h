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

#ifndef STREAMS_UI_H_
#define STREAMS_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/ui/event_queue.h"

#include "streams/audio_cv_meter.h"
#include "streams/cv_scaler.h"
#include "streams/drivers/leds.h"
#include "streams/drivers/switches.h"

namespace streams {

enum DisplayMode {
  DISPLAY_MODE_FUNCTION,
  DISPLAY_MODE_MONITOR,
  DISPLAY_MODE_MONITOR_FUNCTION
};

enum MonitorMode {
  MONITOR_MODE_EXCITE_IN,
  MONITOR_MODE_VCA_CV,
  MONITOR_MODE_AUDIO_IN,
  MONITOR_MODE_OUTPUT,
  MONITOR_MODE_LAST
};

enum Switch {
  SWITCH_MODE_1,
  SWITCH_MODE_2,
  SWITCH_MONITOR,
  SWITCH_LAST
};

struct UiSettings {
  uint8_t function[kNumChannels];
  uint8_t alternate[kNumChannels];
  uint8_t monitor_mode;
  uint8_t linked;
  uint8_t padding[2];
};

class Processor;

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Adc* adc, CvScaler* cv_scaler, Processor* processors);
  void Poll();
  void DoEvents();
  void FlushEvents();
  
  inline bool calibrating() const {
    return calibrating_;
  }

 private:
  // Synchronize UI state of channels 1 & 2.
  void Link(uint8_t channel);
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void OnPotMoved(const stmlib::Event& e);
  void SaveState();
  void PaintLeds();
  void PaintTestStatus();
  void PaintMonitor(uint8_t channel);
  void PaintAdaptive(uint8_t channel, int32_t sample, int32_t db_offset);

  stmlib::EventQueue<16> queue_;
  
  Adc* adc_;
  CvScaler* cv_scaler_;
  Processor* processor_;
  Leds leds_;
  Switches switches_;
  uint32_t press_time_[kNumSwitches];
  DisplayMode display_mode_[kNumChannels];
  
  MonitorMode monitor_mode_;
  
  bool calibrating_;
  bool factory_testing_;
  uint8_t show_offset_level_;
  
  UiSettings ui_settings_;
  uint16_t version_token_;
  
  uint8_t secret_handshake_counter_;
  
  int32_t pot_value_[kNumPots];
  int32_t pot_threshold_[kNumPots];
  
  AudioCvMeter meter_[kNumChannels];
  
  uint8_t divider_;
  
  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace streams

#endif  // STREAMS_UI_H_
