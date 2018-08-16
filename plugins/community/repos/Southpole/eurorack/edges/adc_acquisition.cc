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

#include "edges/adc_acquisition.h"

namespace edges {

/* static */
ADCInterface ADCAcquisition::adc_;

/* static */
Word ADCAcquisition::rx_;

/* static */
uint8_t ADCAcquisition::active_channel_;

/* static */
uint8_t ADCAcquisition::acquisition_stage_;

/* static */
uint16_t ADCAcquisition::channels_[kNumAdcChannels];

/* extern */
ADCAcquisition adc_acquisition;

}  // namespace edges
