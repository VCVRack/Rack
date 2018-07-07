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
// Resonestor.

#ifndef CLOUDS_DSP_RESONESTOR_H_
#define CLOUDS_DSP_RESONESTOR_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/random.h"
#include "stmlib/dsp/units.h"
#include "clouds/dsp/fx/fx_engine.h"
#include "clouds/resources.h"

using namespace stmlib;

namespace clouds {

const float chords[3][18] =
  {
    { 0.0f, 4.0f/128.0f, 16.0f/128.0f, 4.0f/128.0f, 4.0f/128.0f,
      12.0f, 12.0f, 4.0f, 4.0f,
      3.0f, 3.0f, 2.0f, 4.0f,
      3.0f, 4.0f, 3.0f, 4.0f,
      4.0f },
    { 0.0f, 8.0f/128.0f, 32.0f/128.0f, 7.0f, 12.0f,
      24.0f, 7.0f, 7.0f, 7.0f,
      7.0f, 7.0f, 7.0f, 7.0f,
      7.0f, 7.0f, 7.0f, 7.0f,
      7.0f },
    { 0.0f, 12.0f/128.0f, 48.0f/128.0f, 7.0f + 4.0f/128.0f, 12.0f + 4.0f/128.0f,
      36.0f, 19.0f, 12.0f, 11.0f,
      10.0f, 12.0f, 12.0f, 12.0f,
      14.0f, 14.0f, 16.0f, 16.0f,
      16.0f }
  };

#define PLATEAU 2.0f

inline float InterpolatePlateau(const float* table, float index, float size) {
  index *= size;
  MAKE_INTEGRAL_FRACTIONAL(index)
  float a = table[index_integral];
  float b = table[index_integral + 1];
  if (index_fractional < 1.0f/PLATEAU)
    return a + (b - a) * index_fractional * PLATEAU;
  else
    return b;
}

class Resonestor {
 public:
  Resonestor() { }
  ~Resonestor() { }

  void Init(float* buffer) {
    engine_.Init(buffer);
    for (int v=0; v<2; v++) {
      pitch_[v] = 0.0f;
      chord_[v] = 0.0f;
      feedback_[v] = 0.0f;
      narrow_[v] = 0.001f;
      damp_[v] = 1.0f;
      harmonicity_[v] = 1.0f;
      distortion_[v] = 0.0f;
    }
    spread_amount_ = 0.0f;
    stereo_ = 0.0f;
    burst_time_ = 0.0f;
    burst_damp_ = 1.0f;
    burst_comb_ = 1.0f;
    burst_duration_ = 0.0f;
    trigger_ = previous_trigger_ = 0.0f;
    freeze_ = previous_freeze_ = 0.0f;
    voice_ = false;
    for (int i=0; i<3; i++)
      spread_delay_[i] = Random::GetFloat() * 3999;
    burst_lp_.Init();
    rand_lp_.Init();
    rand_hp_.Init();
    rand_hp_.set_f<FREQUENCY_FAST>(1.0f / 32000.0f);
    for (int v=0; v<2; v++)
      for (int p=0; p<4; p++) {
        lp_[p][v].Init();
        bp_[p][v].Init();
        hp_[p][v] = 0.0f;
        comb_period_[p][v] = 0.0f;
        comb_feedback_[p][v] = 0.0f;
      }
  }

#define MAX_COMB 1000
#define BASE_PITCH 261.626f

