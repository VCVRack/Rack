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
// LFO.

#ifndef PEAKS_MODULATIONS_LFO_H_
#define PEAKS_MODULATIONS_LFO_H_

#include "stmlib/stmlib.h"
#include "stmlib/algorithms/pattern_predictor.h"

#include "peaks/gate_processor.h"

namespace peaks {

enum LfoShape {
  LFO_SHAPE_SINE,
  LFO_SHAPE_TRIANGLE,
  LFO_SHAPE_SQUARE,
  LFO_SHAPE_STEPS,
  LFO_SHAPE_NOISE,
  LFO_SHAPE_LAST
};

class Lfo {
 public:
  typedef int16_t (Lfo::*ComputeSampleFn)();
   
  Lfo() { }
  ~Lfo() { }
  
  void Init();
  void FillBuffer(InputBuffer* input_buffer, OutputBuffer* output_buffer);
  
  void Configure(uint16_t* parameter, ControlMode control_mode) {
    if (control_mode == CONTROL_MODE_HALF) {
      if (sync_) {
        set_shape_integer(parameter[0]);
        set_parameter(parameter[1] - 32768);
      } else {
        set_rate(parameter[0]);
        set_shape_parameter_preset(parameter[1]);
      }
      set_reset_phase(0);
      set_level(65535);
    } else {
      if (sync_) {
        set_level(parameter[0]);
        set_shape_integer(parameter[1]);
        set_parameter(parameter[2] - 32768);
        set_reset_phase(parameter[3] - 32768);
      } else {
        set_level(65535);
        set_rate(parameter[0]);
        set_shape_integer(parameter[1]);
        set_parameter(parameter[2] - 32768);
        set_reset_phase(parameter[3] - 32768);
      }
    }
  }
  
  inline void set_rate(uint16_t rate) {
    rate_ = rate;
  }
  
  inline void set_shape(LfoShape shape) {
    shape_ = shape;
  }

  inline void set_shape_integer(uint16_t value) {
    shape_ = static_cast<LfoShape>(value * LFO_SHAPE_LAST >> 16);
  }
  
  void set_shape_parameter_preset(uint16_t value);
  
  inline void set_parameter(int16_t parameter) {
    parameter_ = parameter;
  }
  
  inline void set_reset_phase(int16_t reset_phase) {
    reset_phase_ = static_cast<int32_t>(reset_phase) << 16;
  }
  
  inline void set_sync(bool sync) {
    if (!sync_ && sync) {
      pattern_predictor_.Init();
    }
    sync_ = sync;
  }
  
  inline void set_level(uint16_t level) {
    level_ = level >> 1;
  }
  
 private:
  int16_t ComputeSampleSine();
  int16_t ComputeSampleTriangle();
  int16_t ComputeSampleSquare();
  int16_t ComputeSampleSteps();
  int16_t ComputeSampleNoise();
   
  uint16_t rate_;
  LfoShape shape_;
  int16_t parameter_;
  int32_t reset_phase_;
  int32_t level_;

  bool sync_;
  uint32_t sync_counter_;
  stmlib::PatternPredictor<32, 8> pattern_predictor_;
  
  uint32_t phase_;
  uint32_t phase_increment_;
  
  uint32_t period_;
  uint32_t end_of_attack_;
  uint32_t attack_factor_;
  uint32_t decay_factor_;
  int16_t previous_parameter_;
  
  int32_t value_;
  int32_t next_value_;
  
  static ComputeSampleFn compute_sample_fn_table_[];

  DISALLOW_COPY_AND_ASSIGN(Lfo);
};

}  // namespace peaks

#endif  // PEAKS_MODULATIONS_LFO_H_
