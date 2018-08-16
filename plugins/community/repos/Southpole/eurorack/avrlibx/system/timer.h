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
// Timer/counter

#ifndef AVRLIBX_SYSTEM_TIMER_H_
#define AVRLIBX_SYSTEM_TIMER_H_

#include <avr/delay.h>

#include "avrlibx/avrlibx.h"
#include "avrlibx/io/gpio.h"
#include "avrlibx/system/event_system.h"

namespace avrlibx {

enum TimerMode {
  TIMER_MODE_NORMAL = 0,
  TIMER_MODE_FREQUENCY_GENERATOR = 1,
  TIMER_MODE_SINGLE_PWM = 3,
  TIMER_MODE_DUAL_PWM_T = 5,
  TIMER_MODE_DUAL_PWM_TB = 6,
  TIMER_MODE_DUAL_PWM_B = 7
};

enum TimerPrescaler {
  TIMER_PRESCALER_OFF = 0,
  TIMER_PRESCALER_CLK = 1,
  TIMER_PRESCALER_CLK_2 = 2,
  TIMER_PRESCALER_CLK_4 = 3,
  TIMER_PRESCALER_CLK_8 = 4,
  TIMER_PRESCALER_CLK_64 = 5,
  TIMER_PRESCALER_CLK_256 = 6,
  TIMER_PRESCALER_CLK_1024 = 7,
};

enum TimerChannel {
  TIMER_CHANNEL_A,
  TIMER_CHANNEL_B,
  TIMER_CHANNEL_C,
  TIMER_CHANNEL_D
};

enum TimerEventAction {
  TIMER_EVENT_ACTION_NONE = TC_EVACT_OFF_gc,
  TIMER_EVENT_ACTION_CAPTURE = TC_EVACT_CAPT_gc,
  TIMER_EVENT_ACTION_UPDOWN = TC_EVACT_UPDOWN_gc,
  TIMER_EVENT_ACTION_QDEC = TC_EVACT_QDEC_gc,
  TIMER_EVENT_ACTION_RESTART = TC_EVACT_RESTART_gc,
  TIMER_EVENT_ACTION_FRQ = TC_EVACT_FRW_gc,
  TIMER_EVENT_ACTION_PW = TC_EVACT_PW_gc
};

template<typename Port, uint8_t index> struct TCWrapper { };

#define WRAP_TIMER(port, index) \
template<> \
struct TCWrapper<Port ## port, index> { \
  static inline TC0_t& tc() { \
    return (TC0_t&)(TC ## port ## index); \
  } \
  static volatile inline uint16_t count() { \
    return TC ## port ## index ## _CNT; \
  } \
  static inline void set_count(uint16_t value) { \
    TC ## port ## index ## _CNT = value; \
  } \
  static inline uint16_t period() { \
    return TC ## port ## index ## _PERBUF; \
  } \
  static inline void set_period(uint16_t value) { \
    TC ## port ## index ## _PERBUF = value; \
  } \
  static inline uint8_t dma_tx_trigger() { \
    return DMA_CH_TRIGSRC_TC ## port ## index ## _OVF_gc; \
  } \
  static inline uint8_t overflow_event() { \
    return EVSYS_CHMUX_TC ## port ## index ## _OVF_gc; \
  } \
  template<uint8_t channel> \
  static inline void set_channel(uint16_t value) { \
    if (channel == TIMER_CHANNEL_A) { \
      TC ## port ## index ## _CCABUF = value; \
    } else if (channel == TIMER_CHANNEL_B) { \
      TC ## port ## index ## _CCBBUF = value; \
    } else if (channel == TIMER_CHANNEL_C && !index) { \
      TC ## port ## 0_CCCBUF = value; \
    } else if (channel == TIMER_CHANNEL_D && !index) { \
      TC ## port ## 0_CCDBUF = value; \
    } \
  } \
  template<uint8_t channel> \
  static inline uint16_t get_channel() { \
    if (channel == TIMER_CHANNEL_A) { \
      return TC ## port ## index ## _CCA; \
    } else if (channel == TIMER_CHANNEL_B) { \
      return TC ## port ## index ## _CCB; \
    } else if (channel == TIMER_CHANNEL_C && !index) { \
      return TC ## port ## 0_CCC; \
    } else if (channel == TIMER_CHANNEL_D && !index) { \
      return TC ## port ## 0_CCD; \
    } \
  } \
};

WRAP_TIMER(C, 0)
WRAP_TIMER(C, 1)
WRAP_TIMER(D, 0)
WRAP_TIMER(D, 1)
WRAP_TIMER(E, 0)
#ifdef TCE1
  WRAP_TIMER(E, 1)
#endif
#ifdef TCF0
  WRAP_TIMER(F, 0)
#endif

template<typename Port, uint8_t index>
class Timer {
 public:
  typedef TCWrapper<Port, index> TC;
  static inline void set_prescaler(TimerPrescaler prescaler) {
    TC::tc().CTRLA = prescaler;
  }

  static inline void set_mode(TimerMode mode) {
    // Preserve Compare/capture enable and set mode.
    TC::tc().CTRLB = (TC::tc().CTRLB & 0xf0) | mode;
  }
   
