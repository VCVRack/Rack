// Copyright 2015 Olivier Gillet.
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
// Random generation channel.

#include "marbles/random/output_channel.h"

#include "marbles/random/distributions.h"
#include "marbles/random/random_sequence.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/parameter_interpolator.h"
#include "stmlib/utils/random.h"

namespace marbles {

using namespace stmlib;

const size_t kNumReacquisitions = 20; // 6.4 samples per millisecond

void OutputChannel::Init() {
  spread_ = 0.5f;
  bias_ = 0.5f;
  steps_ = 0.5f;
  scale_index_ = 0;

  register_mode_ = false;
  register_value_ = 0.0f;
  register_transposition_ = 0.0f;

  previous_steps_ = 0.0f;
  previous_phase_ = 0.0f;
  reacquisition_counter_ = 0;
  
  previous_voltage_ = 0.0f;
  voltage_ = 0.0f;
  quantized_voltage_ = 0.0f;
  
  scale_offset_ = ScaleOffset(10.0f, -5.0f);
  
  lag_processor_.Init();
  
  Scale scale;
  scale.Init();
  for (int i = 0; i < 6; ++i) {
    quantizer_[i].Init(scale);
  }
}

float OutputChannel::GenerateNewVoltage(RandomSequence* random_sequence) {
  float u = random_sequence->NextValue(register_mode_, register_value_);
  
  if (register_mode_) {
    return 10.0f * (u - 0.5f) + register_transposition_;
  } else {
    float degenerate_amount = 1.25f - spread_ * 25.0f;
    float bernoulli_amount = spread_ * 25.0f - 23.75f;
  
    CONSTRAIN(degenerate_amount, 0.0f, 1.0f);
    CONSTRAIN(bernoulli_amount, 0.0f, 1.0f);

    float value = BetaDistributionSample(u, spread_, bias_);
    float bernoulli_value = u >= (1.0f - bias_) ? 0.999999f : 0.0f;
    
    value += degenerate_amount * (bias_ - value);
    value += bernoulli_amount * (bernoulli_value - value);
    return scale_offset_(value);
  }
}

void OutputChannel::Process(
    RandomSequence* random_sequence,
    const float* phase,
    float* output,
    size_t size,
    size_t stride) {
  
  ParameterInterpolator steps_modulation(
      &previous_steps_, steps_, size);
  
  // This is a horrible hack that wouldn't be here if all the sequencers
  // and MIDI/CV interfaces in this world didn't have *horrible* slew on
  // their CV output (I'm looking at you KORG).
  // Without this hack, the shift register gets its value as soon as the
  // rising edge is observed on the GATE input. Problem: the CV input is
  // probably still slewing up, so we acquire the wrong value in the shift
  // register. What to do then? Over the next 2ms, we'll just track the CV
  // input until it reaches its final value - which means that Marbles
  // output will be slewed too. Another option would have been to wait 2ms
  // between the rising edge and the actual acquisition, but we don't want
  // to penalize people who use tighter sequencers.
  if (reacquisition_counter_) {
    --reacquisition_counter_;
    float u = random_sequence->RewriteValue(register_value_);
    voltage_ = 10.0f * (u - 0.5f) + register_transposition_;
    quantized_voltage_ = Quantize(voltage_, 2.0f * steps_ - 1.0f);
  }
  
  while (size--) {
    const float steps = steps_modulation.Next();
    if (*phase < previous_phase_) {
      previous_voltage_ = voltage_;
      voltage_ = GenerateNewVoltage(random_sequence);
      lag_processor_.ResetRamp();
      quantized_voltage_ = Quantize(voltage_, 2.0f * steps - 1.0f);
      if (register_mode_) {
        reacquisition_counter_ = kNumReacquisitions;
      }
    }
    
    if (steps >= 0.5f) {
      *output = quantized_voltage_;
    } else {
      const float smoothness = 1.0f - 2.0f * steps;
      *output = lag_processor_.Process(voltage_, smoothness, *phase);
    }
    output += stride;
    previous_phase_ = *phase++;
  }
}

}  // namespace marbles