// Copyright 2012 Olivier Gillet.
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

#include <avr/eeprom.h>

#include "avrlib/adc.h"
#include "avrlib/boot.h"
#include "avrlib/op.h"
#include "avrlib/watchdog_timer.h"

#include "grids/clock.h"
#include "grids/hardware_config.h"
#include "grids/pattern_generator.h"

using namespace avrlib;
using namespace grids;

Leds leds;
Inputs inputs;
AdcInputScanner adc;
ShiftRegister shift_register;
MidiInput midi;

enum Parameter {
  PARAMETER_NONE,
  PARAMETER_WAITING,
  PARAMETER_CLOCK_RESOLUTION,
  PARAMETER_TAP_TEMPO,
  PARAMETER_SWING,
  PARAMETER_GATE_MODE,
  PARAMETER_OUTPUT_MODE,
  PARAMETER_CLOCK_OUTPUT
};

uint32_t tap_duration = 0;
uint8_t led_pattern ;
uint8_t led_off_timer;

int8_t swing_amount;

volatile Parameter parameter = PARAMETER_NONE;
volatile bool long_press_detected = false;
const uint8_t kUpdatePeriod = F_CPU / 32 / 8000;

inline void UpdateLeds() {
  uint8_t pattern;
  if (parameter == PARAMETER_NONE) {
    if (led_off_timer) {
      --led_off_timer;
      if (!led_off_timer) {
        led_pattern = 0;
      }
    }
    pattern = led_pattern;
    if (pattern_generator.tap_tempo()) {
      if (pattern_generator.on_beat()) {
        pattern |= LED_CLOCK;
      }
    } else {
      if (pattern_generator.on_first_beat()) {
        pattern |= LED_CLOCK;
      }
    }
  } else {
    pattern = LED_CLOCK;
    switch (parameter) {
      case PARAMETER_CLOCK_RESOLUTION:
        pattern |= LED_BD >> pattern_generator.clock_resolution();
        break;
        
      case PARAMETER_CLOCK_OUTPUT:
        if (pattern_generator.output_clock()) {
          pattern |= LED_ALL;
        }
        break;
      
      case PARAMETER_SWING:
        if (pattern_generator.swing()) {
          pattern |= LED_ALL;
        }
        break;

      case PARAMETER_OUTPUT_MODE:
        if (pattern_generator.output_mode() == OUTPUT_MODE_DRUMS) {
          pattern |= LED_ALL;
        }
        break;
      
      case PARAMETER_TAP_TEMPO:
        if (pattern_generator.tap_tempo()) {
          pattern |= LED_ALL;
        }
        break;
        
      case PARAMETER_GATE_MODE:
        if (pattern_generator.gate_mode()) {
          pattern |= LED_ALL;
        }
    }
  }
  leds.Write(pattern);
}

inline void UpdateShiftRegister() {
  static uint8_t previous_state = 0;
  if (pattern_generator.state() != previous_state) {
    previous_state = pattern_generator.state();
    shift_register.Write(previous_state);
    if (!previous_state) {
      // Switch off the LEDs, but not now.
      led_off_timer = 200;
    } else {
      // Switch on the LEDs with a new pattern.
      led_pattern = pattern_generator.led_pattern();
      led_off_timer = 0;
    }
  }
}

uint8_t ticks_granularity[] = { 6, 3, 1 };

