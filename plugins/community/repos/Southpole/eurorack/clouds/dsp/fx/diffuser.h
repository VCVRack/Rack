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
// AP diffusion network.

#ifndef CLOUDS_DSP_FX_DIFFUSER_H_
#define CLOUDS_DSP_FX_DIFFUSER_H_

#include "stmlib/stmlib.h"

#include "clouds/dsp/fx/fx_engine.h"

namespace clouds {

class Diffuser {
 public:
  Diffuser() { }
  ~Diffuser() { }
  
  void Init(float* buffer) {
    engine_.Init(buffer);
  }
  
  void Process(FloatFrame* in_out, size_t size) {
    typedef E::Reserve<126,
      E::Reserve<180,
      E::Reserve<269,
      E::Reserve<444,
      E::Reserve<151,
      E::Reserve<205,
      E::Reserve<245,
      E::Reserve<405> > > > > > > > Memory;
    E::DelayLine<Memory, 0> apl1;
    E::DelayLine<Memory, 1> apl2;
    E::DelayLine<Memory, 2> apl3;
    E::DelayLine<Memory, 3> apl4;
    E::DelayLine<Memory, 4> apr1;
    E::DelayLine<Memory, 5> apr2;
    E::DelayLine<Memory, 6> apr3;
    E::DelayLine<Memory, 7> apr4;
    E::Context c;
    const float kap = 0.625f;
    while (size--) {
      engine_.Start(&c);
      
      float wet = 0.0f;
      c.Read(in_out->l);
      c.Read(apl1 TAIL, kap);
      c.WriteAllPass(apl1, -kap);
      c.Read(apl2 TAIL, kap);
      c.WriteAllPass(apl2, -kap);
      c.Read(apl3 TAIL, kap);
      c.WriteAllPass(apl3, -kap);
      c.Read(apl4 TAIL, kap);
      c.WriteAllPass(apl4, -kap);
      c.Write(wet, 0.0f);
      in_out->l += amount_ * (wet - in_out->l);
      
      c.Read(in_out->r);
      c.Read(apr1 TAIL, kap);
      c.WriteAllPass(apr1, -kap);
      c.Read(apr2 TAIL, kap);
      c.WriteAllPass(apr2, -kap);
      c.Read(apr3 TAIL, kap);
      c.WriteAllPass(apr3, -kap);
      c.Read(apr4 TAIL, kap);
      c.WriteAllPass(apr4, -kap);
      c.Write(wet, 0.0f);
      in_out->r += amount_ * (wet - in_out->r);

      ++in_out;
    }
  }
  
  void set_amount(float amount) {
    amount_ = amount;
  }
  
 private:
  typedef FxEngine<2048, FORMAT_32_BIT> E;
  E engine_;
  
  float amount_;
  DISALLOW_COPY_AND_ASSIGN(Diffuser);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_FX_DIFFUSER_H_
