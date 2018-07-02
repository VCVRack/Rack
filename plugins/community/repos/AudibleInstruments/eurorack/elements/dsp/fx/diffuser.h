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
// Granular diffuser.

#ifndef ELEMENTS_DSP_FX_DIFFUSER_H_
#define ELEMENTS_DSP_FX_DIFFUSER_H_

#include "stmlib/stmlib.h"

#include "elements/dsp/fx/fx_engine.h"

namespace elements {

class Diffuser {
 public:
  Diffuser() { }
  ~Diffuser() { }
  
  void Init(float* buffer) {
    engine_.Init(buffer);
  }
  
  void Process(float* in_out, size_t size) {
    typedef E::Reserve<126,
      E::Reserve<180,
      E::Reserve<269,
      E::Reserve<444> > > > Memory;
    E::DelayLine<Memory, 0> ap1;
    E::DelayLine<Memory, 1> ap2;
    E::DelayLine<Memory, 2> ap3;
    E::DelayLine<Memory, 3> ap4;
    E::Context c;
    const float kap = 0.625f;
    while (size--) {
      engine_.Start(&c);
      c.Read(*in_out);
      c.Read(ap1 TAIL, kap);
      c.WriteAllPass(ap1, -kap);
      c.Read(ap2 TAIL, kap);
      c.WriteAllPass(ap2, -kap);
      c.Read(ap3 TAIL, kap);
      c.WriteAllPass(ap3, -kap);
      c.Read(ap4 TAIL, kap);
      c.WriteAllPass(ap4, -kap);
      c.Write(*in_out, 0.0f);
      ++in_out;
    }
  }
  
 private:
  typedef FxEngine<1024, FORMAT_32_BIT> E;
  E engine_;
  
  DISALLOW_COPY_AND_ASSIGN(Diffuser);
};

}  // namespace elements

#endif  // ELEMENTS_DSP_FX_DIFFUSER_H_
