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
// Settings

#ifndef FRAMES_UI_H_
#define FRAMES_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/ui/event_queue.h"

#include "frames/drivers/adc.h"
#include "frames/drivers/channel_leds.h"
#include "frames/drivers/factory_testing_switch.h"
#include "frames/drivers/keyframe_led.h"
#include "frames/drivers/rgb_led.h"
#include "frames/drivers/switches.h"

namespace frames {

class Keyframer;
class PolyLfo;

enum SwitchIndex {
  SWITCH_ADD_FRAME,
  SWITCH_DELETE_FRAME
};

enum UiMode {
  UI_MODE_SPLASH,
  UI_MODE_NORMAL,
  UI_MODE_SAVE_CONFIRMATION,  
  UI_MODE_ERASE_CONFIRMATION,  
  UI_MODE_EDIT_RESPONSE,
  UI_MODE_EDIT_EASING,
  UI_MODE_FACTORY_TESTING
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Keyframer* keyframer, PolyLfo* poly_lfo);
  void Poll();
  void DoEvents();
  void FlushEvents();
  void TryCalibration();
  
  inline uint16_t frame() const {
    return adc_.value(kFrameAdcChannel);
  }

  inline uint16_t frame_modulation() const {
    return adc_.value(kFrameModulationAdcChannel);
  }
  
  inline UiMode mode() const {
    return mode_;
  }
  
  inline bool sequencer_mode() const {
    return sequencer_mode_;
  }

  inline bool poly_lfo_mode() const {
    return poly_lfo_mode_;
  }
  
 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void OnPotChanged(const stmlib::Event& e);
  
  // Force the gain value of the 4 channels to match that of the 4 pots.
  void SyncWithPots();
  
  void FindNearestKeyframe();

  stmlib::EventQueue<32> queue_;

  uint16_t adc_value_[kNumAdcChannels];
  uint16_t adc_filtered_value_[kNumAdcChannels];
  uint32_t press_time_[kNumSwitches];
  bool detect_very_long_press_[kNumSwitches];
  
  ChannelLeds channel_leds_;
  KeyframeLed keyframe_led_;
  RgbLed rgb_led_;
  Switches switches_;
  Adc adc_;
  FactoryTestingSwitch factory_testing_switch_;
  
  UiMode mode_;
  
  Keyframer* keyframer_;
  PolyLfo* poly_lfo_;
  
  int16_t active_keyframe_;
  int16_t active_channel_;
  bool active_keyframe_lock_;

  bool poly_lfo_mode_;
  bool sequencer_mode_;
  int8_t secret_handshake_counter_;
  
  uint16_t animation_counter_;
  uint16_t keyframe_led_pwm_counter_;
  
  bool test_led_;
  
  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace frames

#endif  // FRAMES_UI_H_
