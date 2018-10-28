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
// LPC10 encoded words extracted from various TI ROMs.

#ifndef PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_WORDS_H_
#define PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_WORDS_H_

#include "plaits/dsp/speech/lpc_speech_synth_controller.h"

namespace plaits {

#define LPC_SPEECH_SYNTH_NUM_WORD_BANKS 5

extern const uint8_t bank_0[1233];
extern const uint8_t bank_1[900];
extern const uint8_t bank_2[1552];
extern const uint8_t bank_3[2524];
extern const uint8_t bank_4[4802];

extern LPCSpeechSynthWordBankData word_banks_[LPC_SPEECH_SYNTH_NUM_WORD_BANKS];

}  // namespace plaits

#endif  // PLAITS_DSP_SPEECH_LPC_SPEECH_SYNTH_WORDS_H_
