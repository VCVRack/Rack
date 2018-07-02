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
//
// -----------------------------------------------------------------------------
//
// Staged SPI communication with the ADC. Prevents the 128 cycles lock.

#ifndef EDGES_ADC_ACQUISITION_H_
#define EDGES_ADC_ACQUISITION_H_

#include "avrlibx/avrlibx.h"

#include "edges/hardware_config.h"

namespace edges {
  
class ADCAcquisition {
 public:
  ADCAcquisition() { }
  ~ADCAcquisition() { }
  
  static void Init() {
    adc_.Init();
    acquisition_stage_ = 0;
    active_channel_ = 0;
  }
  static inline uint8_t Cycle() {
    int8_t result = -1;
    switch (acquisition_stage_) {
      case 0:
        rx_.bytes[0] = adc_.ImmediateRead();
        channels_[active_channel_] = rx_.value;
        adc_.End();
        adc_.Begin();
        result = active_channel_;
        active_channel_ = (active_channel_ + 1) & (kNumAdcChannels - 1);
        adc_.Overwrite(0x04 | 0x02 | (active_channel_ >> 2));
        acquisition_stage_ = 1;
        break;
      case 1:
        adc_.Overwrite(active_channel_ << 6);
        acquisition_stage_ = 2;
        break;
      case 2:
        rx_.bytes[1] = adc_.ImmediateRead() & 0xf;
        adc_.Overwrite(0x00);  // Dummy write to get next byte.
        acquisition_stage_ = 0;
        break;
    }
    return result;
  }
  
  static uint16_t channel(uint8_t index) {
    return channels_[index];
  }
  
 private:
  static ADCInterface adc_;
  
  static Word rx_;
  static uint8_t active_channel_;
  static uint8_t acquisition_stage_;
  static uint16_t channels_[kNumAdcChannels];
};

extern ADCAcquisition adc_acquisition;

}  // namespace edges

#endif  // EDGES_ADC_ACQUISITION_H_