inline void HandleClockResetInputs() {
  static uint8_t previous_inputs;
  
  uint8_t inputs_value = ~inputs.Read();
  uint8_t num_ticks = 0;
  uint8_t increment = ticks_granularity[pattern_generator.clock_resolution()];
  
  // CLOCK
  if (clock.bpm() < 40 && !clock.locked()) {
    if ((inputs_value & INPUT_CLOCK) && !(previous_inputs & INPUT_CLOCK)) {
      num_ticks = increment;
    }
    if (!(inputs_value & INPUT_CLOCK) && (previous_inputs & INPUT_CLOCK)) {
      pattern_generator.ClockFallingEdge();
    }
    if (midi.readable()) {
      uint8_t byte = midi.ImmediateRead();
      if (byte == 0xf8) {
        num_ticks = 1;
      } else if (byte == 0xfa) {
        pattern_generator.Reset();
      }
    }
  } else {
    clock.Tick();
    clock.Wrap(swing_amount);
    if (clock.raising_edge()) {
      num_ticks = increment;
    }
    if (clock.past_falling_edge()) {
      pattern_generator.ClockFallingEdge();
    }
  }

  // RESET
  if ((inputs_value & INPUT_RESET) && !(previous_inputs & INPUT_RESET)) {
    pattern_generator.Reset();

    // !! HACK AHEAD !!
    // 
    // Earlier versions of the firmware retriggered the outputs whenever a
    // RESET signal was received. This allowed for nice drill'n'bass effects,
    // but made synchronization with another sequencer a bit glitchy (risk of
    // double notes at the beginning of a pattern). It was later decided
    // to remove this behaviour and make the RESET transparent (just set the 
    // step index without producing any trigger) - similar to the MIDI START
    // message. However, the factory testing script relies on the old behaviour.
    // To solve this problem, we reproduce this behaviour the first 5 times the
    // module is powered. After the 5th power-on (or settings change) cycle,
    // this odd behaviour disappears.
    if (pattern_generator.factory_testing() ||
        clock.bpm() >= 40 ||
        clock.locked()) {
      pattern_generator.Retrigger();
      clock.Reset();
    }
  }
  previous_inputs = inputs_value;
  
  if (num_ticks) {
    swing_amount = pattern_generator.swing_amount();
    pattern_generator.TickClock(num_ticks);
  }
}

enum SwitchState {
  SWITCH_STATE_JUST_PRESSED = 0xfe,
  SWITCH_STATE_PRESSED = 0x00,
  SWITCH_STATE_JUST_RELEASED = 0x01,
  SWITCH_STATE_RELEASED = 0xff
};

inline void HandleTapButton() {
  static uint8_t switch_state = 0xff;
  static uint16_t switch_hold_time = 0;
  
  switch_state = switch_state << 1;
  if (inputs.Read() & INPUT_SW_RESET) {
    switch_state |= 1;
  }
  
  if (switch_state == SWITCH_STATE_JUST_PRESSED) {
    if (parameter == PARAMETER_NONE) {
      if (!pattern_generator.tap_tempo()) {
        pattern_generator.Reset();
        if (pattern_generator.factory_testing() ||
            clock.bpm() >= 40 ||
            clock.locked()) {
          clock.Reset();
        }
      } else {
        uint32_t new_bpm = (F_CPU * 60L) / (32L * kUpdatePeriod * tap_duration);
        if (new_bpm >= 30 && new_bpm <= 480) {
          clock.Update(new_bpm, pattern_generator.clock_resolution());
          clock.Reset();
          clock.Lock();
        } else {
          clock.Unlock();
        }
        tap_duration = 0;
      }
    }
    switch_hold_time = 0;
  } else if (switch_state == SWITCH_STATE_PRESSED) {
    ++switch_hold_time;
    if (switch_hold_time == 500) {
      long_press_detected = true;
    }
  }
}

ISR(TIMER2_COMPA_vect, ISR_NOBLOCK) {
  static uint8_t switch_debounce_prescaler;
  
  ++tap_duration;
  ++switch_debounce_prescaler;
  if (switch_debounce_prescaler >= 10) {
    // Debounce RESET/TAP switch and perform switch action.
    HandleTapButton();
    switch_debounce_prescaler = 0;
  }
  
  HandleClockResetInputs();
  adc.Scan();
  
  pattern_generator.IncrementPulseCounter();
  UpdateShiftRegister();
  UpdateLeds();
}

static int16_t pot_values[8];

