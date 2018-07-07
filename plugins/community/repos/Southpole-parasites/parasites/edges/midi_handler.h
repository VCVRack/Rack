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
//
// -----------------------------------------------------------------------------

#ifndef EDGES_MIDI_HANDLER_H_
#define EDGES_MIDI_HANDLER_H_

#include "avrlibx/avrlibx.h"

#include "edges/hardware_config.h"
#include "edges/midi.h"
#include "edges/note_stack.h"
#include "edges/settings.h"
#include "edges/voice_allocator.h"

namespace edges {

enum MidiMode {
  MIDI_MODE_MULTITIMBRAL,
  MIDI_MODE_POLYPHONIC
};

class MidiHandler : public midi::MidiDevice {
 public:
  MidiHandler() { }
  
  static inline void Init() {
    for (uint8_t channel = 0; channel < kNumChannels; ++channel) {
      stack_[channel].Init();
    }
    allocator_.Init();
    allocator_.set_size(4);
    learning_ = false;
    memset(gate_, false, sizeof(gate_));
    memset(pitch_, -1, sizeof(pitch_));
  }
  
  static void ToggleMidiMode() {
    settings.ToggleMidiMode();
    Init();
  }

  static void DisableMidiCoupling() {
    Init();
  }
  
  static void ResetAllControllers(uint8_t channel) {
    channel = (channel - base_channel()) & 0xf;
    pitch_bend_[channel] = 0;
  }

  static inline void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    channel = (channel - base_channel()) & 0xf;
    if (velocity == 0) {
      NoteOff(channel, note, 0);
    }
    if (midi_mode() == MIDI_MODE_MULTITIMBRAL) {
      stack_[channel].NoteOn(note, velocity);
      pitch_[channel] = stack_[channel].most_recent_note().note << 7;
    } else {
      channel = allocator_.NoteOn(note);
      pitch_[channel] = note << 7;
    }
    gate_[channel] = true;
  }
  
  static inline void NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    channel = (channel - base_channel()) & 0xf;
    
    if (midi_mode() == MIDI_MODE_MULTITIMBRAL) {
      stack_[channel].NoteOff(note);
      if (stack_[channel].size()) {
        pitch_[channel] = stack_[channel].most_recent_note().note << 7;
      }
      gate_[channel] = stack_[channel].size() != 0;
    } else {
      channel = allocator_.NoteOff(note);
      if (channel != 0xff) {
        gate_[channel] = false;
      }
    }
  }
  
  static inline void PitchBend(uint8_t channel, uint16_t value) {
    channel = (channel - base_channel()) & 0xf;
    int16_t v = value;
    v -= 8192;
    v >>= 5;
    if (midi_mode() == MIDI_MODE_MULTITIMBRAL) {
      pitch_bend_[channel] = v;
    } else {
      for (uint8_t i = 0; i < kNumChannels; ++i) {
        pitch_bend_[i] = v;
      }
    }
  }
  
  static inline void RawMidiData(
      uint8_t status,
      uint8_t* data,
      uint8_t data_size,
      uint8_t accepted_channel) {
    if (learning_ && (status & 0xf0) == 0x90) {
      settings.set_midi_channel(status & 0xf);
      learning_ = false;
    }
  }

  static uint8_t CheckChannel(uint8_t channel) {
    if (midi_mode() == MIDI_MODE_MULTITIMBRAL) {
      return ((channel - base_channel()) & 0xf) < kNumChannels;
    } else {
      return channel == base_channel();
    }
  }
  
  static void Learn() {
    learning_ = true;
  }
  
  static inline bool learning() { return learning_; }
  
  static inline bool gate(uint8_t channel) { return gate_[channel]; }
  static int16_t shift_pitch(uint8_t channel, int16_t pitch) {
    if (pitch_[channel] == -1) {
      return pitch;
    } else {
      pitch -= (60 << 7);
      pitch += pitch_[channel];
      pitch += pitch_bend_[channel];
      if (pitch < 0) {
        pitch = 0;
      } else if (pitch > 16383) {
        pitch = 16383;
      }
      return pitch;
    }
    return pitch;
  }
  
  static inline uint8_t base_channel() {
    return settings.midi_channel();
  }

  static inline MidiMode midi_mode() {
    return static_cast<MidiMode>(settings.midi_mode());
  }
  
 private:
  static bool learning_;
  static bool gate_[kNumChannels];
  static int16_t pitch_[kNumChannels];
  static int16_t pitch_bend_[kNumChannels];
  
  static NoteStack<10> stack_[kNumChannels];
  static VoiceAllocator allocator_;

  DISALLOW_COPY_AND_ASSIGN(MidiHandler);
};

extern MidiHandler midi_handler;

}  // namespace edges

#endif  // EDGES_MIDI_HANDLER_H_
