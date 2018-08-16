// Copyright 2014 Olivier Gillet.
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
// Modal synthesis voice.

#include "elements/dsp/voice.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"

#include <algorithm>

namespace elements {

using namespace std;
using namespace stmlib;

void Voice::Init() {
  envelope_.Init();
  bow_.Init();
  blow_.Init();
  strike_.Init();
  diffuser_.Init(diffuser_buffer_);
  
  ResetResonator();

  bow_.set_model(EXCITER_MODEL_FLOW);
  bow_.set_parameter(0.7f);
  bow_.set_timbre(0.5f);
  
  blow_.set_model(EXCITER_MODEL_GRANULAR_SAMPLE_PLAYER);
  
  envelope_.set_adsr(0.5f, 0.5f, 0.5f, 0.5f);

  previous_gate_ = false;
  strength_ = 0.0f;
  exciter_level_ = 0.0f;
  envelope_value_ = 0.0f;
  chord_index_ = 0.0f;
  
  resonator_model_ = RESONATOR_MODEL_MODAL;
}

void Voice::ResetResonator() {
  resonator_.Init();
  for (size_t i = 0; i < kNumStrings; ++i) {
    string_[i].Init(true);
  }
  dc_blocker_.Init(1.0f - 10.0f / kSampleRate);
  resonator_.set_resolution(52);  // Runs with 56 extremely tightly.
}

float chords[11][5] = {
    { 0.0f, -12.0f, 0.0f, 0.01f, 12.0f },
    { 0.0f, -12.0f, 3.0f, 7.0f,  10.0f },
    { 0.0f, -12.0f, 3.0f, 7.0f,  12.0f },
    { 0.0f, -12.0f, 3.0f, 7.0f,  14.0f },
    { 0.0f, -12.0f, 3.0f, 7.0f,  17.0f },
    { 0.0f, -12.0f, 7.0f, 12.0f, 19.0f },
    { 0.0f, -12.0f, 4.0f, 7.0f,  17.0f },
    { 0.0f, -12.0f, 4.0f, 7.0f,  14.0f },
    { 0.0f, -12.0f, 4.0f, 7.0f,  12.0f },
    { 0.0f, -12.0f, 4.0f, 7.0f,  11.0f },
    { 0.0f, -12.0f, 5.0f, 7.0f,  12.0f },
};

void Voice::Process(
    const Patch& patch,
    float frequency,
    float strength,
    const bool gate_in,
    const float* blow_in,
    const float* strike_in,
    float* raw,
    float* center,
    float* sides,
    size_t size) {
  uint8_t flags = GetGateFlags(gate_in);

  // Compute the envelope.
  float envelope_gain = 1.0f;
  if (patch.exciter_envelope_shape < 0.4f) {
    float a = patch.exciter_envelope_shape * 0.75f + 0.15f;
    float dr = a * 1.8f;
    envelope_.set_adsr(a, dr, 0.0f, dr);
    envelope_gain = 5.0f - patch.exciter_envelope_shape * 10.0f;
  } else if (patch.exciter_envelope_shape < 0.6f) {
    float s = (patch.exciter_envelope_shape - 0.4f) * 5.0f;
    envelope_.set_adsr(0.45f, 0.81f, s, 0.81f);
  } else {
    float a = (1.0f - patch.exciter_envelope_shape) * 0.75f + 0.15f;
    float dr = a * 1.8f;
    envelope_.set_adsr(a, dr, 1.0f, dr);
  }
  float envelope_value = envelope_.Process(flags) * envelope_gain;
  float envelope_increment = (envelope_value - envelope_value_) / size;
  
  // Configure and evaluate exciters.
  float brightness_factor = 0.4f + 0.6f * patch.resonator_brightness;
  bow_.set_timbre(patch.exciter_bow_timbre * brightness_factor);

  blow_.set_parameter(patch.exciter_blow_meta);
  blow_.set_timbre(patch.exciter_blow_timbre);
  blow_.set_signature(patch.exciter_signature);
  
  float strike_meta = patch.exciter_strike_meta;
  strike_.set_meta(
      strike_meta <= 0.4f ? strike_meta * 0.625f : strike_meta * 1.25f - 0.25f,
      EXCITER_MODEL_SAMPLE_PLAYER,
      EXCITER_MODEL_PARTICLES);
  strike_.set_timbre(patch.exciter_strike_timbre);
  strike_.set_signature(patch.exciter_signature);

  bow_.Process(flags, bow_buffer_, size);
  
  float blow_level, tube_level;
  blow_level = patch.exciter_blow_level * 1.5f;
  tube_level = blow_level > 1.0f ? (blow_level - 1.0f) * 2.0f : 0.0f;
  blow_level = blow_level < 1.0f ? blow_level * 0.4f : 0.4f;
  blow_.Process(flags, blow_buffer_, size);
  tube_.Process(
      frequency,
      envelope_value,
      patch.resonator_damping,
      tube_level,
      blow_buffer_,
      tube_level * 0.5f,
      size);
  
  for (size_t i = 0; i < size; ++i) {
    blow_buffer_[i] = blow_buffer_[i] * blow_level + blow_in[i];
  }
  diffuser_.Process(blow_buffer_, size);
  strike_.Process(flags, strike_buffer_, size);
  
  // The Strike exciter is implemented in such a way that raising the level
  // beyond a certain point doesn't change the exciter amplitude, but instead,
  // increasingly mixes the raw exciter signal into the resonator output.
  float strike_level, strike_bleed;
  strike_level = patch.exciter_strike_level * 1.25f;
  strike_bleed = strike_level > 1.0f ? (strike_level - 1.0f) * 2.0f : 0.0f;
  strike_level = strike_level < 1.0f ? strike_level : 1.0f;
  strike_level *= 1.5f;
  
  // The strength parameter is very sensitive to zipper noise.
  strength *= 256.0f;
  float strength_increment = (strength - strength_) / size;
  
  // Sum all sources of excitation.
  for (size_t i = 0; i < size; ++i) {
    strength_ += strength_increment;
    envelope_value_ += envelope_increment;
    float input_sample = 0.0f;
    float e = envelope_value_;
    float strength_lut = strength_;
    MAKE_INTEGRAL_FRACTIONAL(strength_lut);
    float accent = lut_accent_gain_coarse[strength_lut_integral] *
       lut_accent_gain_fine[
           static_cast<int32_t>(256.0f * strength_lut_fractional)];
    bow_strength_buffer_[i] = e * patch.exciter_bow_level;

    strike_buffer_[i] *= accent;
    e *= accent;

    input_sample += bow_buffer_[i] * bow_strength_buffer_[i] * 0.125f * accent;
    input_sample += blow_buffer_[i] * e;
    input_sample += strike_buffer_[i] * strike_level;
    input_sample += strike_in[i];
    raw[i] = input_sample * 0.5f;
  }
  
  // Update meter for exciter.
  for (size_t i = 0; i < size; ++i) {
    float error = raw[i] * raw[i] - exciter_level_;
    exciter_level_ += error * (error > 0.0f ? 0.5f : 0.001f);
  }
  
  // Some exciters can cause palm mutes on release.
  float damping = patch.resonator_damping;
  damping -= strike_.damping() * strike_level * 0.125f;
  damping -= (1.0f - bow_strength_buffer_[0]) * \
      patch.exciter_bow_level * 0.0625f;
  
  if (damping <= 0.0f) {
    damping = 0.0f;
  }

  // Configure resonator.
  if (resonator_model_ == RESONATOR_MODEL_MODAL) {
    resonator_.set_frequency(frequency);
    resonator_.set_geometry(patch.resonator_geometry);
    resonator_.set_brightness(patch.resonator_brightness);
    resonator_.set_position(patch.resonator_position);
    resonator_.set_damping(damping);
    resonator_.set_modulation_frequency(patch.resonator_modulation_frequency);
    resonator_.set_modulation_offset(patch.resonator_modulation_offset);

    // Process through resonator.
    resonator_.Process(bow_strength_buffer_, raw, center, sides, size);
  } else {
    size_t num_notes = resonator_model_ == RESONATOR_MODEL_STRING
        ? 1
        : kNumStrings;
    
    float normalization = 1.0f / static_cast<float>(num_notes);
    dc_blocker_.Process(raw, size);
    for (size_t i = 0; i < size; ++i) {
      raw[i] *= normalization;
    }
    
    float chord = patch.resonator_geometry * 10.0f;
    float hysteresis = chord > chord_index_ ? -0.1f : 0.1f;
    int chord_index = static_cast<int>(chord + hysteresis + 0.5f);
    CONSTRAIN(chord_index, 0, 10);
    chord_index_ = static_cast<float>(chord_index);

    fill(&center[0], &center[size], 0.0f);
    fill(&sides[0], &sides[size], 0.0f);
    for (size_t i = 0; i < num_notes; ++i) {
      float transpose = chords[chord_index][i];
      string_[i].set_frequency(frequency * SemitonesToRatio(transpose));
      string_[i].set_brightness(patch.resonator_brightness);
      string_[i].set_position(patch.resonator_position);
      string_[i].set_damping(damping);
      if (num_notes == 1) {
        string_[i].set_dispersion(patch.resonator_geometry);
      } else {
        float b = patch.resonator_brightness;
        string_[i].set_dispersion(b < 0.5f ? 0.0f : (b - 0.5f) * -0.4f);
      }
      string_[i].Process(raw, center, sides, size);
    }
    for (size_t i = 0; i < size; ++i) {
      float left = center[i];
      float right = sides[i];
      center[i] = left - right;
      sides[i] = left + right;
    }
  }

  // This is where the raw mallet signal bleeds through the exciter output.
  for (size_t i = 0; i < size; ++i) {
    center[i] += strike_bleed * strike_buffer_[i];
  }
}

}  // namespace elements
