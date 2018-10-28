// Copyright 2016 Olivier Gillet.
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
// This is a table with vowels (a, e, i, o, u) and a random selection of
// consonnants for the LPC10 speech synth.

#include "plaits/dsp/speech/lpc_speech_synth_controller.h"

namespace plaits {

/* static */
const LPCSpeechSynth::Frame
  LPCSpeechSynthController::phonemes_[kLPCSpeechSynthNumPhonemes] = {
  {
    192,
    80,
    -18368, 11584, 52, 29, 23,
    14, -17, 79, 37, 4
  },
  {
    192,
    80,
    -14528, 1536, 38, 29, 11,
    14, -41, 79, 57, 4
  },
  {
    192,
    80,
    14528, 9216, 25, -54, -70,
    36, 19, 79, 57, 22
  },
  {
    192,
    80,
    -14528, -13440, 38, 57, 57,
    14, -53, 7, 37, 77
  },
  {
    192,
    80,
    -26368, 4160, 11, 15, -1,
    36, -41, 31, 77, 22
  },
  {
    15,
    0,
    5184, 9216, -29, -12, 0,
    0, 0, 0, 0, 0
  },
  {
    10,
    0,
    27968, 17856, 25, 43, -24,
    -20, -53, 55, -4, -51
  },
  {
    128,
    160,
    14528, -3712, -43, -26, -24,
    -20, -53, 55, -4, -51
  },
  {
    128,
    160,
    10048, 11584, -16, 15, 0,
    0, 0, 0, 0, 0
  },
  {
    224,
    100,
    18368, -13440, -97, -26, -12,
    -53, -41, 7, 57, 32
  },
  { 192,
    80,
    -10048, 9216, -70, 15, 34,
    -20, -17, 31, -24, 22
  },
  { 96,
    160,
    -18368, 17856, -29, -12, -35,
    3, -5, 7, 37, 22
  },
  { 64,
    80,
    -21632, -6272, -83, 29, 57,
    3, -5, 7, 16, 32
  },
  {
    192,
    80,
    0, -1088, 11, -26, -24,
    -9, -5, 55, 37, 22
  },
  {
    64,
    80,
    21632, -17536, -97, 85, 57,
    -20, -17, 31, -4, 59
  }
};

}  // namespace plaits