void ScanPots() {
  if (long_press_detected) {
    if (parameter == PARAMETER_NONE) {
      // Freeze pot values
      for (uint8_t i = 0; i < 8; ++i) {
        pot_values[i] = adc.Read8(i);
      }
      parameter = PARAMETER_WAITING;
    } else {
      parameter = PARAMETER_NONE;
      pattern_generator.SaveSettings();
    }
    long_press_detected = false;
  }
  
  if (parameter == PARAMETER_NONE) {
    uint8_t bpm = adc.Read8(ADC_CHANNEL_TEMPO);
    bpm = U8U8MulShift8(bpm, 220) + 20;
    if (bpm != clock.bpm() && !clock.locked()) {
      clock.Update(bpm, pattern_generator.clock_resolution());
    }
    PatternGeneratorSettings* settings = pattern_generator.mutable_settings();
    settings->options.drums.x = ~adc.Read8(ADC_CHANNEL_X_CV);
    settings->options.drums.y = ~adc.Read8(ADC_CHANNEL_Y_CV);
    settings->options.drums.randomness = ~adc.Read8(ADC_CHANNEL_RANDOMNESS_CV);
    settings->density[0] = ~adc.Read8(ADC_CHANNEL_BD_DENSITY_CV);
    settings->density[1] = ~adc.Read8(ADC_CHANNEL_SD_DENSITY_CV);
    settings->density[2] = ~adc.Read8(ADC_CHANNEL_HH_DENSITY_CV);
  } else {
    for (uint8_t i = 0; i < 8; ++i) {
      int16_t value = adc.Read8(i);
      int16_t delta = value - pot_values[i];
      if (delta < 0) {
        delta = -delta;
      }
      if (delta > 32) {
        pot_values[i] = value;
        switch (i) {
          case ADC_CHANNEL_BD_DENSITY_CV:
            parameter = PARAMETER_CLOCK_RESOLUTION;
            pattern_generator.set_clock_resolution((255 - value) >> 6);
            clock.Update(clock.bpm(), pattern_generator.clock_resolution());
            pattern_generator.Reset();
            break;
            
          case ADC_CHANNEL_SD_DENSITY_CV:
            parameter = PARAMETER_TAP_TEMPO;
            pattern_generator.set_tap_tempo(!(value & 0x80));
            if (!pattern_generator.tap_tempo()) {
              clock.Unlock();
            }
            break;

          case ADC_CHANNEL_HH_DENSITY_CV:
            parameter = PARAMETER_SWING;
            pattern_generator.set_swing(!(value & 0x80));
            break;

          case ADC_CHANNEL_X_CV:
            parameter = PARAMETER_OUTPUT_MODE;
            pattern_generator.set_output_mode(!(value & 0x80) ? 1 : 0);
            break;

          case ADC_CHANNEL_Y_CV:
            parameter = PARAMETER_GATE_MODE;
            pattern_generator.set_gate_mode(!(value & 0x80));
            break;

          case ADC_CHANNEL_RANDOMNESS_CV:
            parameter = PARAMETER_CLOCK_OUTPUT;
            pattern_generator.set_output_clock(!(value & 0x80));
            break;
        }
      }
    }
  }
}

void Init() {
  sei();
  UCSR0B = 0;
  
  leds.set_mode(DIGITAL_OUTPUT);
  inputs.set_mode(DIGITAL_INPUT);
  inputs.EnablePullUpResistors();
  
  clock.Init();
  adc.Init();
  adc.set_num_inputs(ADC_CHANNEL_LAST);
  Adc::set_reference(ADC_DEFAULT);
  Adc::set_alignment(ADC_LEFT_ALIGNED);
  pattern_generator.Init();
  shift_register.Init();
  midi.Init();
  
  TCCR2A = _BV(WGM21);
  TCCR2B = 3;
  OCR2A = kUpdatePeriod - 1;
  TIMSK2 |= _BV(1);
}

int main(void) {
  ResetWatchdog();
  Init();
  clock.Update(120, pattern_generator.clock_resolution());
  while (1) {
    // Use any spare cycles to read the CVs and update the potentiometers
    ScanPots();
  }
}
