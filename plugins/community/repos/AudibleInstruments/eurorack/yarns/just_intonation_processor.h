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
// Just intonation processor. This use a very simple algorithm, which gives
// surprising nice results in simple cases.
//
// A history of the previous notes is kept in a circular buffer. For memory
// and efficienty reason, we keep here the 16 previous note. Each note is
// given a weight. The weight of a note stays 1.0 while the note is still heard,
// but it decays exponentially once the note is no longer played.
//
// To tune a note, each possible tuning in the +/- 1 quartertone range is
// considered. For each tuning, a "badness" score is computed. The badness score
// is equal to the weighted sum of the "dissonance" score of the interval
// between the candidate tuning and the tuning of each previous note in the
// history. The dissonance score is read from a lookup table. It has a low
// value for just intervals (say 4/3), which goes slightly higher as the
// interval involves more convoluted ratios (say 32/27), and goes up according
// to a square law as we move away from the just intervals.
// The tuning giving the least badness score is selected.

#ifndef YARNS_JUST_INTONATION_PROCESSOR_H_
#define YARNS_JUST_INTONATION_PROCESSOR_H_

#include "stmlib/stmlib.h"

namespace yarns {

struct HistoryEntry {
  uint8_t note;
  uint8_t weight;
  int16_t pitch;
};

const size_t kHistorySize = 16;

class JustIntonationProcessor {
 public:
  JustIntonationProcessor() { }
  ~JustIntonationProcessor() { }
  
  void Init();
  
  inline void NoteOff(uint8_t note) {
    for (uint8_t i = 0; i < kHistorySize; ++i) {
      if (history_[i].note == note && history_[i].weight == 255) {
        history_[i].weight = 192;
      }
    }
  }
  
  inline int16_t NoteOn(uint8_t note) {
    if (note != cached_note_) {
      // Skip the computationally expensive routine on expensive notes.
      cached_note_ = note;
      cached_pitch_ = Tune(static_cast<int>(note) << 7);
    }
    // Decay the weight of the previous notes - except those that are still
    // playing.
    for (size_t i = 0; i < kHistorySize; ++i) {
      if (history_[i].weight != 255) {
        history_[i].weight = (history_[i].weight * 3) >> 2;
      }
    }
    history_[write_ptr_].note = note;
    history_[write_ptr_].weight = 255;
    history_[write_ptr_].pitch = cached_pitch_;
    ++write_ptr_;
    if (write_ptr_ >= kHistorySize) {
      write_ptr_ = 0;
    }
    return cached_pitch_;
  }
  
 private:
  int Tune(int note, int min, int max, int steps);
  
  int16_t Tune(int note) {
    int coarse = Tune(note, -32, 32, 4);
    return int16_t(note + Tune(note, coarse - 6, coarse + 6, 1));
  }

  size_t write_ptr_;
  int16_t cached_pitch_;
  uint8_t cached_note_;
  HistoryEntry history_[kHistorySize];
  
  DISALLOW_COPY_AND_ASSIGN(JustIntonationProcessor);
};

extern JustIntonationProcessor just_intonation_processor;

}  // namespace yarns

#endif // YARNS_JUST_INTONATION_PROCESSOR_H_
