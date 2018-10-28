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

#ifndef MARBLES_RANDOM_OUTPUT_CHANNEL_H_
#define MARBLES_RANDOM_OUTPUT_CHANNEL_H_

#include "stmlib/stmlib.h"

#include "marbles/random/lag_processor.h"
#include "marbles/random/quantizer.h"

namespace marbles {

class RandomSequence;

struct ScaleOffset {
  ScaleOffset(float s, float o) {
    scale = s;
    offset = o;
  }
  
  ScaleOffset() { scale = 1.0f; offset = 0.0f; }
  
  float scale;
  float offset;
  inline float operator()(float x) { return x * scale + offset; }
};

class OutputChannel {
 public:
  OutputChannel() { }
  ~OutputChannel() { }
  
  void Init();
  
  void LoadScale(int i, const Scale& scale) {
    quantizer_[i].Init(scale);
  }
  
  void Process(
      RandomSequence* random_sequence,
      const float* phase,
      float* output,
      size_t size,
      size_t stride);

  inline void set_spread(float spread) {
    spread_ = spread;
  }
  
  inline void set_bias(float bias) {
    bias_ = bias;
  }
  
  inline void set_scale_index(int i) {
    scale_index_ = i;
  }
  
  inline void set_steps(float steps) {
    steps_ = steps;
  }
  
  inline void set_register_mode(bool register_mode) {
    register_mode_ = register_mode;
  }

  inline void set_register_value(float register_value) {
    register_value_ = register_value;
  }
  
  inline void set_register_transposition(float register_transposition) {
    register_transposition_ = register_transposition;
  }
  
  inline void set_scale_offset(const ScaleOffset& scale_offset) {
    scale_offset_ = scale_offset;
  }
  
  inline float Quantize(float voltage, float amount) {
    return quantizer_[scale_index_].Process(voltage, amount, false);
  }
  
 private:
  float GenerateNewVoltage(RandomSequence* random_sequence);
  
  float spread_;
  float bias_;
  float steps_;
  int scale_index_;
  
  bool register_mode_;
  float register_value_;
  float register_transposition_;
  
  float previous_steps_;
  float previous_phase_;
  uint32_t reacquisition_counter_;
  
  float previous_voltage_;
  float voltage_;
  float quantized_voltage_;
  
  ScaleOffset scale_offset_;
  
  LagProcessor lag_processor_;
  
  Quantizer quantizer_[6];
  
  DISALLOW_COPY_AND_ASSIGN(OutputChannel);
};

}  // namespace marbles

#endif  // MARBLES_RANDOM_OUTPUT_CHANNEL_H_
