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
// Digital oscillator generated from a timer.

#include "edges/digital_oscillator.h"

#include "avrlibx/utils/op.h"

#include "edges/audio_buffer.h"
#include "edges/resources.h"

namespace edges {

static const uint8_t kMaxZone = 7;
static const int16_t kOctave = 12 * 128;
static const int16_t kPitchTableStart = 116 * 128;
  
using namespace avrlibx;

#define UPDATE_PHASE \
  phase = U24Add(phase, phase_increment);

#define BEGIN_SAMPLE_LOOP \
  uint24_t phase; \
  uint24_t phase_increment; \
  phase_increment.integral = phase_increment_.integral; \
  phase_increment.fractional = phase_increment_.fractional; \
  phase.integral = phase_.integral; \
  phase.fractional = phase_.fractional; \
  uint8_t size = kAudioBlockSize; \
  while (size--) {

#define END_SAMPLE_LOOP \
  } \
  phase_.integral = phase.integral; \
  phase_.fractional = phase.fractional;

void DigitalOscillator::ComputePhaseIncrement() {
  int16_t ref_pitch = pitch_ - kPitchTableStart;
  uint8_t num_shifts = shape_ >= OSC_PITCHED_NOISE ? 0 : 1;
  while (ref_pitch < 0) {
    ref_pitch += kOctave;
    ++num_shifts;
  }
  uint24_t increment;
  uint16_t pitch_lookup_index_integral = U16ShiftRight4(ref_pitch);
  uint8_t pitch_lookup_index_fractional = U8ShiftLeft4(ref_pitch);
  uint16_t increment16 = ResourcesManager::Lookup<uint16_t, uint16_t>(
      lut_res_oscillator_increments, pitch_lookup_index_integral);
  uint16_t increment16_next = ResourcesManager::Lookup<uint16_t, uint16_t>(
      lut_res_oscillator_increments, pitch_lookup_index_integral + 1);
  increment.integral = increment16 + U16U8MulShift8(
      increment16_next - increment16, pitch_lookup_index_fractional);
  increment.fractional = 0;
  while (num_shifts--) {
    increment = U24ShiftRight(increment);
  }
  
  note_ = U15ShiftRight7(pitch_);
  if (note_ < 12) {
    note_ = 12;
  }
  phase_increment_ = increment;
}

void DigitalOscillator::Render() {
  if (gate_) {
    ComputePhaseIncrement();
  }
  while (audio_buffer.writable() >= kAudioBlockSize) {
    if (!gate_) {
      RenderSilence();
    } else {
      RenderFn fn;
      ResourcesManager::Load(fn_table_, shape_, &fn);
      (this->*fn)();
    }
  }
}

static inline uint8_t InterpolateSample(
    const prog_uint8_t* table,
    uint16_t phase) {
  uint8_t result;
  uint8_t work;
  asm(
    "movw r30, %A2"           "\n\t"  // copy base address to r30:r31
    "mov %1, %A3"             "\n\t"  // duplicate phase increment
    "add %1, %A3"             "\n\t"  // duplicate
    "adc r30, %B3"            "\n\t"  // duplicate
    "adc r31, r1"             "\n\t"  // duplicate
    "add r30, %B3"            "\n\t"  // duplicate
    "adc r31, r1"             "\n\t"  // duplicate
    "lpm %0, z+"              "\n\t"  // load sample[n]
    "lpm r1, z+"              "\n\t"  // load sample[n+1]
    "mul %1, r1"              "\n\t"  // multiply second sample by phaseL
    "movw r30, r0"            "\n\t"  // result to accumulator
    "com %1"                  "\n\t"  // 255 - phaseL -> phaseL
    "mul %1, %0"              "\n\t"  // multiply first sample by phaseL
    "add r30, r0"             "\n\t"  // accumulate L
    "adc r31, r1"             "\n\t"  // accumulate H
    "eor r1, r1"              "\n\t"  // reset r1 after multiplication
    "mov %0, r31"             "\n\t"  // use sum H as output
    : "=r" (result), "=r" (work)
    : "r" (table), "r" (phase)
    : "r30", "r31"
  );
  return result;
}

static inline uint16_t InterpolateSample16(
    const prog_uint8_t* table,
    uint16_t phase) {
  uint16_t result;
  uint8_t work;
  asm(
    "movw r30, %A2"           "\n\t"  // copy base address to r30:r31
    "mov %1, %A3"             "\n\t"  // duplicate phase increment
    "add %1, %A3"             "\n\t"  // duplicate
    "adc r30, %B3"            "\n\t"  // duplicate
    "adc r31, r1"             "\n\t"  // duplicate
    "add r30, %B3"            "\n\t"  // duplicate
    "adc r31, r1"             "\n\t"  // duplicate
    "lpm %0, z+"              "\n\t"  // load sample[n]
    "lpm r1, z+"              "\n\t"  // load sample[n+1]
    "mul %1, r1"              "\n\t"  // multiply second sample by phaseL
    "movw r30, r0"            "\n\t"  // result to accumulator
    "com %1"                  "\n\t"  // 255 - phaseL -> phaseL
    "mul %1, %0"              "\n\t"  // multiply first sample by phaseL
    "add r30, r0"             "\n\t"  // accumulate L
    "adc r31, r1"             "\n\t"  // accumulate H
    "eor r1, r1"              "\n\t"  // reset r1 after multiplication
    "movw %0, r30"            "\n\t"  // use sum H as output
    : "=r" (result), "=r" (work)
    : "r" (table), "r" (phase)
    : "r30", "r31"
  );
  return result;
}


static inline uint16_t InterpolateTwoTables(
    const prog_uint8_t* table_a, const prog_uint8_t* table_b,
    uint16_t phase, uint8_t gain_a, uint8_t gain_b) {
  uint16_t result = 0;
  result += U8U8Mul(InterpolateSample(table_a, phase), gain_a);
  result += U8U8Mul(InterpolateSample(table_b, phase), gain_b);
  return result;
}

void DigitalOscillator::RenderSilence() {
  uint8_t size = kAudioBlockSize;
  while (size--) {
    audio_buffer.Overwrite(2048);
  }
}

void DigitalOscillator::RenderSine() {
  uint16_t aux_phase_increment = pgm_read_word(
      lut_res_bitcrusher_increments + cv_pw_);
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    aux_phase_ += aux_phase_increment;
    if (aux_phase_ < aux_phase_increment || !aux_phase_increment) {
      sample_ = InterpolateSample16(
          wav_res_bandlimited_triangle_6,
          phase.integral);
    }
    audio_buffer.Overwrite(sample_ >> 4);
  END_SAMPLE_LOOP
}

void DigitalOscillator::RenderBandlimitedTriangle() {
  uint8_t balance_index = U8Swap4(note_ - 12);
  uint8_t gain_2 = balance_index & 0xf0;
  uint8_t gain_1 = ~gain_2;

  uint8_t wave_index = balance_index & 0xf;
  uint8_t base_resource_id = (shape_ == OSC_NES_TRIANGLE)
      ? WAV_RES_BANDLIMITED_NES_TRIANGLE_0
      : WAV_RES_BANDLIMITED_TRIANGLE_0;

  const prog_uint8_t* wave_1 = waveform_table[base_resource_id + wave_index];
  wave_index = U8AddClip(wave_index, 1, kMaxZone);
  const prog_uint8_t* wave_2 = waveform_table[base_resource_id + wave_index];
  
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint16_t sample = InterpolateTwoTables(
        wave_1, wave_2,
        phase.integral, gain_1, gain_2);
    audio_buffer.Overwrite(sample >> 4);
  END_SAMPLE_LOOP
}

void DigitalOscillator::RenderNoiseNES() {
  uint16_t rng_state = rng_state_;
  uint16_t sample = sample_;
  BEGIN_SAMPLE_LOOP
    phase = U24Add(phase, phase_increment);
    if (phase.integral < phase_increment.integral) {
      uint8_t tap = rng_state >> 1;
      if (shape_ == OSC_NES_NOISE_SHORT) {
        tap >>= 5;
      }
      uint8_t random_bit = (rng_state ^ tap) & 1;
      rng_state >>= 1;
      if (random_bit) {
        rng_state |= 0x4000;
        sample = 0x0300;
      } else {
        sample = 0x0cff;
      }
    }
    audio_buffer.Overwrite(sample);
  END_SAMPLE_LOOP
  rng_state_ = rng_state;
  sample_ = sample;
}

void DigitalOscillator::RenderNoise() {
  uint16_t rng_state = rng_state_;
  uint16_t sample = sample_;
  BEGIN_SAMPLE_LOOP
    phase = U24Add(phase, phase_increment);
    if (phase.integral < phase_increment.integral) {
      rng_state = (rng_state >> 1) ^ (-(rng_state & 1) & 0xb400);
      sample = rng_state & 0x0fff;
      sample = 512 + ((sample * 3) >> 2);
    }
    audio_buffer.Overwrite(sample);
  END_SAMPLE_LOOP
  rng_state_ = rng_state;
  sample_ = sample;
}

/* static */
const DigitalOscillator::RenderFn DigitalOscillator::fn_table_[] PROGMEM = {
  &DigitalOscillator::RenderBandlimitedTriangle,
  &DigitalOscillator::RenderBandlimitedTriangle,
  &DigitalOscillator::RenderNoise,
  &DigitalOscillator::RenderNoiseNES,
  &DigitalOscillator::RenderNoiseNES,
  &DigitalOscillator::RenderSine,
};

}  // namespace shruthi
