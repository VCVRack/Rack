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
// Instance of the audio buffer class.

#ifndef EDGES_AUDIO_BUFFER_H_
#define EDGES_AUDIO_BUFFER_H_

#include "avrlibx/io/ring_buffer.h"

namespace edges {

struct AudioBufferSpecs {
  typedef uint16_t Value;
  enum {
    buffer_size = 128,
    data_size = 16,
  };
};

static const uint8_t kAudioBlockSize = 16;

extern avrlibx::RingBuffer<AudioBufferSpecs> audio_buffer;

}  // namespace edges

#endif  // EDGES_AUDIO_BUFFER_H_
