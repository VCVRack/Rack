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
// Oscillator - digital style waveforms.

#ifndef BRAIDS_DIGITAL_OSCILLATOR_H_
#define BRAIDS_DIGITAL_OSCILLATOR_H_

#include "stmlib/stmlib.h"

#include "braids/excitation.h"
#include "braids/svf.h"

#include <cstring>

namespace braids {

static const size_t kWGBridgeLength = 1024;
static const size_t kWGNeckLength = 4096;
static const size_t kWGBoreLength = 2048;
static const size_t kWGJetLength = 1024;
static const size_t kWGFBoreLength = 4096;
static const size_t kCombDelayLength = 8192;

static const size_t kNumFormants = 5;
static const size_t kNumPluckVoices = 3;
static const size_t kNumOverlappingFof = 3;
static const size_t kNumBellPartials = 11;
static const size_t kNumDrumPartials = 6;
static const size_t kNumAdditiveHarmonics = 12;

enum DigitalOscillatorShape {
  OSC_SHAPE_TRIPLE_RING_MOD,
  OSC_SHAPE_SAW_SWARM,
  OSC_SHAPE_COMB_FILTER,
  OSC_SHAPE_TOY,

  OSC_SHAPE_DIGITAL_FILTER_LP,
  OSC_SHAPE_DIGITAL_FILTER_PK,
  OSC_SHAPE_DIGITAL_FILTER_BP,
  OSC_SHAPE_DIGITAL_FILTER_HP,
  OSC_SHAPE_VOSIM,
  OSC_SHAPE_VOWEL,
  OSC_SHAPE_VOWEL_FOF,
  
  OSC_SHAPE_HARMONICS,

  OSC_SHAPE_FM,
  OSC_SHAPE_FEEDBACK_FM,
  OSC_SHAPE_CHAOTIC_FEEDBACK_FM,

  OSC_SHAPE_STRUCK_BELL,
  OSC_SHAPE_STRUCK_DRUM,

  OSC_SHAPE_KICK,
  OSC_SHAPE_HAT,
  OSC_SHAPE_SNARE,
  
  OSC_SHAPE_PLUCKED,
  OSC_SHAPE_BOWED,
  OSC_SHAPE_BLOWN,
  OSC_SHAPE_FLUTED,
  
  OSC_SHAPE_WAVETABLES,
  OSC_SHAPE_WAVE_MAP,
  OSC_SHAPE_WAVE_LINE,
  OSC_SHAPE_WAVE_PARAPHONIC,
  
  OSC_SHAPE_FILTERED_NOISE,
  OSC_SHAPE_TWIN_PEAKS_NOISE,
  OSC_SHAPE_CLOCKED_NOISE,
  OSC_SHAPE_GRANULAR_CLOUD,
  OSC_SHAPE_PARTICLE_NOISE,

  OSC_SHAPE_DIGITAL_MODULATION,

