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
// Sine FM drum - similar to the BD/SD in Anushri.

#include "peaks/drums/fm_drum.h"
#include <cstdio>
#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"

#include "peaks/resources.h"

namespace peaks {

using namespace stmlib;

void FmDrum::Init() {
  phase_ = 0;
  fm_envelope_phase_ = 0xffffffff;
  am_envelope_phase_ = 0xffffffff;
  previous_sample_ = 0;
}

static const uint16_t kHighestNote = 128 * 128;
static const uint16_t kPitchTableStart = 116 * 128;
static const uint16_t kOctave = 128 * 12;
static const uint8_t kNumZones = 6;

static const uint16_t bd_map[10][4] = {
  { 4096, 0, 65535, 32768 },
  { 8192 + 4096, 0, 65535, 32768 },

  { 8192, 4096, 49512, 32768 },
  { 8192, 16384, 40960, 32768 },

  { 10240, 4096, 24576, 32768 },
  { 10240, 16384, 24576, 16384 },

  { 8192, 8192, 32768, 16384 },
  { 8192, 24576, 49152, 8192 },

  { 4096, 16384, 40960, 16384 },
  { 8192, 24576, 49152, 0 },
};

static const uint16_t sd_map[10][4] = {
  { 24576, 0, 24576, 36864 },
  { 24576, 0, 16384, 65535 },

  { 28672, 0, 16384, 36864 },
  { 28672, 0, 16384, 65535 },

  { 20488, 0, 32768, 57344 },
  { 28672, 0, 24576, 65535 },

  { 20488, 0, 24576, 65535 },
  { 28672, 0, 32768, 65535 },

  { 20488, 65535, 16384, 0 },
  { 65535, 0, 8192, 32768 },
};

void FmDrum::Morph(uint16_t x, uint16_t y) {
  const uint16_t (*map)[4] = sd_range_ ? sd_map : bd_map;
  uint16_t parameters[4];
  for (uint8_t i = 0; i < 4; ++i) {
    uint16_t x_integral = (x >> 14) << 1;
    uint16_t x_fractional = x << 2;
    uint16_t a = map[x_integral][i];
    uint16_t b = map[x_integral + 2][i];
    uint16_t c = map[x_integral + 1][i];
    uint16_t d = map[x_integral + 3][i];
    
    uint16_t e = a + ((b - a) * x_fractional >> 16);
    uint16_t f = c + ((d - c) * x_fractional >> 16);
    parameters[i] = e + ((f - e) * y >> 16);
  }
  Configure(parameters, CONTROL_MODE_FULL);
}

uint32_t FmDrum::ComputeEnvelopeIncrement(uint16_t decay) {
  uint32_t a = lut_env_increments[decay >> 8];
  uint32_t b = lut_env_increments[(decay >> 8) + 1];
  return a - ((a - b) * (decay & 0xff) >> 8);
}

uint32_t FmDrum::ComputePhaseIncrement(int16_t midi_pitch) {
  if (midi_pitch >= kHighestNote) {
    midi_pitch = kHighestNote - 1;
  }

  int32_t ref_pitch = midi_pitch;
  ref_pitch -= kPitchTableStart;

  size_t num_shifts = 0;
  while (ref_pitch < 0) {
    ref_pitch += kOctave;
    ++num_shifts;
  }

  uint32_t a = lut_oscillator_increments[ref_pitch >> 4];
  uint32_t b = lut_oscillator_increments[(ref_pitch >> 4) + 1];
  uint32_t phase_increment = a + \
      (static_cast<int32_t>(b - a) * (ref_pitch & 0xf) >> 4);
  phase_increment >>= num_shifts;
  return phase_increment;
}

void FmDrum::FillBuffer(
    InputBuffer* input_buffer,
    OutputBuffer* output_buffer) {
  uint8_t size = kBlockSize;
  uint32_t am_envelope_increment = ComputeEnvelopeIncrement(am_decay_);
  uint32_t fm_envelope_increment = ComputeEnvelopeIncrement(fm_decay_);
  uint32_t phase = phase_;
  uint32_t fm_envelope_phase = fm_envelope_phase_;
  uint32_t am_envelope_phase = am_envelope_phase_;
  uint32_t aux_envelope_phase = aux_envelope_phase_;
  uint32_t phase_increment = phase_increment_;
  while (size--) {
    uint8_t control = input_buffer->ImmediateRead();
    if (control & CONTROL_GATE_RISING) {
      fm_envelope_phase = 0;
      am_envelope_phase = 0;
      aux_envelope_phase = 0;
      phase = 0x3fff * fm_amount_ >> 16;
    }

    fm_envelope_phase += fm_envelope_increment;
    if (fm_envelope_phase < fm_envelope_increment) {
      fm_envelope_phase = 0xffffffff;
    }
    aux_envelope_phase += 4473924;
    if (aux_envelope_phase < 4473924) {
      aux_envelope_phase = 0xffffffff;
    }
    if ((size & 3) == 0) {
      uint32_t aux_envelope = 65535 - Interpolate824(
          lut_env_expo, aux_envelope_phase);
      uint32_t fm_envelope = 65535 - Interpolate824(
          lut_env_expo, fm_envelope_phase);
      phase_increment = ComputePhaseIncrement(
          frequency_ + \
          (fm_envelope * fm_amount_ >> 16) + \
          (aux_envelope * aux_envelope_strength_ >> 15) + \
          (previous_sample_ >> 6));
    }
    phase += phase_increment;

    int16_t mix = Interpolate1022(wav_sine, phase);
    if (noise_) {
      mix = Mix(mix, Random::GetSample(), noise_);
    }

    am_envelope_phase += am_envelope_increment;
    if (am_envelope_phase < am_envelope_increment) {
      am_envelope_phase = 0xffffffff;
    }
    uint32_t am_envelope = 65535 - Interpolate824(
        lut_env_expo, am_envelope_phase);
    mix = mix * am_envelope >> 16;
    if (overdrive_) {
      uint32_t phi = (static_cast<int32_t>(mix) << 16) + (1L << 31);
      int16_t overdriven = Interpolate1022(wav_overdrive, phi);
      mix = Mix(mix, overdriven, overdrive_);
    }
    previous_sample_ = mix;
    output_buffer->Overwrite(mix);
  }
  phase_ = phase;
  fm_envelope_phase_ = fm_envelope_phase;
  am_envelope_phase_ = am_envelope_phase;
  aux_envelope_phase_ = aux_envelope_phase;
  phase_increment_ = phase_increment;
}

}  // namespace peaks
