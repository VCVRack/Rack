// Copyright 2012 Olivier Gillet.
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
// Macro-oscillator entry point.

#ifndef BRAIDS_MACRO_OSCILLATOR_H_
#define BRAIDS_MACRO_OSCILLATOR_H_

#include "stmlib/stmlib.h"

#include <cstring>

#include "braids/analog_oscillator.h"
#include "braids/digital_oscillator.h"
#include "braids/resources.h"
#include "braids/settings.h"

namespace braids {
  
class MacroOscillator {
 public:
  typedef void (MacroOscillator::*RenderFn)(const uint8_t*, int16_t*, size_t);

  MacroOscillator() { }
  ~MacroOscillator() { }
  
  inline void Init() {
    analog_oscillator_[0].Init();
    analog_oscillator_[1].Init();
    analog_oscillator_[2].Init();
    digital_oscillator_.Init();
    lp_state_ = 0;
    previous_parameter_[0] = 0;
    previous_parameter_[1] = 0;
  }
  
  inline void set_shape(MacroOscillatorShape shape) {
    if (shape != shape_) {
      Strike();
    }
    shape_ = shape;
  }

  inline void set_pitch(int16_t pitch) {
    pitch_ = pitch;
  }

  inline int16_t pitch() const { return pitch_; }

  inline void set_parameters(
      int16_t parameter_1,
      int16_t parameter_2) {
    parameter_[0] = parameter_1;
    parameter_[1] = parameter_2;
  }
  
  inline void Strike() {
    digital_oscillator_.Strike();
  }
  
  void Render(const uint8_t* sync_buffer, int16_t* buffer, size_t size);
  
 private:
  void RenderCSaw(const uint8_t*, int16_t*, size_t);
  void RenderMorph(const uint8_t*, int16_t*, size_t);
  void RenderSawSquare(const uint8_t*, int16_t*, size_t);
  void RenderDualSync(const uint8_t*, int16_t*, size_t);
  void RenderSineTriangle(const uint8_t*, int16_t*, size_t);
  void RenderBuzz(const uint8_t*, int16_t*, size_t);
  void RenderDigital(const uint8_t*, int16_t*, size_t);
  void RenderSawComb(const uint8_t*, int16_t*, size_t);
  void RenderTriple(const uint8_t*, int16_t*, size_t);
  void ConfigureTriple(AnalogOscillatorShape shape);

  int16_t parameter_[2];
  int16_t previous_parameter_[2];
  int16_t pitch_;
  uint8_t sync_buffer_[24];
  int16_t temp_buffer_[24];
  int32_t lp_state_;
  
  AnalogOscillator analog_oscillator_[3];
  DigitalOscillator digital_oscillator_;
  
  MacroOscillatorShape shape_;
  static RenderFn fn_table_[];
  
  DISALLOW_COPY_AND_ASSIGN(MacroOscillator);
};

}  // namespace braids

#endif // BRAIDS_MACRO_OSCILLATOR_H_
