// Copyright 2017 Olivier Gillet.
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
// User interface

#include "stages/ui.h"

#include <algorithm>

#include "stmlib/system/system_clock.h"

using namespace std;
using namespace stmlib;

const int32_t kLongPressDuration = 1000;

namespace stages {

/* static */
const LedColor Ui::palette_[4] = {
  LED_COLOR_GREEN,
  LED_COLOR_YELLOW,
  LED_COLOR_RED,
  LED_COLOR_OFF
};

void Ui::Init(Settings* settings, ChainState* chain_state) {
  leds_.Init();
  switches_.Init();
  
  system_clock.Init();
  fill(&press_time_[0], &press_time_[kNumSwitches], 0);
  
  settings_ = settings;
  mode_ = UI_MODE_NORMAL;
  chain_state_ = chain_state;
  
  if (switches_.pressed_immediate(0)) {
    State* state = settings_->mutable_state();
    if (state->color_blind == 1) {
      state->color_blind = 0; 
    } else {
      state->color_blind = 1; 
    }
    settings_->SaveState();
  }
  
  fill(&slider_led_counter_[0], &slider_led_counter_[kNumLEDs], 0);
}

void Ui::Poll() {
  system_clock.Tick();
  UpdateLEDs();
  
  switches_.Debounce();
  
  if (chain_state_->ouroboros()) {
    State* s = settings_->mutable_state();
    for (int i = 0; i < kNumSwitches; ++i) {
      if (switches_.pressed(i)) {
        if (press_time_[i] != -1) {
          ++press_time_[i];
        }
        if (press_time_[i] > kLongPressDuration) {
          uint8_t loop_bit = s->segment_configuration[i] & 0x4;
          uint8_t type_bits = s->segment_configuration[i] & 0x03;
          s->segment_configuration[i] = type_bits | (4 - loop_bit);
          settings_->SaveState();
          press_time_[i] = -1;
        }
      } else {
        if (press_time_[i] > 0) {
          uint8_t loop_bit = s->segment_configuration[i] & 0x4;
          uint8_t type_bits = s->segment_configuration[i] & 0x03;
          s->segment_configuration[i] = ((type_bits + 1) % 3) | loop_bit;
          settings_->SaveState();
        }
        press_time_[i] = 0;
      }
    }
  } else {
    ChainState::ChannelBitmask pressed = 0;
    for (int i = 0; i < kNumSwitches; ++i) {
      if (switches_.pressed(i)) {
        pressed |= 1 << i;
      }
    }
    chain_state_->set_local_switch_pressed(pressed);
  }
}

inline uint8_t Ui::FadePattern(uint8_t shift, uint8_t phase) const {
  uint8_t x = system_clock.milliseconds() >> shift;
  x += phase;
  x &= 0x1f;
  return x <= 0x10 ? x : 0x1f - x;
}

void Ui::UpdateLEDs() {
  leds_.Clear();

  if (mode_ == UI_MODE_FACTORY_TEST) {
    size_t counter = (system_clock.milliseconds() >> 8) % 3;
    for (size_t i = 0; i < kNumChannels; ++i) {
      if (slider_led_counter_[i] == 0) {
        leds_.set(LED_GROUP_UI + i, palette_[counter]);
        leds_.set(LED_GROUP_SLIDER + i,
                  counter == 0 ? LED_COLOR_GREEN : LED_COLOR_OFF);
      } else if (slider_led_counter_[i] == 1) {
        leds_.set(LED_GROUP_UI + i, LED_COLOR_GREEN);
        leds_.set(LED_GROUP_SLIDER + i, LED_COLOR_OFF);
      } else {
        leds_.set(LED_GROUP_UI + i, LED_COLOR_GREEN);
        leds_.set(LED_GROUP_SLIDER + i, LED_COLOR_GREEN);
      }
    }
  } else if (chain_state_->discovering_neighbors()) {
    size_t counter = system_clock.milliseconds() >> 5;
    size_t n = chain_state_->size() * kNumChannels;
    counter = counter % (2 * n - 2);
    if (counter >= n) {
      counter = 2 * n - 2 - counter;
    }
    if (counter >= chain_state_->index() * kNumChannels) {
      counter -= chain_state_->index() * kNumChannels;
      if (counter < kNumChannels) {
        leds_.set(LED_GROUP_UI + counter, LED_COLOR_YELLOW);
        leds_.set(LED_GROUP_SLIDER + counter, LED_COLOR_GREEN);
      }
    }
  } else {
    uint8_t pwm = system_clock.milliseconds() & 0xf;
    uint8_t fade_patterns[4] = {
      0xf,  // NONE
      FadePattern(4, 0),  // START
      FadePattern(4, 0x0f),  // END
      FadePattern(4, 0x08),  // SELF
    };
    for (size_t i = 0; i < kNumChannels; ++i) {
      uint8_t configuration = settings_->state().segment_configuration[i];
      uint8_t type = configuration & 0x3;
      int brightness = fade_patterns[chain_state_->ouroboros()
          ? (configuration & 0x4 ? 3 : 0)
          : chain_state_->loop_status(i)];
      LedColor color = palette_[type];
      if (settings_->state().color_blind == 1) {
        if (type == 0) {
          color = LED_COLOR_GREEN;
          uint8_t modulation = FadePattern(6, 13 - (2 * i)) >> 1;
          brightness = brightness * (7 + modulation) >> 4;
        } else if (type == 1) {
          color = LED_COLOR_YELLOW;
          brightness = brightness >= 0x8 ? 0xf : 0;
        } else if (type == 2) {
          color = LED_COLOR_RED;
          brightness = brightness >= 0xc ? 0x1 : 0;
        }
      }
      leds_.set(
          LED_GROUP_UI + i,
          (brightness >= pwm && brightness != 0) ? color : LED_COLOR_OFF);
      leds_.set(
          LED_GROUP_SLIDER + i,
          slider_led_counter_[i] ? LED_COLOR_GREEN : LED_COLOR_OFF);
      if (slider_led_counter_[i]) {
        --slider_led_counter_[i];
      }
    }
  }
  leds_.Write();
}

}  // namespace stages
