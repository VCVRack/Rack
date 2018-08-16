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
// Modulator.

#include "warps/dsp/modulator.h"

#include <algorithm>

#include "stmlib/dsp/units.h"

#include "warps/drivers/debug_pin.h"
#include "warps/resources.h"

namespace warps {

using namespace std;
using namespace stmlib;

void Modulator::Init(float sample_rate) {
  bypass_ = false;
  easter_egg_ = false;
  
  for (int32_t i = 0; i < 2; ++i) {
    amplifier_[i].Init();
    src_up_[i].Init();
    quadrature_transform_[i].Init(lut_ap_poles, LUT_AP_POLES_SIZE);
  }
  src_down_.Init();
  
  xmod_oscillator_.Init(sample_rate);
  vocoder_oscillator_.Init(sample_rate);
  quadrature_oscillator_.Init(sample_rate);
  vocoder_.Init(sample_rate);
  
  previous_parameters_.carrier_shape = 0;
  previous_parameters_.channel_drive[0] = 0.0f;
  previous_parameters_.channel_drive[1] = 0.0f;
  previous_parameters_.modulation_algorithm = 0.0f;
  previous_parameters_.modulation_parameter = 0.0f;
  previous_parameters_.note = 48.0f;

  feedback_sample_ = 0.0f;
}

void Modulator::ProcessEasterEgg(
    ShortFrame* input,
    ShortFrame* output,
    size_t size) {
  float* carrier = buffer_[0];
  float* carrier_i = &src_buffer_[0][0];
  float* carrier_q = &src_buffer_[0][size];
  
  // Generate the I/Q components.
  if (parameters_.carrier_shape) {
    float d = parameters_.frequency_shift_pot - 0.5f;
    float linear_modulation_amount = 1.0f - 14.0f * d * d;
    if (linear_modulation_amount < 0.0f) {
      linear_modulation_amount = 0.0f;
    }
    float frequency = parameters_.frequency_shift_pot;
    frequency += linear_modulation_amount * parameters_.frequency_shift_cv;
    
    float direction = frequency >= 0.5f ? 1.0f : -1.0f;
    frequency = 2.0f * fabs(frequency - 0.5f);
    frequency = frequency <= 0.4f
        ? frequency * frequency * frequency * 62.5f
        : 4.0f * SemitonesToRatio(180.0f * (frequency - 0.4f));
    frequency *= SemitonesToRatio(
        parameters_.frequency_shift_cv * 60.0f * \
            (1.0f - linear_modulation_amount) * direction);
    frequency *= direction;
    
    float shape = static_cast<float>(parameters_.carrier_shape - 1) * 0.5f;
    quadrature_oscillator_.Render(shape, frequency, carrier_i, carrier_q, size);
  } else {
    for (size_t i = 0; i < size; ++i) {
      carrier[i] = static_cast<float>(input[i].l) / 32768.0f;
    }
    quadrature_transform_[0].Process(carrier, carrier_i, carrier_q, size);
    
    ParameterInterpolator phase_shift(
        &previous_parameters_.phase_shift,
        parameters_.phase_shift,
        size);
  
    for (size_t i = 0; i < size; ++i) {
      float x_i = carrier_i[i];
      float x_q = carrier_q[i];
      float angle = phase_shift.Next();
      float r_sin = Interpolate(lut_sin, angle, 1024.0f);
      float r_cos = Interpolate(lut_sin + 256, angle, 1024.0f);
      carrier_i[i] = r_sin * x_i + r_cos * x_q;
      carrier_q[i] = r_sin * x_q - r_cos * x_i;
    }
  }

  // Setup parameter interpolation.
  ParameterInterpolator mix(
      &previous_parameters_.modulation_parameter,
      parameters_.modulation_parameter,
      size);
  ParameterInterpolator feedback_amount(
      &previous_parameters_.channel_drive[0],
      parameters_.channel_drive[0],
      size);
  ParameterInterpolator dry_wet(
      &previous_parameters_.channel_drive[1],
      parameters_.channel_drive[1],
      size);
  
  float feedback_sample = feedback_sample_;
  for (size_t i = 0; i < size; ++i) {
    float timbre = mix.Next();
    float modulator_i, modulator_q;

    // Start from the signal from input 2, with non-linear gain.
    float in = static_cast<float>(input->r) / 32768.0f;
    
    if (parameters_.carrier_shape) {
      in += static_cast<float>(input->l) / 32768.0f;
    }
    
    float modulator = in;
    
    // Apply feedback if necessary, and soft limit.
    float amount = feedback_amount.Next();
    amount *= (2.0f - amount);
    amount *= (2.0f - amount);
    
    // mic.w feedback amount tweak.
    float max_fb = 1.0f + 2.0f * (timbre - 0.5f) * (timbre - 0.5f);
    modulator += amount * (
        SoftClip(modulator + max_fb * feedback_sample * amount) - modulator);

    quadrature_transform_[1].Process(modulator, &modulator_i, &modulator_q);

    // Modulate!
    float a = *carrier_i++ * modulator_i;
    float b = *carrier_q++ * modulator_q;
    float up = a - b;
    float down = a + b;
    float lut_index = timbre;
    float fade_in = Interpolate(lut_xfade_in, lut_index, 256.0f);
    float fade_out = Interpolate(lut_xfade_out, lut_index, 256.0f);
    float main = up * fade_in + down * fade_out;
    float aux = down * fade_in + up * fade_out;
    
    // Simple LP to prevent feedback of high-frequencies.
    ONE_POLE(feedback_sample, main, 0.2f);

    float wet_dry = 1.0f - dry_wet.Next();
    main += wet_dry * (in - main);
    aux += wet_dry * (in - aux);
    
    output->l = Clip16(static_cast<int32_t>(main * 32768.0f));
    output->r = Clip16(static_cast<int32_t>(aux * 32768.0f));
    ++output;
    ++input;
  }
  feedback_sample_ = feedback_sample;
  previous_parameters_ = parameters_;
}

void Modulator::Process(ShortFrame* input, ShortFrame* output, size_t size) {
  if (bypass_) {
    copy(&input[0], &input[size], &output[0]);
    return;
  } else if (easter_egg_) {
    ProcessEasterEgg(input, output, size);
    return;
  }
  float* carrier = buffer_[0];
  float* modulator = buffer_[1];
  float* main_output = buffer_[0];
  float* aux_output = buffer_[2];
  float* oversampled_carrier = src_buffer_[0];
  float* oversampled_modulator = src_buffer_[1];
  float* oversampled_output = src_buffer_[0];
  
  // 0.0: use cross-modulation algorithms. 1.0f: use vocoder.
  float vocoder_amount = (
      parameters_.modulation_algorithm - 0.7f) * 20.0f + 0.5f;
  CONSTRAIN(vocoder_amount, 0.0f, 1.0f);
  
  if (!parameters_.carrier_shape) {
    fill(&aux_output[0], &aux_output[size], 0.0f);
  }
  
  // Convert audio inputs to float and apply VCA/saturation (5.8% per channel)
  short* input_samples = &input->l;
  for (int32_t i = parameters_.carrier_shape ? 1 : 0; i < 2; ++i) {
      amplifier_[i].Process(
          parameters_.channel_drive[i],
          1.0f - vocoder_amount,
          input_samples + i,
          buffer_[i],
          aux_output,
          2,
          size);
  }

  // If necessary, render carrier. Otherwise, sum signals 1 and 2 for aux out.
  if (parameters_.carrier_shape) {
    // Scale phase-modulation input.
    for (size_t i = 0; i < size; ++i) {
      internal_modulation_[i] = static_cast<float>(input[i].l) / 32768.0f;
    }
    // Xmod: sine, triangle saw.
    // Vocoder: saw, pulse, noise.
    OscillatorShape xmod_shape = static_cast<OscillatorShape>(
        parameters_.carrier_shape - 1);
    OscillatorShape vocoder_shape = static_cast<OscillatorShape>(
        parameters_.carrier_shape + 1);
    
    const float kXmodCarrierGain = 0.5f;

    // Outside of the transition zone between the cross-modulation and vocoding
    // algorithm, we need to render only one of the two oscillators.
    if (vocoder_amount == 0.0f) {
      xmod_oscillator_.Render(
          xmod_shape,
          parameters_.note,
          internal_modulation_,
          aux_output,
          size);
      for (size_t i = 0; i < size; ++i) {
        carrier[i] = aux_output[i] * kXmodCarrierGain;
      }
    } else if (vocoder_amount >= 0.5f) {
      float carrier_gain = vocoder_oscillator_.Render(
          vocoder_shape,
          parameters_.note,
          internal_modulation_,
          aux_output,
          size);
      for (size_t i = 0; i < size; ++i) {
        carrier[i] = aux_output[i] * carrier_gain;
      }
    } else {
      float balance = vocoder_amount * 2.0f;
      xmod_oscillator_.Render(
          xmod_shape,
          parameters_.note,
          internal_modulation_,
          carrier,
          size);
      float carrier_gain = vocoder_oscillator_.Render(
          vocoder_shape,
          parameters_.note,
          internal_modulation_,
          aux_output,
          size);
      for (size_t i = 0; i < size; ++i) {
        float a = carrier[i];
        float b = aux_output[i];
        aux_output[i] = a + (b - a) * balance;
        a *= kXmodCarrierGain;
        b *= carrier_gain;
        carrier[i] = a + (b - a) * balance;
      }
    }
  }
  
  if (vocoder_amount < 0.5f) {
    src_up_[0].Process(carrier, oversampled_carrier, size);
    src_up_[1].Process(modulator, oversampled_modulator, size);
    
    float algorithm = min(parameters_.modulation_algorithm * 8.0f, 5.999f);
    float previous_algorithm = min(
        previous_parameters_.modulation_algorithm * 8.0f, 5.999f);
    
    MAKE_INTEGRAL_FRACTIONAL(algorithm);
    MAKE_INTEGRAL_FRACTIONAL(previous_algorithm);
    
    if (algorithm_integral != previous_algorithm_integral) {
      previous_algorithm_fractional = algorithm_fractional;
    }

    (this->*xmod_table_[algorithm_integral])(
        previous_algorithm_fractional,
        algorithm_fractional,
        previous_parameters_.skewed_modulation_parameter(),
        parameters_.skewed_modulation_parameter(),
        oversampled_modulator,
        oversampled_carrier,
        oversampled_output,
        size * kOversampling);

    src_down_.Process(oversampled_output, main_output, size * kOversampling);
  } else {
    float release_time = 4.0f * (parameters_.modulation_algorithm - 0.75f);
    CONSTRAIN(release_time, 0.0f, 1.0f);
    
    vocoder_.set_release_time(release_time * (2.0f - release_time));
    vocoder_.set_formant_shift(parameters_.modulation_parameter);
    vocoder_.Process(modulator, carrier, main_output, size);
  }
  
  // Cross-fade to raw modulator for the transition between cross-modulation
  // algorithms and vocoding algorithms.
  float transition_gain = 2.0f * (vocoder_amount < 0.5f
      ? vocoder_amount
      : 1.0f - vocoder_amount);
  if (transition_gain != 0.0f) {
    for (size_t i = 0; i < size; ++i) {
      main_output[i] += transition_gain * (modulator[i] - main_output[i]);
    }
  }

  // Convert back to integer and clip.
  while (size--) {
    output->l = Clip16(static_cast<int32_t>(*main_output * 32768.0f));
    output->r = Clip16(static_cast<int32_t>(*aux_output * 16384.0f));
    ++main_output;
    ++aux_output;
    ++output;
  }
  previous_parameters_ = parameters_;
}


/* static */
inline float Modulator::Diode(float x) {
  // Approximation of diode non-linearity from:
  // Julian Parker - "A simple Digital model of the diode-based ring-modulator."
  // Proc. DAFx-11
  float sign = x > 0.0f ? 1.0f : -1.0f;
  float dead_zone = fabs(x) - 0.667f;
  dead_zone += fabs(dead_zone);
  dead_zone *= dead_zone;
  return 0.04324765822726063f * dead_zone * sign;
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_XFADE>(
    float x_1, float x_2, float parameter) {
  float fade_in = Interpolate(lut_xfade_in, parameter, 256.0f);
  float fade_out = Interpolate(lut_xfade_out, parameter, 256.0f);
  return x_1 * fade_in + x_2 * fade_out;
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_FOLD>(
    float x_1, float x_2, float parameter) {
  float sum = 0.0f;
  sum += x_1;
  sum += x_2;
  sum += x_1 * x_2 * 0.25f;
  sum *= 0.02f + parameter;
  const float kScale = 2048.0f / ((1.0f + 1.0f + 0.25f) * 1.02f);
  return Interpolate(lut_bipolar_fold + 2048, sum, kScale);
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_ANALOG_RING_MODULATION>(
    float modulator, float carrier, float parameter) {
  carrier *= 2.0f;
  float ring = Diode(modulator + carrier) + Diode(modulator - carrier);
  ring *= (4.0f + parameter * 24.0f);
  return SoftLimit(ring);
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_DIGITAL_RING_MODULATION>(
    float x_1, float x_2, float parameter) {
  float ring = 4.0f * x_1 * x_2 * (1.0f + parameter * 8.0f);
  return ring / (1.0f + fabs(ring));
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_XOR>(
    float x_1, float x_2, float parameter) {
  short x_1_short = Clip16(static_cast<int32_t>(x_1 * 32768.0f));
  short x_2_short = Clip16(static_cast<int32_t>(x_2 * 32768.0f));
  float mod = static_cast<float>(x_1_short ^ x_2_short) / 32768.0f;
  float sum = (x_1 + x_2) * 0.7f;
  return sum + (mod - sum) * parameter;
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_COMPARATOR>(
    float modulator, float carrier, float parameter) {
  float x = parameter * 2.995f;
  MAKE_INTEGRAL_FRACTIONAL(x)

  float direct = modulator < carrier ? modulator : carrier;
  float window = fabs(modulator) > fabs(carrier) ? modulator : carrier;
  float window_2 = fabs(modulator) > fabs(carrier)
      ? fabs(modulator)
      : -fabs(carrier);
  float threshold = carrier > 0.05f ? carrier : modulator;
  
  float sequence[4] = { direct, threshold, window, window_2 };
  float a = sequence[x_integral];
  float b = sequence[x_integral + 1];
  
  return a + (b - a) * x_fractional;
}

/* static */
template<>
inline float Modulator::Xmod<ALGORITHM_NOP>(
    float modulator, float carrier, float parameter) {
  return modulator;
}

/* static */
Modulator::XmodFn Modulator::xmod_table_[] = {
  &Modulator::ProcessXmod<ALGORITHM_XFADE, ALGORITHM_FOLD>,
  &Modulator::ProcessXmod<ALGORITHM_FOLD, ALGORITHM_ANALOG_RING_MODULATION>,
  &Modulator::ProcessXmod<
      ALGORITHM_ANALOG_RING_MODULATION, ALGORITHM_DIGITAL_RING_MODULATION>,
  &Modulator::ProcessXmod<ALGORITHM_DIGITAL_RING_MODULATION, ALGORITHM_XOR>,
  &Modulator::ProcessXmod<ALGORITHM_XOR, ALGORITHM_COMPARATOR>,
  &Modulator::ProcessXmod<ALGORITHM_COMPARATOR, ALGORITHM_NOP>,
};

}  // namespace warps
