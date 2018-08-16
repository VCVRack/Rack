// Copyright 2009 Olivier Gillet.
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
// Instance of the midi out filter class.

#include "edges/midi_handler.h"

namespace edges {

/* static */
bool MidiHandler::gate_[kNumChannels];

/* static */
int16_t MidiHandler::pitch_[kNumChannels];

/* static */
int16_t MidiHandler::pitch_bend_[kNumChannels] = { 0, 0, 0, 0 };

/* static */
NoteStack<10> MidiHandler::stack_[kNumChannels];

/* static */
VoiceAllocator MidiHandler::allocator_;

/* static */
bool MidiHandler::learning_;

/* extern */
MidiHandler midi_handler;

}  // namespace anu