  static volatile inline uint16_t count() {
    return TC::count();
  }
  static inline void set_count(uint16_t value) {
    TC::set_count(value);
  }
  
  static inline uint16_t period() {
    return TC::period();
  }
  static inline void set_period(uint16_t value) {
    TC::set_period(value);
  }
  
  static inline void Restart() {
    TC::tc().CTRLFSET = 8;
  }
  
  static inline void Bind(uint8_t channel, TimerEventAction event_action) {
    TC::tc().CTRLD = event_action | 0x08 | channel;
  }
  
  static inline void EnableCC(uint8_t channel) {
    TC::tc().CTRLB |= (16 << channel);
  }
  static inline void StopCC(uint8_t channel) {
    TC::tc().CTRLB &= ~(16 << channel);
  }
  
  static inline void set_pwm_resolution(uint8_t resolution) {
    TC::set_period((1 << static_cast<uint16_t>(resolution) )- 1);
  }
  
  static inline void EnableOverflowInterrupt(uint8_t int_level) {
    TC::tc().INTCTRLA = (TC::tc().INTCTRLA & 0xfc) | int_level;
  }
  
  static inline void DisableOverflowInterrupt() {
    TC::tc().INTCTRLA = TC::tc().INTCTRLA & 0xfc;
  }
  
  static inline void EnableChannelInterrupt(
      uint8_t channel, uint8_t int_level) {
    uint8_t shift = channel << 2;
    uint8_t mask = (0x3) << shift;
    TC::tc().INTCTRLB = (TC::tc().INTCTRLB & ~mask) | (int_level << shift);
  }
  
  static inline void DisableChannelInterrupt(uint8_t channel) {
    uint8_t shift = channel << 2;
    uint8_t mask = (0x3) << shift;
    TC::tc().INTCTRLB = TC::tc().INTCTRLB & ~mask;
  }
  
  template<uint8_t channel>
  static inline void set_channel(uint16_t value) {
    TC::template set_channel<channel>(value);
  }
  
  template<uint8_t channel>
  static inline uint16_t get_channel() {
    return TC::template get_channel<channel>();
  }
  
  template<uint8_t channel>
  static inline void EnableChannelInterrupt(uint8_t int_level) {
    uint8_t shift = channel << 2;
    uint8_t mask = (0x3) << shift;
    TC::tc().INTCTRLB = (TC::tc().INTCTRLB & ~mask) | (int_level << shift);
  }
  
  template<uint8_t channel>
  static inline void DisableChannelInterrupt() {
    uint8_t shift = channel << 2;
    uint8_t mask = (0x3) << shift;
    TC::tc().INTCTRLB = TC::tc().INTCTRLB & ~mask;
  }
  
  static inline uint8_t dma_tx_trigger() {
    //
    // Horrible hack ahead!
    //
    // It looks like a timer overflow cannot be used as a DMA trigger. The
    // workaround for this is to use an Event as a proxy: set the event trigger
    // to be the timer overflow ; set the DMA trigger to be the event.
    // Here we use system event 0 for this purpose.
    EVSYS_CH0MUX = TC::overflow_event();
    return DMA_CH_TRIGSRC_EVSYS_CH0_gc;
  }
};

// To be tested (XMega-AU only)
template<typename Port, uint8_t index>
class DualTimer {
 public:
  typedef TCWrapper<Port, index> TC;
  static inline void set_prescaler(TimerPrescaler prescaler) {
    TC::tc().CTRLE = 0x2;
    TC::tc().CTRLA = prescaler;
  }

  static inline void EnabledInterrupts(uint8_t level_1, uint8_t level_2) {
    TC::tc().INTCTRLA = (level_1 << 2) | level_2;
  }
  