  OSC_SHAPE_QUESTION_MARK_LAST
};

struct ResoSquareState {
  uint32_t modulator_phase_increment;
  uint32_t modulator_phase;
  uint32_t square_modulator_phase;
  int32_t integrator;
  bool polarity;
};

struct VowelSynthesizerState {
  uint32_t formant_increment[3];
  uint32_t formant_phase[3];
  uint32_t formant_amplitude[3];
  uint16_t consonant_frames;
  uint16_t noise;
};

struct SawSwarmState {
  uint32_t phase[6];
  int32_t filter_state[2][2];
  int32_t dc_blocked;
  int32_t lp;
  int32_t bp;
};

struct HarmonicsState {
  int32_t amplitude[kNumAdditiveHarmonics];
};

struct AdditiveState {
  uint32_t partial_phase[kNumBellPartials];
  uint32_t partial_phase_increment[kNumBellPartials];
  int32_t partial_amplitude[kNumBellPartials];
  int32_t target_partial_amplitude[kNumBellPartials];
  int16_t previous_sample;
  size_t current_partial;
  int32_t lp_noise[3];
};

struct PluckState {
  size_t size;
  size_t write_ptr;
  size_t shift;
  size_t mask;
  size_t pluck_position;
  size_t initialization_ptr;
  uint32_t phase;
  uint32_t phase_increment;
  uint32_t max_phase_increment;
  int16_t previous_sample;
  uint8_t polyphony_assigner;
};

struct FeedbackFmState {
  uint32_t modulator_phase;
  int16_t previous_sample;
};

struct ParticleNoiseState {
  uint16_t amplitude;
  int32_t filter_state[3][2];
  int32_t filter_scale[3];
  int32_t filter_coefficient[3];
};

struct PhysicalModellingState {
  uint16_t delay_ptr;
  uint16_t excitation_ptr;
  int32_t lp_state;
  int32_t filter_state[2];
  int16_t previous_sample;
};

struct Grain {
  uint32_t phase;
  uint32_t phase_increment;
  uint32_t envelope_phase;
  uint32_t envelope_phase_increment;
};

struct FofState {
  int32_t next_saw_sample;
  int16_t previous_sample;
  int32_t svf_lp[kNumFormants];
  int32_t svf_bp[kNumFormants];
};

struct ToyState {
  uint8_t held_sample;
  uint16_t decimation_counter;
};

struct SvfState {
  int32_t bp;
  int32_t lp;
};

struct DigitalModulationState {
  uint32_t symbol_phase;
  uint16_t symbol_count;
  int32_t filter_state;
  uint8_t data_byte;
};

struct ClockedNoiseState {
  uint32_t cycle_phase;
  uint32_t cycle_phase_increment;
  uint32_t rng_state;
  int32_t seed;
  int16_t sample;
};

struct HatState {
  uint32_t phase[6];
  uint32_t rng_state;
};

union DigitalOscillatorState {
  ResoSquareState res;
  VowelSynthesizerState vow;
  SawSwarmState saw;
  PluckState plk[4];
  FeedbackFmState ffm;
  ParticleNoiseState pno;
  PhysicalModellingState phy;
  Grain grain[4];
  FofState fof;
  ToyState toy;
  SvfState svf;
  AdditiveState add;
  DigitalModulationState dmd;
  ClockedNoiseState clk;
  HatState hat;
  HarmonicsState hrm;
  uint32_t modulator_phase;
};

class DigitalOscillator {
 public:
  typedef void (DigitalOscillator::*RenderFn)(const uint8_t*, int16_t*, size_t);

  DigitalOscillator() { }
  ~DigitalOscillator() { }
  
  inline void Init() {
    memset(&state_, 0, sizeof(state_));
    pulse_[0].Init();
    pulse_[1].Init();
    pulse_[2].Init();
    pulse_[3].Init();
    svf_[0].Init();
    svf_[1].Init();
    svf_[2].Init();
    phase_ = 0;
    strike_ = true;
    init_ = true;
  }
  
  inline void set_shape(DigitalOscillatorShape shape) {
    shape_ = shape;
  }
  
  inline void set_pitch(int16_t pitch) {
    // Smooth HF noise when the pitch CV is noisy.
    if (pitch_ > (90 << 7) && pitch > (90 << 7)) {
      pitch_ = (static_cast<int32_t>(pitch_) + pitch) >> 1;
    } else {
      pitch_ = pitch;
    }
  }

  inline void set_parameters(
      int16_t parameter_1,
      int16_t parameter_2) {
    parameter_[0] = parameter_1;
    parameter_[1] = parameter_2;
  }
  
  inline uint32_t phase_increment() const {
    return phase_increment_;
  }
  
  inline void Strike() {
    strike_ = true;
  }

  void Render(const uint8_t* sync, int16_t* buffer, size_t size);
  
