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
// Square oscillator generated from a timer.

#include <avr/pgmspace.h>

#include "avrlibx/utils/op.h"

#include "edges/resources.h"
#include "edges/timer_oscillator.h"

namespace edges {

using namespace avrlibx;

static const int16_t kOctave = 12 << 7;
static const int16_t kFirstNoteNormalMode = 24 << 7;
static const int16_t kFirstNoteLFOMode = -(12 << 7);

const uint8_t pulse_widths[] = {
  128, 85, 64, 32, 13, 10
};

void TimerOscillator::UpdateTimerParameters(
    int16_t pitch,
    PulseWidth pulse_width) {
  int8_t shifts = 0;
  
  // Lowest note: E0.
  pitch -= prescaler_ == TIMER_PRESCALER_CLK_64 \
      ? kFirstNoteLFOMode
      : kFirstNoteNormalMode;

  // Transpose the lowest octave up.
  while (pitch < 0) {
    pitch += kOctave;
  }
  while (pitch >= kOctave) {
    pitch -= kOctave;
    ++shifts;
  }
  uint16_t index_integral = U16ShiftRight4(pitch);
  uint16_t index_fractional = U8U8Mul(pitch & 0xf, 16);
  uint16_t count = pgm_read_word(lut_res_timer_count + index_integral);
  uint16_t next = pgm_read_word(lut_res_timer_count + index_integral + 1);
  count -= U16U8MulShift8(count - next, index_fractional);
  
  period_ = count >> shifts;
  uint8_t pw = pulse_width == PULSE_WIDTH_CV_CONTROLLED
      ? cv_pw_
      : pulse_widths[pulse_width];
  value_ = U16U8MulShift8(period_, pw);
}

}  // namespace edges
