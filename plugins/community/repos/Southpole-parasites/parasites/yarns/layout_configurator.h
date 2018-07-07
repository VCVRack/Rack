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

#ifndef YARNS_LAYOUT_CONFIGURATOR_H_
#define YARNS_LAYOUT_CONFIGURATOR_H_

#include "stmlib/stmlib.h"

namespace yarns {

const uint8_t kMaxNumNotes = 8;

class Multi;

class LayoutConfigurator {
 public:
  LayoutConfigurator() {
    learning_ = false;
    num_notes_ = 0;
  }
  ~LayoutConfigurator() { }
  
  void StartLearning() {
    num_notes_ = 0;
    learning_ = true;
  }
  
  void StopLearning(Multi* m);
  
  void RegisterNote(uint8_t channel, uint8_t note) {
    if (num_notes_ < kMaxNumNotes) {
      NoteEntry n;
      n.channel = channel;
      n.note = note;
      recorded_notes_[num_notes_] = n;
      num_notes_++;
    }
  }
  
  inline bool learning() const {
    return learning_ && num_notes_ < kMaxNumNotes;
  }
  
  inline uint8_t num_notes() const {
    return num_notes_;
  }

 private:
  struct NoteEntry {
    uint8_t channel;
    uint8_t note;
  };

  struct Split {
    uint8_t channel;
    uint8_t split_point;
  };

  struct SplitLess {
    bool operator()(const Split& lhs, const Split& rhs) {
      if (lhs.channel == rhs.channel) {
        return lhs.split_point < rhs.split_point;
      } else {
        return lhs.channel < rhs.channel;
      }
    }
  };
  
  bool learning_;
  uint8_t num_notes_;
  NoteEntry recorded_notes_[kMaxNumNotes];
  Split splits_[kMaxNumNotes];
  
  DISALLOW_COPY_AND_ASSIGN(LayoutConfigurator);
};

extern Multi multi;

}  // namespace yarns

#endif // YARNS_LAYOUT_CONFIGURATOR_H_
