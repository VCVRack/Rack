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
// Generic class interfacing all dynamics processors.

#ifndef STREAMS_PROCESSOR_H_
#define STREAMS_PROCESSOR_H_

#include <cstdio>

#include "stmlib/stmlib.h"

#include "streams/compressor.h"
#include "streams/envelope.h"
#include "streams/filter_controller.h"
#include "streams/follower.h"
#include "streams/lorenz_generator.h"
#include "streams/vactrol.h"

namespace streams {

enum ProcessorFunction {
  PROCESSOR_FUNCTION_ENVELOPE,
  PROCESSOR_FUNCTION_VACTROL,
  PROCESSOR_FUNCTION_FOLLOWER,
  PROCESSOR_FUNCTION_COMPRESSOR,
  PROCESSOR_FUNCTION_FILTER_CONTROLLER,
  PROCESSOR_FUNCTION_LORENZ_GENERATOR,
  PROCESSOR_FUNCTION_LAST
};

#define DECLARE_PROCESSOR(ClassName, variable) \
  void ClassName ## Init() { \
    variable.Init(); \
  } \
  void ClassName ## Process(int16_t a, int16_t e, uint16_t* g, uint16_t* f) { \
    variable.Process(a, e, g, f); \
  } \
  void ClassName ## Configure(bool a, int32_t* p, int32_t* g) { \
    variable.Configure(a, p, g); \
  } \
  ClassName variable;


class Processor {
 public:
  Processor() { }
  ~Processor() { }
  
  void Init(uint8_t index);
  void Init() { Init(0); }
  
  typedef void (Processor::*InitFn)(); 
  typedef void (Processor::*ProcessFn)(
      int16_t,
      int16_t,
      uint16_t*,
      uint16_t*); 
  typedef void (Processor::*ConfigureFn)(
      bool,
      int32_t*,
      int32_t*);
  
  struct ProcessorCallbacks {
    InitFn init;
    ProcessFn process;
    ConfigureFn configure;
  };
  
  inline void set_function(ProcessorFunction function) {
    function_ = function;
    callbacks_ = callbacks_table_[function];
    (this->*callbacks_.init)();
    dirty_ = true;
  }
  
  inline void set_alternate(bool alternate) {
    alternate_ = alternate;
    dirty_ = true;
  }

  inline void set_linked(bool linked) {
    linked_ = linked;
    dirty_ = true;
  }
  
  inline void set_parameter(uint16_t index, uint16_t value) {
    parameters_[index] = value;
    dirty_ = true;
  }
  inline void set_global(uint16_t index, uint16_t value) {
    globals_[index] = value;
    dirty_ = linked_;
  }
  
  inline ProcessorFunction function() const { return function_; }
  inline bool alternate() const { return alternate_; }
  inline bool linked() const { return linked_; }
  inline uint8_t last_frequency() const { return last_frequency_value_ >> 8; }
  inline uint8_t last_gain() const { return last_gain_value_ >> 8; }
  inline int32_t gain_reduction() const { return compressor_.gain_reduction(); }
  
  inline void Process(
      int16_t audio,
      int16_t excite,
      uint16_t* gain,
      uint16_t* frequency) {
    (this->*callbacks_.process)(audio, excite, gain, frequency);
    last_gain_value_ = *gain;
    last_frequency_value_ = *frequency;
  }

  void Configure() {
    if (!dirty_) {
      return;
    }
    (this->*callbacks_.configure)(
        alternate_,
        parameters_,
        linked_ ? globals_ : NULL);
    dirty_ = false;
  }

 private:
  uint8_t index_;
  ProcessorFunction function_;
  bool linked_;
  bool alternate_;
  bool dirty_;
  int32_t parameters_[2];
  int32_t globals_[4];
  
  uint16_t last_gain_value_;
  uint16_t last_frequency_value_;
  
  ProcessorCallbacks callbacks_;
  static const ProcessorCallbacks callbacks_table_[PROCESSOR_FUNCTION_LAST];
  
  DECLARE_PROCESSOR(Envelope, envelope_);
  DECLARE_PROCESSOR(Vactrol, vactrol_);
  DECLARE_PROCESSOR(Follower, follower_);
  DECLARE_PROCESSOR(Compressor, compressor_);
  DECLARE_PROCESSOR(FilterController, filter_controller_);
  DECLARE_PROCESSOR(LorenzGenerator, lorenz_generator_);

  DISALLOW_COPY_AND_ASSIGN(Processor);
};

}  // namespace streams

#endif  // STREAMS_PROCESSOR_H_
