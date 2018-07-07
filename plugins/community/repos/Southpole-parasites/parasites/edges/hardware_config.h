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

#ifndef EDGES_HARDWARE_CONFIG_H_
#define EDGES_HARDWARE_CONFIG_H_

#include "avrlibx/avrlibx.h"
#include "avrlibx/devices/switch.h"
#include "avrlibx/io/gpio.h"
#include "avrlibx/io/parallel_io.h"
#include "avrlibx/io/ring_buffer.h"
#include "avrlibx/io/usart.h"
#include "avrlibx/io/usart_spi.h"
#include "avrlibx/system/timer.h"

namespace edges {

using avrlibx::DualTimer;
using avrlibx::Gpio;
using avrlibx::ParallelPort;
using avrlibx::PortA;
using avrlibx::PortB;
using avrlibx::PortC;
using avrlibx::PortD;
using avrlibx::PortE;
using avrlibx::PWM;
using avrlibx::RingBuffer;
using avrlibx::SPIMaster;
using avrlibx::Timer;
using avrlibx::UsartSPIMaster;

// MIDI
typedef avrlibx::Usart<
    PortC,
    0,
    31250,
    avrlibx::RX_POLLED,
    avrlibx::TX_DISABLED> MidiIO;

struct MidiBufferSpecs {
  typedef uint8_t Value;
  enum {
    buffer_size = 128,
    data_size = 8,
  };
};

typedef RingBuffer<MidiBufferSpecs> MidiBuffer;

// IO
typedef ParallelPort<PortA, 0, 3> GateInputs;
typedef ParallelPort<PortA, 4, 7> Leds;
typedef ParallelPort<PortB, 0, 3> Switches;
typedef Gpio<PortE, 0> SyncSwitch;

// Audio DAC
typedef Gpio<PortC, 6> DACSS;
typedef UsartSPIMaster<
    PortC, 1,
    DACSS, avrlibx::MSB_FIRST, 2> AudioDACInterface;

// ADC
typedef Gpio<PortD, 3> ADCSS;
typedef SPIMaster<
    PortD, ADCSS,
    avrlibx::MSB_FIRST,
    avrlibx::SPI_PRESCALER_CLK_16> ADCInterface;

// Timers
typedef Timer<PortC, 0> Channel3Timer;
typedef Timer<PortC, 1> Channel2Timer;
typedef Timer<PortD, 0> Channel1Timer;
typedef Timer<PortD, 1> Channel0Timer;
typedef PWM<PortC, 0> Channel3;
typedef PWM<PortC, 4> Channel2;
typedef PWM<PortD, 0> Channel1;
typedef PWM<PortD, 4> Channel0;

typedef Timer<PortE, 0> AudioRateTimer;

const uint8_t kNumChannels = 4;

#ifdef OCTAL_ADC
  const uint8_t kNumAdcChannels = 8;
#else
  const uint8_t kNumAdcChannels = 4;
#endif
}  // namespace edges

#endif  // EDGES_HARDWARE_CONFIG_H_