 private:
  void RenderTripleRingMod(const uint8_t*, int16_t*, size_t);
  void RenderSawSwarm(const uint8_t*, int16_t*, size_t);
  void RenderComb(const uint8_t*, int16_t*, size_t);
  void RenderToy(const uint8_t*, int16_t*, size_t);

  void RenderDigitalFilter(const uint8_t*, int16_t*, size_t);
  void RenderVosim(const uint8_t*, int16_t*, size_t);
  void RenderVowel(const uint8_t*, int16_t*, size_t);
  void RenderVowelFof(const uint8_t*, int16_t*, size_t);

  void RenderHarmonics(const uint8_t*, int16_t*, size_t);

  void RenderFm(const uint8_t*, int16_t*, size_t);
  void RenderFeedbackFm(const uint8_t*, int16_t*, size_t);
  void RenderChaoticFeedbackFm(const uint8_t*, int16_t*, size_t);
  
  void RenderStruckBell(const uint8_t*, int16_t*, size_t);
  void RenderStruckDrum(const uint8_t*, int16_t*, size_t);
  void RenderPlucked(const uint8_t*, int16_t*, size_t);
  void RenderBowed(const uint8_t*, int16_t*, size_t);
  void RenderBlown(const uint8_t*, int16_t*, size_t);
  void RenderFluted(const uint8_t*, int16_t*, size_t);

  void RenderWavetables(const uint8_t*, int16_t*, size_t);
  void RenderWaveMap(const uint8_t*, int16_t*, size_t);
  void RenderWaveLine(const uint8_t*, int16_t*, size_t);
  void RenderWaveParaphonic(const uint8_t*, int16_t*, size_t);
  
  void RenderTwinPeaksNoise(const uint8_t*, int16_t*, size_t);
  void RenderFilteredNoise(const uint8_t*, int16_t*, size_t);
  void RenderClockedNoise(const uint8_t*, int16_t*, size_t);
  void RenderGranularCloud(const uint8_t*, int16_t*, size_t);
  void RenderParticleNoise(const uint8_t*, int16_t*, size_t);
  
  void RenderDigitalModulation(const uint8_t*, int16_t*, size_t);
  void RenderKick(const uint8_t*, int16_t*, size_t);
  void RenderSnare(const uint8_t*, int16_t*, size_t);
  void RenderCymbal(const uint8_t*, int16_t*, size_t);
  void RenderQuestionMark(const uint8_t*, int16_t*, size_t);
  
  // void RenderYourAlgo(const uint8_t*, int16_t*, size_t);
  
  uint32_t ComputePhaseIncrement(int16_t midi_pitch);
  uint32_t ComputeDelay(int16_t midi_pitch);
  int16_t InterpolateFormantParameter(
      const int16_t table[][kNumFormants][kNumFormants],
      int16_t x,
      int16_t y,
      uint8_t formant);
   
  uint32_t phase_;
  uint32_t phase_increment_;
  uint32_t delay_;

  int16_t parameter_[2];
  int16_t previous_parameter_[2];
  int32_t smoothed_parameter_;
  int16_t pitch_;
  
  uint8_t active_voice_;
  
  bool init_;
  bool strike_;

  DigitalOscillatorShape shape_;
  DigitalOscillatorShape previous_shape_;
  DigitalOscillatorState state_;
  
  Excitation pulse_[4];
  Svf svf_[3];
  
  union {
    int16_t comb[kCombDelayLength];
    int16_t ks[1025 * 4];
    struct {
      int8_t bridge[kWGBridgeLength];
      int8_t neck[kWGNeckLength];
    } bowed;
    int16_t bore[kWGBoreLength];
    struct {
      int8_t jet[kWGJetLength];
      int8_t bore[kWGFBoreLength];
    } fluted;
  } delay_lines_;
  
  static RenderFn fn_table_[];
  
  DISALLOW_COPY_AND_ASSIGN(DigitalOscillator);
};

}  // namespace braids

#endif // BRAIDS_DIGITAL_OSCILLATOR_H_