  void Process(FloatFrame* in_out, size_t size) {

    typedef E::Reserve<MAX_COMB,
      E::Reserve<MAX_COMB,
      E::Reserve<MAX_COMB,
      E::Reserve<MAX_COMB,
      E::Reserve<200,          /* bc */
      E::Reserve<4000,          /* bd0 */
      E::Reserve<4000,          /* bd1 */
      E::Reserve<MAX_COMB,
      E::Reserve<MAX_COMB,
      E::Reserve<MAX_COMB,
      E::Reserve<MAX_COMB > > > > > > > > > > > Memory;
    E::DelayLine<Memory, 0> c00;
    E::DelayLine<Memory, 1> c10;
    E::DelayLine<Memory, 2> c20;
    E::DelayLine<Memory, 3> c30;
    E::DelayLine<Memory, 4> bc;
    E::DelayLine<Memory, 5> bd0;
    E::DelayLine<Memory, 6> bd1;
    E::DelayLine<Memory, 7> c01;
    E::DelayLine<Memory, 8> c11;
    E::DelayLine<Memory, 9> c21;
    E::DelayLine<Memory, 10> c31;
    E::Context c;

    /* switch active voice */
    if (trigger_ && !previous_trigger_ && !freeze_) {
      voice_ = !voice_;
    }

    if (freeze_ && !previous_freeze_) {
      previous_freeze_ = freeze_;
      voice_ = !voice_;
    }

    /* set comb filters pitch */
    comb_period_[0][voice_] = 32000.0f / BASE_PITCH / SemitonesToRatio(pitch_[voice_]);
    CONSTRAIN(comb_period_[0][voice_], 0, MAX_COMB);
    for (int p=1; p<4; p++) {
      float pitch = InterpolatePlateau(chords[p-1], chord_[voice_], 16);
      comb_period_[p][voice_] = comb_period_[0][voice_] / SemitonesToRatio(pitch);
      CONSTRAIN(comb_period_[p][voice_], 0, MAX_COMB);
    }

    /* set LP/BP filters frequencies and feedback */
    for (int p=0; p<4; p++) {
      float freq = 1.0f / comb_period_[p][voice_];
      bp_[p][voice_].set_f_q<FREQUENCY_FAST>(freq, narrow_[voice_]);
      float lp_freq = (2.0f * freq + 1.0f) * damp_[voice_];
      CONSTRAIN(lp_freq, 0.0f, 1.0f);
      lp_[p][voice_].set_f_q<FREQUENCY_FAST>(lp_freq, 0.4f);
      comb_feedback_[p][voice_] = powf(feedback_[voice_], comb_period_[p][voice_] / 32000.0f);
    }

    /* initiate burst if trigger */
    if (trigger_ && !previous_trigger_) {
      previous_trigger_ = trigger_;
      burst_time_ = comb_period_[0][voice_];
      burst_time_ *= 2.0f * burst_duration_;

      for (int i=0; i<3; i++)
        spread_delay_[i] = Random::GetFloat() * (bd0.length - 1);
    }

    rand_lp_.set_f_q<FREQUENCY_FAST>(distortion_[voice_] * 0.4f, 1.0f);

    while (size--) {
      engine_.Start(&c);

      burst_time_--;
      float burst_gain = burst_time_ > 0.0f ? 1.0f : 0.0f;

      float random = Random::GetFloat() * 2.0f - 1.0f;
      /* burst noise generation */
      c.Read(random, burst_gain);
      // goes through comb and lp filters
      const float comb_fb = 0.6f - burst_comb_ * 0.4f;
      float comb_del = burst_comb_ * bc.length;
      if (comb_del <= 1.0f) comb_del = 1.0f;
      c.InterpolateHermite(bc, comb_del, comb_fb);
      c.Write(bc, 1.0f);
      float burst;
      c.Write(burst);
      c.Load(burst_lp_.Process<FILTER_MODE_LOW_PASS>(burst));
      c.Write(burst);

      c.Load(burst);
      c.Read(in_out->l, 1.0f);
      c.Write(bd0, 0.0f);

      c.Load(burst);
      c.Read(in_out->r, 1.0f);
      c.Write(bd1, 0.0f);

      float amplitude = distortion_[voice_];
      amplitude = 1.0f - amplitude;
      amplitude *= 0.3f;
      amplitude *= amplitude;
      random *= amplitude;
      random = rand_lp_.Process<FILTER_MODE_LOW_PASS>(random);
      random = rand_hp_.Process<FILTER_MODE_HIGH_PASS>(random);

#define COMB(pre, part, voice, vol)                                     \
      {                                                                 \
        c.Load(0.0f);                                                   \
        c.Read(bd ## voice, pre * spread_amount_, vol);                 \
        float tap = comb_period_[part][voice] * (1.0f + random);        \
        c.InterpolateHermite(c ## part ## voice, tap ,                  \
                             comb_feedback_[part][voice] * 0.7f);      \
        c.InterpolateHermite(c ## part ## voice,                        \
                             tap * harmonicity_[voice],                 \
                             comb_feedback_[part][voice] * 0.3f);       \
        float acc;                                                      \
        c.Write(acc);                                                   \
        acc = lp_[part][voice].Process<FILTER_MODE_LOW_PASS>(acc);      \
        acc = bp_[part][voice].Process<FILTER_MODE_BAND_PASS_NORMALIZED>(acc); \
        c.Load(acc);                                                    \
        c.Hp(hp_[part][voice], 10.0f / 32000.0f);                       \
        c.Write(acc, 0.5f);                                             \
        c.SoftLimit();                                                  \
        c.Write(acc, 2.0f);                                             \
        c.Write(c ## part ## voice, 0.0f);                              \
      }                                                                 \

      /* first voice: */
      COMB(0, 0, 0, !voice_);
      COMB(spread_delay_[0], 1, 0, !voice_);
      COMB(spread_delay_[1], 2, 0, !voice_);
      COMB(spread_delay_[2], 3, 0, !voice_);

      /* second voice: */
      COMB(0, 0, 1, voice_);
      COMB(spread_delay_[0], 1, 1, voice_);
      COMB(spread_delay_[1], 2, 1, voice_);
      COMB(spread_delay_[2], 3, 1, voice_);

      /* left mix */
      c.Read(c00, (1.0f + 0.5f * narrow_[0]) *
             0.25f * (1.0f - stereo_) * (1.0f - separation_));
      c.Read(c10, (1.0f + 0.5f * narrow_[0]) *
             (0.25f + 0.25f * stereo_) * (1.0f - separation_));
      c.Read(c20, (1.0f + 0.5f * narrow_[0]) *
             (0.25f * (1.0f - stereo_)) * (1.0f - separation_));
      c.Read(c30, (1.0f + 0.5f * narrow_[0]) *
             (0.25f + 0.25f * stereo_) * (1.0f - separation_));
      c.Read(c01, (1.0f + 0.5f * narrow_[1]) * (0.25f + 0.25f * stereo_));
      c.Read(c11, (1.0f + 0.5f * narrow_[1]) * 0.25f * (1.0f - stereo_));
      c.Read(c21, (1.0f + 0.5f * narrow_[1]) * (0.25f + 0.25 * stereo_));
      c.Read(c31, (1.0f + 0.5f * narrow_[1]) * 0.25f * (1.0f - stereo_));
      c.Write(in_out->l, 0.0f);

      /* right mix */
      c.Read(c00, (1.0f + 0.5f * narrow_[0]) * (0.25f + 0.25f * stereo_));
      c.Read(c10, (1.0f + 0.5f * narrow_[0]) * 0.25f * (1.0f - stereo_));
      c.Read(c20, (1.0f + 0.5f * narrow_[0]) * (0.25f + 0.25f * stereo_));
      c.Read(c30, (1.0f + 0.5f * narrow_[0]) * 0.25f * (1.0f - stereo_));
      c.Read(c01, (1.0f + 0.5f * narrow_[1]) *
             0.25f * (1.0f - stereo_) * (1.0f - separation_));
      c.Read(c11, (1.0f + 0.5f * narrow_[1]) *
             (0.25f + 0.25f * stereo_) * (1.0f - separation_));
      c.Read(c21, (1.0f + 0.5f * narrow_[1]) *
             0.25f * (1.0f - stereo_) * (1.0f - separation_));
      c.Read(c31, (1.0f + 0.5f * narrow_[1]) *
             (0.25f + 0.25f * stereo_) * (1.0f - separation_));
      c.Write(in_out->r, 0.0f);

      ++in_out;
    }
  }

  void set_pitch(float pitch) {
    pitch_[voice_] = pitch;
  }

  void set_chord(float chord) {
    chord_[voice_] = chord;
  }

  void set_feedback(float feedback) {
    feedback_[voice_] = feedback;
  }

  void set_narrow(float narrow) {
    narrow_[voice_] = narrow;
  }

  void set_damp(float damp) {
    damp_[voice_] = damp;
  }

  void set_distortion(float distortion) {
    distortion *= distortion * distortion;
    distortion_[voice_] = distortion;
  }

  void set_trigger(bool trigger) {
    previous_trigger_ = trigger_;
    trigger_ = trigger;
  }

  void set_burst_damp(float burst_damp) {
    burst_lp_.set_f_q<FREQUENCY_FAST>(burst_damp * burst_damp * 0.5f, 0.8f);
  }

  void set_burst_comb(float burst_comb) {
    burst_comb_ = burst_comb;
  }

  void set_burst_duration(float burst_duration) {
    burst_duration_ = burst_duration;
  }

  void set_spread_amount(float spread_amount) {
    spread_amount_ = spread_amount;
  }

  void set_stereo(float stereo) {
    stereo_ = stereo;
  }

  void set_separation(float separation) {
    separation_ = separation;
  }

  void set_freeze(float freeze) {
    previous_freeze_ = freeze_;
    freeze_ = freeze;
  }

  void set_harmonicity(float harmonicity) {
    harmonicity_[voice_] = harmonicity;
  }

 private:
  typedef FxEngine<16384, FORMAT_32_BIT> E;
  E engine_;

  /* parameters: */
  float feedback_[2];
  float pitch_[2];
  float chord_[2];
  float narrow_[2];
  float damp_[2];
  float harmonicity_[2];
  float distortion_[2];
  float spread_amount_;
  float stereo_;
  float separation_;
  float burst_time_;
  float burst_damp_;
  float burst_comb_;
  float burst_duration_;
  int16_t trigger_, previous_trigger_;
  int16_t freeze_, previous_freeze_;

  /* internal states: */
  float spread_delay_[3];
  float comb_period_[4][2];
  float comb_feedback_[4][2];

  float hp_[4][2];
  Svf lp_[4][2];
  Svf bp_[4][2];
  Svf burst_lp_;
  Svf rand_lp_;
  OnePole rand_hp_;

  int32_t voice_, __align1;


  DISALLOW_COPY_AND_ASSIGN(Resonestor);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_RESONESTOR_H_
