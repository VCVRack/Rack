// Copyright 2013 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Class monitoring a sequence of notes on the MIDI input and automatically
// chosing the best layout

#include "yarns/layout_configurator.h"
#include "yarns/multi.h"

#include <algorithm>

namespace yarns {

using namespace std;

void LayoutConfigurator::StopLearning(Multi* multi) {
  learning_ = false;
  
  bool used_channels[16];
  uint8_t num_used_channels = 0;
  uint8_t num_extra_drum_channels = 0;

  fill(&used_channels[0], &used_channels[16], 0);

  for (uint8_t i = 0; i < num_notes_; ++i) {
    uint8_t channel = recorded_notes_[i].channel;
    if (!used_channels[channel]) {
      ++num_used_channels;
    } else {
      if (channel == 9) {
        ++num_extra_drum_channels;
      }
    }
    used_channels[channel] = true;
  }

  if (!num_used_channels) {
    return;
  }
  
  // Count the number of splits
  uint8_t num_splits = 0;
  for (uint8_t i = 0; i < num_notes_; ++i) {
    for (uint8_t j = i + 1; j < num_notes_; ++j) {
      if (recorded_notes_[i].channel != 9 &&
          recorded_notes_[i].channel == recorded_notes_[j].channel && (
          (recorded_notes_[i].note == recorded_notes_[j].note + 1) ||
          (recorded_notes_[i].note + 1 == recorded_notes_[j].note))) {
        Split s;
        s.channel = recorded_notes_[i].channel;
        s.split_point = min(recorded_notes_[i].note, recorded_notes_[j].note);
        splits_[num_splits++] = s;
      }
    }
  }

  sort(&splits_[0], &splits_[num_splits], SplitLess());
  
  uint8_t num_parts = num_used_channels + num_splits;

  if (num_parts == 1) {
    uint8_t channel = recorded_notes_[0].channel;
    if (channel == 9) {
      multi->Set(MULTI_LAYOUT, LAYOUT_QUAD_TRIGGERS);
      for (uint8_t i = 0; i < num_notes_ && i < kNumParts; ++i) {
        uint8_t drum_note = recorded_notes_[i].note;
        Part* instrument = multi->mutable_part(i);
        instrument->Set(PART_MIDI_MIN_NOTE, drum_note);
        instrument->Set(PART_MIDI_MAX_NOTE, drum_note);
        instrument->Set(PART_MIDI_CHANNEL, channel);
      }
    } else {
      if (num_notes_ > 2) {
        multi->Set(MULTI_LAYOUT, LAYOUT_QUAD_POLY);
      } else if (num_notes_ > 1) {
        multi->Set(MULTI_LAYOUT, LAYOUT_DUAL_POLY);
      } else {
        multi->Set(MULTI_LAYOUT, LAYOUT_MONO);
      }
      Part* p = multi->mutable_part(0);
      p->Set(PART_MIDI_CHANNEL, channel);
      p->Set(PART_MIDI_MIN_NOTE, 0);
      p->Set(PART_MIDI_MAX_NOTE, 127);
    }
  } else {
    num_parts += num_extra_drum_channels;
    if (num_parts == 2) {
      multi->Set(MULTI_LAYOUT, LAYOUT_DUAL_MONO);
    } else {
      multi->Set(MULTI_LAYOUT, LAYOUT_QUAD_MONO);
    }
    uint8_t part_index = 0;
    for (uint8_t i = 0; i < num_notes_; ++i) {
      uint8_t channel = recorded_notes_[i].channel;
      if (used_channels[channel]) {
        Part* part = multi->mutable_part(part_index);
        part->Set(PART_MIDI_CHANNEL, channel);
        
        if (channel == 9) {
          part->Set(PART_MIDI_MIN_NOTE, recorded_notes_[i].note);
          part->Set(PART_MIDI_MAX_NOTE, recorded_notes_[i].note);
          used_channels[channel] = true;
        } else {
          part->Set(PART_MIDI_MIN_NOTE, 0);
          part->Set(PART_MIDI_MAX_NOTE, 127);
          used_channels[channel] = false;  // Mark the channel as processed.
        }

        // Register splits.
        for (uint8_t j = 0; j < num_splits; ++j) {
          const Split& split = splits_[j];
          if (split.channel == channel) {
            part->Set(PART_MIDI_MAX_NOTE, split.split_point);
            part = multi->mutable_part(++part_index);
            part->Set(PART_MIDI_CHANNEL, channel);
            part->Set(PART_MIDI_MIN_NOTE, split.split_point + 1);
            part->Set(PART_MIDI_MAX_NOTE, 127);
          }
        }
        ++part_index;
      }
    }
    if (part_index == 3) {
      multi->mutable_part(3)->Set(PART_MIDI_MIN_NOTE, 0);
      multi->mutable_part(3)->Set(PART_MIDI_MAX_NOTE, 0);
    }
  }
}

}  // namespace yarns
