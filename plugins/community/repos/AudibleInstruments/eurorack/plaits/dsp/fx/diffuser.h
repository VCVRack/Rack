// Copyright 2016 Olivier Gillet.
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

#ifndef PLAITS_DSP_FX_DIFFUSER_H_
#define PLAITS_DSP_FX_DIFFUSER_H_

#include "stmlib/stmlib.h"

#include "plaits/dsp/fx/fx_engine.h"

namespace plaits {

class Diffuser {
 public:
  Diffuser() { }
  ~Diffuser() { }
  
  void Init(uint16_t* buffer) {
    engine_.Init(buffer);
    engine_.SetLFOFrequency(LFO_1, 0.3f / 48000.0f);
    lp_decay_ = 0.0f;
  }
  
  void Clear() {
    engine_.Clear();
  }
  
  void Process(float amount, float rt, float* in_out, size_t size) {
    typedef E::Reserve<126,
      E::Reserve<180,
      E::Reserve<269,
      E::Reserve<444,
      E::Reserve<1653,
      E::Reserve<2010,
      E::Reserve<3411> > > > > > > Memory;
    E::DelayLine<Memory, 0> ap1;
    E::DelayLine<Memory, 1> ap2;
    E::DelayLine<Memory, 2> ap3;
    E::DelayLine<Memory, 3> ap4;
    E::DelayLine<Memory, 4> dapa;
    E::DelayLine<Memory, 5> dapb;
    E::DelayLine<Memory, 6> del;
    E::Context c;
    const float kap = 0.625f;
    const float klp = 0.75f;
    float lp = lp_decay_;
    while (size--) {
      float wet;
      engine_.Start(&c);
      c.Read(*in_out);
      c.Read(ap1 TAIL, kap);
      c.WriteAllPass(ap1, -kap);
      c.Read(ap2 TAIL, kap);
      c.WriteAllPass(ap2, -kap);
      c.Read(ap3 TAIL, kap);
      c.WriteAllPass(ap3, -kap);
      c.Interpolate(ap4, 400.0f, LFO_1, 43.0f, kap);
      c.WriteAllPass(ap4, -kap);
      c.Interpolate(del, 3070.0f, LFO_1, 340.0f, rt);
      c.Lp(lp, klp);
      c.Read(dapa TAIL, -kap);
      c.WriteAllPass(dapa, kap);
      c.Read(dapb TAIL, kap);
      c.WriteAllPass(dapb, -kap);
      c.Write(del, 2.0f);
      c.Write(wet, 0.0f);
      *in_out += amount * (wet - *in_out);
      ++in_out;
    }
    lp_decay_ = lp;
  }
  
 private:
  typedef FxEngine<8192, FORMAT_12_BIT> E;
  E engine_;
  float lp_decay_;
  
  DISALLOW_COPY_AND_ASSIGN(Diffuser);
};

}  // namespace plaits

#endif  // PLAITS_DSP_FX_DIFFUSER_H_
