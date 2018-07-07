// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (olivier@mutable-instruments.net)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Sampled oscillator.

#ifndef EDGES_DIGITAL_OSCILLATOR_H_
#define EDGES_DIGITAL_OSCILLATOR_H_

#include "avrlibx/avrlibx.h"

namespace edges {
  
using namespace avrlibx;

enum OscillatorShape {
  OSC_TRIANGLE,
  OSC_NES_TRIANGLE,
  OSC_PITCHED_NOISE,
  OSC_NES_NOISE_LONG,
  OSC_NES_NOISE_SHORT,
  OSC_SINE,
};

class DigitalOscillator {
 public:
  typedef void (DigitalOscillator::*RenderFn)();

  DigitalOscillator() { }
  ~DigitalOscillator() { }
  
  void Init() {
    UpdatePitch(60 << 7, OSC_TRIANGLE);
    Gate(true);
    rng_state_ = 1;
  }
  
  void UpdatePitch(int16_t pitch, OscillatorShape shape) {
    pitch_ = pitch;
    shape_ = shape;
  }
  
  void Gate(bool gate) {
    gate_ = gate;
  }
  
  void Render();
  
  inline void set_cv_pw(uint8_t pw) {
    cv_pw_ = pw;
  }
  
 private:
  void ComputePhaseIncrement();
   
  void RenderSilence();
  void RenderSine();
  void RenderBandlimitedTriangle();
  void RenderNoiseNES();
  void RenderNoise();
  
  OscillatorShape shape_;
  int16_t pitch_;
  bool gate_;

  uint8_t note_;
  uint24_t phase_;
  uint24_t phase_increment_;
  uint16_t sample_;
  uint16_t rng_state_;
  uint16_t aux_phase_;
  uint8_t cv_pw_;
  
  static const RenderFn fn_table_[];
  
  DISALLOW_COPY_AND_ASSIGN(DigitalOscillator);
};

}  // namespace edges


#endif  // EDGES_DIGITAL_OSCILLATOR_H_
