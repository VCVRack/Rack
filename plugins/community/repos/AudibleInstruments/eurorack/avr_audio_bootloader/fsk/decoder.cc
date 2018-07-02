// Copyright 2013 Olivier Gillet.
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
// Simpler/flatter version of the STM FSK bootloader.

#include "avr_audio_bootloader/fsk/decoder.h"

namespace avr_audio_bootloader {

/* static */
bool Decoder::previous_sample_;

/* static */
uint16_t Decoder::duration_;  

/* static */
uint8_t Decoder::swallow_;

/* static */
DecoderState Decoder::state_;

/* static */
uint8_t Decoder::expected_symbols_;

/* static */
uint8_t Decoder::preamble_remaining_size_;

/* static */
uint16_t Decoder::sync_blank_size_;

/* static */
uint8_t Decoder::symbol_count_;

/* static */
uint8_t* Decoder::packet_;

/* static */
uint16_t Decoder::packet_size_;

/* static */
uint16_t Decoder::packet_count_;

}  // namespace avr_audio_bootloader
