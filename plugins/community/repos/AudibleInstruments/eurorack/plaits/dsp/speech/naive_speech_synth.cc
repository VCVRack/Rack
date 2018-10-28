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
// Naive speech synth - made from "synthesizer" building blocks (pulse
// oscillator and zero-delay SVF).

#include "plaits/dsp/speech/naive_speech_synth.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "plaits/resources.h"

namespace plaits {

using namespace std;
using namespace stmlib;

/* static */
NaiveSpeechSynth::Phoneme NaiveSpeechSynth::phonemes_[][kNaiveSpeechNumRegisters] = {
  {
    { { { 74, 255 }, { 83, 114 }, { 97, 90 }, { 98, 90 }, { 100, 25 } } },
    { { { 75, 255 }, { 84, 128 }, { 100, 114 }, { 101, 101 }, { 103, 20 } } },
    { { { 76, 255 }, { 85, 128 }, { 100, 18 }, { 102, 16 }, { 104, 3 } } },
    { { { 79, 255 }, { 85, 161 }, { 101, 25 }, { 104, 4 }, { 110, 0 } } },
    { { { 79, 255 }, { 85, 128 }, { 101, 6 }, { 106, 25 }, { 110, 0 } } }
  },
  {
    { { { 67, 255 }, { 91, 64 }, { 98, 90 }, { 101, 64 }, { 102, 32 } } },
    { { { 67, 255 }, { 92, 51 }, { 99, 64 }, { 103, 51 }, { 105, 25 } } },
    { { { 69, 255 }, { 93, 51 }, { 100, 32 }, { 102, 25 }, { 103, 25 } } },
    { { { 67, 255 }, { 91, 16 }, { 100, 8 }, { 103, 4 }, { 110, 0 } } },
    { { { 65, 255 }, { 95, 25 }, { 101, 45 }, { 105, 2 }, { 110, 0 } } }
  },
  {
    { { { 59, 255 }, { 92, 8 }, { 99, 40 }, { 102, 20 }, { 104, 10 } } },
    { { { 61, 255 }, { 94, 45 }, { 101, 32 }, { 103, 25 }, { 105, 8 } } },
    { { { 60, 255 }, { 93, 16 }, { 101, 16 }, { 104, 4 }, { 105, 4 } } },
    { { { 65, 255 }, { 92, 25 }, { 100, 8 }, { 105, 4 }, { 110, 0 } } },
    { { { 60, 255 }, { 96, 64 }, { 101, 12 }, { 106, 12 }, { 110, 1 } } }
  },
  {
    { { { 67, 255 }, { 78, 72 }, { 98, 22 }, { 99, 25 }, { 101, 2 } } },
    { { { 67, 255 }, { 79, 80 }, { 99, 64 }, { 101, 64 }, { 102, 12 } } },
    { { { 68, 255 }, { 79, 80 }, { 100, 12 }, { 102, 20 }, { 103, 5 } } },
    { { { 69, 255 }, { 79, 90 }, { 101, 40 }, { 104, 10 }, { 110, 0 } } },
    { { { 69, 255 }, { 79, 72 }, { 101, 20 }, { 106, 20 }, { 110, 0 } } }
  },
  {
    { { { 65, 255 }, { 74, 25 }, { 98, 6 }, { 100, 10 }, { 101, 4 } } },
    { { { 65, 255 }, { 74, 25 }, { 100, 36 }, { 101, 51 }, { 103, 12 } } },
    { { { 66, 255 }, { 75, 25 }, { 100, 18 }, { 102, 8 }, { 104, 5 } } },
    { { { 63, 255 }, { 77, 64 }, { 99, 8 }, { 104, 2 }, { 110, 0 } } },
    { { { 63, 255 }, { 77, 40 }, { 100, 4 }, { 106, 2 }, { 110, 0 } } }
  },
};

void NaiveSpeechSynth::Init() {
  pulse_.Init();
  frequency_ = 0.0f;
  click_duration_ = 0;
  
  for (int i = 0; i < kNaiveSpeechNumFormants; ++i) {
    filter_[i].Init();
  }
  pulse_coloration_.Init();
  pulse_coloration_.set_f_q<FREQUENCY_DIRTY>(800.0f / kSampleRate, 0.5f);
}

void NaiveSpeechSynth::Render(
    bool click,
    float frequency,
    float phoneme,
    float vocal_register,
    float* temp,
    float* excitation,
    float* output,
    size_t size) {
  if (click) {
    click_duration_ = kSampleRate * 0.05f;
  }
  click_duration_ -= min(click_duration_, size);
  
  if (click_duration_) {
    frequency *= 0.5f;
  }
  
  // Generate excitation signal (glottal pulse).
  pulse_.Render<OSCILLATOR_SHAPE_IMPULSE_TRAIN>(
      frequency, 0.5f, excitation, size);
  pulse_coloration_.Process<FILTER_MODE_BAND_PASS>(
      excitation, excitation, size);
  for (size_t i = 0; i < size; ++i) {
    excitation[i] *= 4.0f;
  }
  
  float p = phoneme * (kNaiveSpeechNumPhonemes - 1.001f);
  float r = vocal_register * (kNaiveSpeechNumRegisters - 1.001f);
  
  MAKE_INTEGRAL_FRACTIONAL(p);
  MAKE_INTEGRAL_FRACTIONAL(r);
  
  fill(&output[0], &output[size], 0.0f);
  for (int i = 0; i < kNaiveSpeechNumFormants; ++i) {
    const Formant& p0r0 = phonemes_[p_integral][r_integral].formant[i];
    const Formant& p0r1 = phonemes_[p_integral][r_integral + 1].formant[i];
    const Formant& p1r0 = phonemes_[p_integral + 1][r_integral].formant[i];
    const Formant& p1r1 = phonemes_[p_integral + 1][r_integral + 1].formant[i];

    float p0r_f = p0r0.frequency + \
        (p0r1.frequency - p0r0.frequency) * r_fractional;
    float p1r_f = p1r0.frequency + \
        (p1r1.frequency - p1r0.frequency) * r_fractional;
    float f = p0r_f + (p1r_f - p0r_f) * p_fractional;

    float p0r_a = p0r0.amplitude + \
        (p0r1.amplitude - p0r0.amplitude) * r_fractional;
    float p1r_a = p1r0.amplitude + \
        (p1r1.amplitude - p1r0.amplitude) * r_fractional;
    float a = (p0r_a + (p1r_a - p0r_a) * p_fractional) / 256.0f;
    
    if (f >= 160.0f) {
      f = 160.0f;
    }
    f = a0 * stmlib::SemitonesToRatio(f - 33.0f);
    if (click_duration_ && i == 0) {
      f *= 0.5f;
    }
    filter_[i].set_f_q<FREQUENCY_DIRTY>(f, 20.0f);
    filter_[i].ProcessAdd<FILTER_MODE_BAND_PASS>(excitation, output, size, a);
  }
}

}  // namespace plaits
