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

#ifndef GRIDS_HARDWARE_CONFIG_H_
#define GRIDS_HARDWARE_CONFIG_H_

#include "avrlib/base.h"
#include "avrlib/gpio.h"
#include "avrlib/parallel_io.h"
#include "avrlib/serial.h"
#include "avrlib/spi.h"

namespace grids {

enum AdcChannel {
  ADC_CHANNEL_X_CV,
  ADC_CHANNEL_Y_CV,
  ADC_CHANNEL_RANDOMNESS_CV,
  ADC_CHANNEL_BD_DENSITY_CV,
  ADC_CHANNEL_SD_DENSITY_CV,
  ADC_CHANNEL_HH_DENSITY_CV,
  ADC_CHANNEL_TEMPO,
  ADC_CHANNEL_LAST
};

enum LedBits {
  LED_CLOCK = 1,
  LED_BD = 8,
  LED_SD = 4,
  LED_HH = 2,
  LED_ALL = LED_CLOCK | LED_BD | LED_SD | LED_HH
};

enum InputBits {
  INPUT_CLOCK = 2,
  INPUT_RESET = 4,
  INPUT_SW_RESET = 8
};

using avrlib::Gpio;
using avrlib::ParallelPort;
using avrlib::PortB;
using avrlib::PortD;
using avrlib::Serial;
using avrlib::SerialPort0;
using avrlib::SpiMaster;

typedef ParallelPort<PortD, avrlib::PARALLEL_NIBBLE_HIGH> Leds;
typedef ParallelPort<PortD, avrlib::PARALLEL_NIBBLE_LOW> Inputs;
typedef SpiMaster<Gpio<PortB, 2>, avrlib::MSB_FIRST, 2> ShiftRegister;
typedef Serial<SerialPort0, 31250, avrlib::POLLED, avrlib::DISABLED> MidiInput;
}  // namespace grids

#endif  // GRIDS_HARDWARE_CONFIG_H_