  static inline void set_periods(uint8_t period_1, uint8_t period_2) {
    TC::tc().HPER = period_1;
    TC::tc().LPER = period_2;
  }
};

template<typename Port, uint8_t pin>
struct PWMPinToTimer { };

#define BIND_TIMER_TO_PWM_PIN(port, index, Channel, pin) \
template<> struct PWMPinToTimer<port, pin> { \
  typedef Timer<port, index> T; \
  enum { channel = Channel }; \
}; \

BIND_TIMER_TO_PWM_PIN(PortC, 0, TIMER_CHANNEL_A, 0);
BIND_TIMER_TO_PWM_PIN(PortC, 0, TIMER_CHANNEL_B, 1);
BIND_TIMER_TO_PWM_PIN(PortC, 0, TIMER_CHANNEL_C, 2);
BIND_TIMER_TO_PWM_PIN(PortC, 0, TIMER_CHANNEL_D, 3);
BIND_TIMER_TO_PWM_PIN(PortC, 1, TIMER_CHANNEL_A, 4);
BIND_TIMER_TO_PWM_PIN(PortC, 1, TIMER_CHANNEL_B, 5);

BIND_TIMER_TO_PWM_PIN(PortD, 0, TIMER_CHANNEL_A, 0);
BIND_TIMER_TO_PWM_PIN(PortD, 0, TIMER_CHANNEL_B, 1);
BIND_TIMER_TO_PWM_PIN(PortD, 0, TIMER_CHANNEL_C, 2);
BIND_TIMER_TO_PWM_PIN(PortD, 0, TIMER_CHANNEL_D, 3);
BIND_TIMER_TO_PWM_PIN(PortD, 1, TIMER_CHANNEL_A, 4);
BIND_TIMER_TO_PWM_PIN(PortD, 1, TIMER_CHANNEL_B, 5);

BIND_TIMER_TO_PWM_PIN(PortE, 0, TIMER_CHANNEL_A, 0);
BIND_TIMER_TO_PWM_PIN(PortE, 0, TIMER_CHANNEL_B, 1);
BIND_TIMER_TO_PWM_PIN(PortE, 0, TIMER_CHANNEL_C, 2);
BIND_TIMER_TO_PWM_PIN(PortE, 0, TIMER_CHANNEL_D, 3);

#ifdef TCE1
BIND_TIMER_TO_PWM_PIN(PortE, 1, TIMER_CHANNEL_A, 4);
BIND_TIMER_TO_PWM_PIN(PortE, 1, TIMER_CHANNEL_B, 5);
#endif

#ifdef TCF0
BIND_TIMER_TO_PWM_PIN(PortF, 0, TIMER_CHANNEL_A, 0);
BIND_TIMER_TO_PWM_PIN(PortF, 0, TIMER_CHANNEL_B, 1);
BIND_TIMER_TO_PWM_PIN(PortF, 0, TIMER_CHANNEL_C, 2);
BIND_TIMER_TO_PWM_PIN(PortF, 0, TIMER_CHANNEL_D, 3);
#endif

template<typename Port, uint8_t pin>
class PWM {
 public:
  static inline void set_value(uint16_t value) {
    PWMPinToTimer<Port, pin>::T::template \
        set_channel<PWMPinToTimer<Port, pin>::channel>(value);
  }
  static inline uint16_t get_value() {
    return PWMPinToTimer<Port, pin>::T::template \
    get_channel<PWMPinToTimer<Port, pin>::channel>();
  }
  static inline void Init(uint8_t resolution) {
    typename PWMPinToTimer<Port, pin>::T timer;
    timer.set_prescaler(TIMER_PRESCALER_CLK);
    timer.set_mode(TIMER_MODE_SINGLE_PWM);
    timer.set_pwm_resolution(resolution);
    Start();
  }
  static inline void EnableInterrupt(uint8_t level) {
    PWMPinToTimer<Port, pin>::T::template \
        EnableChannelInterrupt<PWMPinToTimer<Port, pin>::channel>(level);
  }
  static inline void DisableInterrupt() {
    PWMPinToTimer<Port, pin>::T::template \
        DisableChannelInterrupt<PWMPinToTimer<Port, pin>::channel>();
  }
  static inline void Start() {
    Gpio<Port, pin>::set_direction(OUTPUT);
    PWMPinToTimer<Port, pin>::T::EnableCC(PWMPinToTimer<Port, pin>::channel);
  }
  static inline void Stop() {
    PWMPinToTimer<Port, pin>::T::StopCC(PWMPinToTimer<Port, pin>::channel);
  }
  static inline void Write(uint16_t value) { set_value(value); }
};

template<typename Port, uint8_t pin>
class InputCapture {
 public:
  template<uint8_t event_channel>
  static inline void Init(bool frequency_measurement_mode) {
    Gpio<Port, pin> gpio;
    // Set the GPIO to input with rising edge detection, and disable pull-up.
    gpio.set_direction(INPUT);
    gpio.set_sense(SENSE_MODE_RISING);
    gpio.Low();
    // The selected event source is the GPIO state change.
    EventSystemChannel<event_channel>::set_source(gpio.event());
    // Bind this event to the channel capture.
    PWMPinToTimer<Port, pin>::T::Bind(
        event_channel,
        frequency_measurement_mode
            ? TIMER_EVENT_ACTION_FRQ
            : TIMER_EVENT_ACTION_CAPTURE);
    Start();
  }
  static inline void EnableInterrupt(uint8_t level) {
    PWMPinToTimer<Port, pin>::T::template \
        EnableChannelInterrupt<PWMPinToTimer<Port, pin>::channel>(level);
  }
  static inline void DisableInterrupt() {
    PWMPinToTimer<Port, pin>::T::template \
        DisableChannelInterrupt<PWMPinToTimer<Port, pin>::channel>();
  }
  static inline uint16_t get_value() {
    return PWMPinToTimer<Port, pin>::T::template \
        get_channel<PWMPinToTimer<Port, pin>::channel>();
  }
  static inline void Start() {
    PWMPinToTimer<Port, pin>::T::EnableCC(PWMPinToTimer<Port, pin>::channel);
  }
  static inline void Stop() {
    PWMPinToTimer<Port, pin>::T::StopCC(PWMPinToTimer<Port, pin>::channel);
  }
};


}  // namespace avrlibx

#endif  // AVRLIBX_SYSTEM_TIMER_H_
