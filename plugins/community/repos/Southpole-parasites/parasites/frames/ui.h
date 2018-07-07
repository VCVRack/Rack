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
class Euclidean;

const uint8_t kMaxRegisters = 16;

enum SwitchIndex {
  SWITCH_ADD_FRAME,
  SWITCH_DELETE_FRAME
};

enum UiMode {
  UI_MODE_SPLASH,
  UI_MODE_FEATURE_SWITCH,
  UI_MODE_NORMAL,
  UI_MODE_SAVE_CONFIRMATION,  
  UI_MODE_ERASE_CONFIRMATION,  
  UI_MODE_EDIT_RESPONSE,
  UI_MODE_EDIT_EASING,
  UI_MODE_FACTORY_TESTING,
  UI_MODE_EDIT_LENGTH,
  UI_MODE_EDIT_SHAPE,
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Keyframer* keyframer, PolyLfo* poly_lfo, Euclidean euclidean[kNumChannels]);
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

  enum FeatureMode {
    FEAT_MODE_KEYFRAMER,
    FEAT_MODE_KEYFRAME_LOOPER,
    FEAT_MODE_SEQ_MAIN,
    FEAT_MODE_SEQ_SHIFT_REGISTER,
    FEAT_MODE_SEQ_STEP_EDIT,
    FEAT_MODE_POLY_LFO,
    FEAT_MODE_EUCLIDEAN,
  };

  inline FeatureMode feature_mode() const {
    return feature_mode_;
  }

  uint8_t sequencer_step;
  uint8_t sequencer_random;
  uint8_t step_divider;
  uint8_t shift_divider;
  uint8_t step_random;
  uint8_t shift_random;
  uint8_t feedback_random;
  uint16_t shift_register[kMaxRegisters];
  uint8_t active_registers;

  KeyframeLed keyframe_led_;
  RgbLed rgb_led_;

 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void OnPotChanged(const stmlib::Event& e);
  void ParseShiftSequencer(uint16_t control_id, int32_t data);
  void ParsePolyLFO(uint16_t control_id, int32_t data);
  void ParseEuclidean(uint16_t control_id, int32_t data);

  // Force the gain value of the 4 channels to match that of the 4 pots.
  void SyncWithPots();
  void SyncWithPotsShiftSequencer();
  void SyncWithPotsPolyLFO();
  void SyncWithPotsEuclidean();

  void FindNearestKeyframe();

  stmlib::EventQueue<32> queue_;

  uint16_t adc_value_[kNumAdcChannels];
  uint16_t adc_filtered_value_[kNumAdcChannels];
  uint32_t press_time_[kNumSwitches];
  bool detect_very_long_press_[kNumSwitches];
  
  ChannelLeds channel_leds_;
  Switches switches_;
  Adc adc_;
  FactoryTestingSwitch factory_testing_switch_;
  
  UiMode mode_;
  
  Keyframer* keyframer_;
  PolyLfo* poly_lfo_;
  Euclidean* euclidean_;

  int16_t active_keyframe_;
  int16_t active_channel_;
  int16_t active_slot_;
  bool active_keyframe_lock_;

  FeatureMode feature_mode_;

  uint16_t animation_counter_;
  uint16_t keyframe_led_pwm_counter_;

  uint8_t ignore_releases_;

  bool test_led_;

  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace frames

#endif  // FRAMES_UI_H_
