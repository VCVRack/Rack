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
//
// Stack of currently pressed keys.
//
// Currently pressed keys are stored as a linked list. The linked list is used
// as a LIFO stack to allow monosynth-like behaviour. An example of such
// behaviour is:
// player presses and holds C4-> C4 is played.
// player presses and holds C5 (while holding C4) -> C5 is played.
// player presses and holds G4 (while holding C4&C5)-> G4 is played.
// player releases C5 -> G4 is played.
// player releases G4 -> C4 is played.
//
// The nodes used in the linked list are pre-allocated from a pool of 16
// nodes, so the "pointers" (to the root element for example) are not actual
// pointers, but indices of an element in the pool.
//
// Additionally, an array of pointers is stored to allow random access to the
// n-th note, sorted by ascending order of pitch (for arpeggiation).

#ifndef EDGES_NOTE_STACK_H_
#define EDGES_NOTE_STACK_H_

#include "avrlibx/avrlibx.h"

#include <string.h>

namespace edges {

static const uint8_t kFreeSlot = 0xff;

struct NoteEntry {
  uint8_t note;
  uint8_t velocity;
  uint8_t next_ptr;  // Base 1.
};

// This looks crazy, but we are more concerned about RAM used than code size here.
template<uint8_t capacity>
class NoteStack {
 public: 
  NoteStack() { }
  void Init() { Clear(); }

  void NoteOn(uint8_t note, uint8_t velocity) {
    // Remove the note from the list first (in case it is already here).
    NoteOff(note);
    // In case of saturation, remove the least recently played note from the
    // stack.
    if (size_ == capacity) {
      uint8_t least_recent_note;
      for (uint8_t i = 1; i <= capacity; ++i) {
        if (pool_[i].next_ptr == 0) {
          least_recent_note = pool_[i].note;
        }
      }
      NoteOff(least_recent_note);
    }
    // Now we are ready to insert the new note. Find a free slot to insert it.
    uint8_t free_slot;
    for (uint8_t i = 1; i <= capacity; ++i) {
      if (pool_[i].note == kFreeSlot) {
        free_slot = i;
        break;
      }
    }
    pool_[free_slot].next_ptr = root_ptr_;
    pool_[free_slot].note = note;
    pool_[free_slot].velocity = velocity;
    root_ptr_ = free_slot;
    // The last step consists in inserting the note in the sorted list.
    for (uint8_t i = 0; i < size_; ++i) {
      if (pool_[sorted_ptr_[i]].note > note) {
        for (uint8_t j = size_; j > i; --j) {
          sorted_ptr_[j] = sorted_ptr_[j - 1];
        }
        sorted_ptr_[i] = free_slot;
        free_slot = 0;
        break;
      }
    }
    if (free_slot) {
      sorted_ptr_[size_] = free_slot;
    }
    ++size_;
  }
  
  void NoteOff(uint8_t note) {
    uint8_t current = root_ptr_;
    uint8_t previous = 0;
    while (current) {
     if (pool_[current].note == note) {
       break;
     }
     previous = current;
     current = pool_[current].next_ptr;
    }
    if (current) {
     if (previous) {
       pool_[previous].next_ptr = pool_[current].next_ptr;
     } else {
       root_ptr_ = pool_[current].next_ptr;
     }
     for (uint8_t i = 0; i < size_; ++i) {
       if (sorted_ptr_[i] == current) {
         for (uint8_t j = i; j < size_ - 1; ++j) {
           sorted_ptr_[j] = sorted_ptr_[j + 1];
         }
         break;
       }
     }
     pool_[current].next_ptr = 0;
     pool_[current].note = kFreeSlot;
     pool_[current].velocity = 0;
     --size_;
    }
  }
  
  void Clear() {
    size_ = 0;
    memset(pool_ + 1, 0, sizeof(NoteEntry) * capacity);
    memset(sorted_ptr_ + 1, 0, capacity);
    root_ptr_ = 0;
    for (uint8_t i = 0; i <= capacity; ++i) {
      pool_[i].note = kFreeSlot;
    }
  }

  uint8_t size() const { return size_; }
  const NoteEntry& most_recent_note() const { return pool_[root_ptr_]; }
  const NoteEntry& least_recent_note() const {
    uint8_t current = root_ptr_;
    while (current && pool_[current].next_ptr) {
      current = pool_[current].next_ptr;
    }
    return pool_[current];
  }
  const NoteEntry& played_note(uint8_t index) const {
    uint8_t current = root_ptr_;
    index = size_ - index - 1;
    for (uint8_t i = 0; i < index; ++i) {
      current = pool_[current].next_ptr;
    }
    return pool_[current];
  }
  const NoteEntry& sorted_note(uint8_t index) const {
    return pool_[sorted_ptr_[index]];
  }
  const NoteEntry& note(uint8_t index) const { return pool_[index]; }
  const NoteEntry& dummy() const { return pool_[0]; }

 private:
  uint8_t size_;
  NoteEntry pool_[capacity + 1];  // First element is a dummy node!
  uint8_t root_ptr_;  // Base 1.
  uint8_t sorted_ptr_[capacity + 1];  // Base 1.

  DISALLOW_COPY_AND_ASSIGN(NoteStack);
};

}  // namespace edges

#endif  // EDGES_NOTE_STACK_H_
