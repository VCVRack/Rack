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
// "Ominous" is a dark 2x2-op FM synth (based on Braids' FM and FBFM modes).

#include "elements/dsp/ominous_voice.h"

#include <algorithm>
#include <cstdio>

namespace elements {

using namespace std;
using namespace stmlib;

// scipy.signal.remez(101, [0, 0.3 / 8, 0.495 / 8, 0.5], [1, 0]);
const float kDownsamplingFilter[] = {
  -0.001859272945f,  0.001184937535f,  0.001212413444f,  0.001369688661f,
   0.001555406705f,  0.001685761819f,  0.001692922383f,  0.001526182555f,
   0.001157229282f,  0.000582212588f, -0.000172916131f, -0.001054896973f,
  -0.001981709311f, -0.002852566234f, -0.003555372122f, -0.003977161477f,
  -0.004020648315f, -0.003616652394f, -0.002735861951f, -0.001399720214f,
   0.000314317050f,  0.002272758644f,  0.004295073578f,  0.006166173855f,
   0.007654982354f,  0.008534833653f,  0.008614353501f,  0.007761247708f,
   0.005919024408f,  0.003133736224f, -0.000445934810f, -0.004563290490f,
  -0.008867277113f, -0.012932060625f, -0.016287075604f, -0.018456326393f,
  -0.018994127671f, -0.017542469537f, -0.013838153737f, -0.007789574712f,
   0.000541523640f,  0.010920252821f,  0.022936167260f,  0.036030557729f,
   0.049530599001f,  0.062696232995f,  0.074769247024f,  0.085030908865f,
   0.092854033134f,  0.097753623479f,  0.099422113893f,  0.097753623479f,
   0.092854033134f,  0.085030908865f,  0.074769247024f,  0.062696232995f,
   0.049530599001f,  0.036030557729f,  0.022936167260f,  0.010920252821f,
   0.000541523640f, -0.007789574712f, -0.013838153737f, -0.017542469537f,
  -0.018994127671f, -0.018456326393f, -0.016287075604f, -0.012932060625f,
  -0.008867277113f, -0.004563290490f, -0.000445934810f,  0.003133736224f,
   0.005919024408f,  0.007761247708f,  0.008614353501f,  0.008534833653f,
   0.007654982354f,  0.006166173855f,  0.004295073578f,  0.002272758644f,
   0.000314317050f, -0.001399720214f, -0.002735861951f, -0.003616652394f,
  -0.004020648315f, -0.003977161477f, -0.003555372122f, -0.002852566234f,
  -0.001981709311f, -0.001054896973f, -0.000172916131f,  0.000582212588f,
   0.001157229282f,  0.001526182555f,  0.001692922383f,  0.001685761819f,
   0.001555406705f,  0.001369688661f,  0.001212413444f,  0.001184937535f,
  -0.001859272945f,
};

void Spatializer::Init(float fixed_position) {
  angle_ = 0.0f;
  fixed_position_ = fixed_position;
  left_ = 0.0f;
  right_ = 0.0f;
  distance_ = 0.0f;
  behind_filter_.Init();
  behind_filter_.set_f_q<FREQUENCY_EXACT>(0.05f, 1.0f);
}
  
void Spatializer::Process(
    float* source,
    float* center,
    float* sides,
    size_t size) {
  behind_filter_.Process<FILTER_MODE_LOW_PASS>(source, behind_, size, 1);
 
  float angle = angle_;
  float x = distance_ * stmlib::InterpolateWrap(
      lut_sine, angle, 4096.0f);
  float y = distance_ * stmlib::InterpolateWrap(
      lut_sine, angle + 0.25f, 4096.0f);
  float backfront = (1.0f + y) * 0.5f * distance_;
  x += fixed_position_ * (1.0f - distance_);

  float target_left = stmlib::InterpolateWrap(
      lut_sine, (1.0f + x) * 0.125f, 4096.0f);
  float target_right = stmlib::InterpolateWrap(
      lut_sine, (3.0f + x) * 0.125f, 4096.0f);

  // Prevent zipper noise during rendering.
  float step = 1.0f / static_cast<float>(size);
  float left_increment = (target_left - left_) * step;
  float right_increment = (target_right - right_) * step;

  for (size_t i = 0; i < size; ++i) {
    left_ += left_increment;
    right_ += right_increment;
    float y = source[i] + backfront * (behind_[i] - source[i]);
    float l = left_ * y;
    float r = right_ * y;
    center[i] += (l + r) * 0.5f;
    sides[i] += (l - r) * 0.5f / 0.7f;
  }
}


void FmOscillator::Process(
    float frequency,
    float ratio,
    float feedback_amount,
    float target_fm_amount,
    const float* external_fm,
    float* destination,
    size_t size) {
  ratio = Interpolate(lut_fm_frequency_quantizer, ratio, 128.0f);

  uint32_t inc_carrier = midi_to_increment(frequency);
  uint32_t inc_mod = midi_to_increment(frequency + ratio);
  
  uint32_t phase_carrier = phase_carrier_;
  uint32_t phase_mod = phase_mod_;

  // Linear interpolation on FM amount parameter.
  float step = 1.0f / static_cast<float>(size);
  float fm_amount = fm_amount_;
  float fm_amount_increment = (target_fm_amount - fm_amount) * step;
  float previous_sample = previous_sample_;
  
  // To prevent aliasing, reduce FM amount when frequency or feedback are
  // too high.
  float brightness = frequency + ratio * 0.75f - 60.0f + \
      feedback_amount * 24.0f;
  float amount_attenuation = brightness <= 0.0f
      ? 1.0f
      : 1.0f - brightness * brightness * 0.0015f;
  if (amount_attenuation < 0.0f) {
    amount_attenuation = 0.0f; 
  }
  
  for (size_t i = 0; i < size; ++i) {
    fm_amount += fm_amount_increment;
    phase_carrier += inc_carrier;
    phase_mod += inc_mod;
    float mod = SineFm(phase_mod, feedback_amount * previous_sample);
    destination[i] = previous_sample = SineFm(
        phase_carrier,
        amount_attenuation * (mod * fm_amount + external_fm[i]));
  }
  
  phase_carrier_ = phase_carrier;
  phase_mod_ = phase_mod;
  fm_amount_ = fm_amount;
  previous_sample_ = previous_sample;
}


void OminousVoice::Init() {
  envelope_.Init();
  envelope_.set_adsr(0.5f, 0.5f, 0.5f, 0.5f);
  previous_gate_ = false;
  level_state_ = 0.0f;
  
  for (size_t i = 0; i < kNumOscillators; ++i) {
    external_fm_state_[i] = 0.0f;
    oscillator_[i].Init();

    // Downsampling is done mostly by the FIR, but since the stopband
    // attenuation peaks at -48dB, we can get a few extra dB of attenution with
    // the IIR for the highest frequencies.
    fir_downsampler_[i].Init(kDownsamplingFilter);
    iir_downsampler_[i].Init();
    iir_downsampler_[i].set_f_q<FREQUENCY_EXACT>(
        1.0f / kOversamplingUp * 0.8f,
        0.5f);
    
    osc_level_[i] = 0.0f;
    filter_[i].Init();
    
    spatializer_[i].Init(i == 0 ? - 0.7f : 0.7f);
  }
}

void OminousVoice::ConfigureEnvelope(const Patch& patch) {
  if (patch.exciter_envelope_shape < 0.4f) {
    float a = 0.0f;
    float dr = (patch.exciter_envelope_shape * 0.625f + 0.2f) * 1.8f;
    envelope_.set_adsr(a, dr, 0.0f, dr);
  } else if (patch.exciter_envelope_shape < 0.6f) {
    float s = (patch.exciter_envelope_shape - 0.4f) * 5.0f;
    envelope_.set_adsr(0.0f, 0.80f, s, 0.80f);
  } else {
    float a = 0.0f;
    float dr = ((1.0f - patch.exciter_envelope_shape) * 0.75f + 0.15f) * 1.8f;
    envelope_.set_adsr(a, dr, 1.0f, dr);
  }
}

void OminousVoice::Process(
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
  ConfigureEnvelope(patch);
  float level = envelope_.Process(flags);
  level += strength >= 0.5f ? 2.0f * strength - 1.0f : 0.0f;
  float level_increment = (level - level_state_) / size;

  damping_ += 0.1f * (patch.resonator_damping - damping_);
  float filter_env_amount = damping_ <= 0.9f ? 1.1f * damping_ : 0.99f;
  float vca_env_amount = 1.0f + \
      damping_ * damping_ * damping_ * damping_ * 0.5f;
  
  // Comfigure the filter.
  float cutoff_midi = 12.0f;
  cutoff_midi += patch.resonator_brightness * 140.0f;
  cutoff_midi += filter_env_amount * level * 120.0f;
  cutoff_midi += 0.5f * (frequency - 64.0f);
  
  float cutoff = midi_to_frequency(cutoff_midi);
  float q_bump = patch.resonator_geometry - 0.6f;
  float q = 1.72f - q_bump * q_bump * 2.0f;
  float cutoff_2 = cutoff * (1.0f + patch.resonator_modulation_offset);

  filter_[0].set_f_q<FREQUENCY_FAST>(cutoff, q);
  filter_[1].set_f_q<FREQUENCY_FAST>(cutoff_2, q * 1.25f);
  
  // Process each oscillator.
  fill(&center[0], &center[size], 0.0f);
  fill(&sides[0], &sides[size], 0.0f);
  fill(&raw[0], &raw[size], 0.0f);
  
  const float rotation_speed[2] = { 1.0f, 1.123456f };
  feedback_ += 0.01f * (patch.exciter_bow_timbre - feedback_);
  frequency += kOversamplingDownMidi;
  for (size_t i = 0; i < 2; ++i) {
    Upsample<kOversamplingUp>(
        &external_fm_state_[i],
        i == 0 ? blow_in : strike_in,
        external_fm_oversampled_, size);
    float detune, ratio, amount, level;
    if (i == 0) {
      detune = 0.0f;
      ratio = patch.exciter_blow_meta;
      amount = patch.exciter_blow_timbre;
      level = patch.exciter_blow_level;
    } else {
      detune = Interpolate(
          lut_detune_quantizer, patch.exciter_bow_level, 64.0f);
      ratio = patch.exciter_strike_meta;
      amount = patch.exciter_strike_timbre;
      level = patch.exciter_strike_level;
    }
    
    oscillator_[i].Process(
        frequency + detune,
        ratio,
        feedback_ * (0.25f + 0.15f * patch.exciter_signature),
        (2.0f - patch.exciter_signature * feedback_) * amount,
        external_fm_oversampled_,
        osc_oversampled_,
        size * kOversamplingUp);
    
    iir_downsampler_[i].Process<FILTER_MODE_LOW_PASS>(
        osc_oversampled_,
        osc_oversampled_,
        size * kOversamplingUp,
        1);
    fir_downsampler_[i].Process(osc_oversampled_, osc_, size * kOversamplingUp);
    
    // Copy to raw buffer.
    float level_state = osc_level_[i];
    for (size_t j = 0; j < size; ++j) {
      level_state += 0.01f * (level - level_state);
      osc_[j] *= level_state;
      raw[j] += osc_[j];
    }
    osc_level_[i] = level_state;

    // Apply filter.
    filter_[i].ProcessMultimode(osc_, osc_, size, patch.resonator_geometry);

    // Apply VCA.
    float l = level_state_;
    for (size_t j = 0; j < size; ++j) {
      float gain = l * vca_env_amount;
      if (gain >= 1.0f) gain = 1.0f;
      osc_[j] *= gain;
      l += level_increment;
    }

    // Spatialize.
    float f = patch.resonator_position * patch.resonator_position * 0.001f;
    float distance = patch.resonator_position;
    
    spatializer_[i].Rotate(f * rotation_speed[i]);
    spatializer_[i].set_distance(distance * (2.0f - distance));
    spatializer_[i].Process(osc_, center, sides, size);
  }
  
  level_state_ = level;
}

}  // namespace elements
