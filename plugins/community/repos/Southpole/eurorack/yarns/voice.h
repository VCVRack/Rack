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
// Voice.

#ifndef YARNS_VOICE_H_
#define YARNS_VOICE_H_

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

namespace yarns {

const uint16_t kNumOctaves = 11;
const size_t kAudioBlockSize = 64;

enum TriggerShape {
  TRIGGER_SHAPE_SQUARE,
  TRIGGER_SHAPE_LINEAR,
  TRIGGER_SHAPE_EXPONENTIAL,
  TRIGGER_SHAPE_RING,
  TRIGGER_SHAPE_STEPS,
  TRIGGER_SHAPE_NOISE_BURST
};

enum AudioMode {
  AUDIO_MODE_OFF,
  AUDIO_MODE_SAW,
  AUDIO_MODE_SQUARE,
  AUDIO_MODE_TRIANGLE,
  AUDIO_MODE_SINE
};

class Oscillator {
 public:
  Oscillator() { }
  ~Oscillator() { }
  void Init(int32_t scale, int32_t offset);
  void Render(uint8_t mode, int16_t note, bool gate);
  inline uint16_t ReadSample() {
    return audio_buffer_.ImmediateRead();
  }

 private:
  uint32_t ComputePhaseIncrement(int16_t pitch);
  
  void RenderSilence();
  void RenderNoise();
  void RenderSine(uint32_t phase_increment);
  void RenderSaw(uint32_t phase_increment);
  void RenderSquare(uint32_t phase_increment, uint32_t pw, bool integrate);

  inline int32_t ThisBlepSample(uint32_t t) {
    if (t > 65535) {
      t = 65535;
    }
    return t * t >> 18;
  }
  
  inline int32_t NextBlepSample(uint32_t t) {
    if (t > 65535) {
      t = 65535;
    }
    t = 65535 - t;
    return -static_cast<int32_t>(t * t >> 18);
  }
  
  int32_t scale_;
  int32_t offset_;
  uint32_t phase_;
  int32_t next_sample_;
  int32_t integrator_state_;
  bool high_;
  stmlib::RingBuffer<uint16_t, kAudioBlockSize * 2> audio_buffer_;
  
  DISALLOW_COPY_AND_ASSIGN(Oscillator);
};

class Voice {
 public:
  Voice() { }
  ~Voice() { }
  
  void Init();
  void ResetAllControllers();

  void Calibrate(uint16_t* calibrated_dac_code);
  void Refresh();
  void NoteOn(int16_t note, uint8_t velocity, uint8_t portamento, bool trigger);
  void NoteOff();
  void ControlChange(uint8_t controller, uint8_t value);
  void PitchBend(uint16_t pitch_bend) {
    mod_pitch_bend_ = pitch_bend;
  }
  void Aftertouch(uint8_t velocity) {
    mod_aux_[2] = velocity << 9;
  }

  inline void set_modulation_rate(uint8_t modulation_rate) {
    modulation_rate_ = modulation_rate;
  }
  inline void set_pitch_bend_range(uint8_t pitch_bend_range) {
    pitch_bend_range_ = pitch_bend_range;
  }
  inline void set_vibrato_range(uint8_t vibrato_range) {
    vibrato_range_ = vibrato_range;
  }
  inline void set_trigger_duration(uint8_t trigger_duration) {
    trigger_duration_ = trigger_duration;
  }
  inline void set_trigger_scale(uint8_t trigger_scale) {
    trigger_scale_ = trigger_scale;
  }
  inline void set_trigger_shape(uint8_t trigger_shape) {
    trigger_shape_ = trigger_shape;
  }
  inline void set_aux_cv(uint8_t aux_cv_source) {
    aux_cv_source_ = aux_cv_source;
  }
  inline void set_aux_cv_2(uint8_t aux_cv_source_2) {
    aux_cv_source_2_ = aux_cv_source_2;
  }
  
  inline int32_t note() const { return note_; }
  inline uint8_t velocity() const { return mod_velocity_; }
  inline uint8_t modulation() const { return mod_wheel_; }
  inline uint8_t aux_cv() const { return mod_aux_[aux_cv_source_] >> 8; }
  inline uint8_t aux_cv_2() const { return mod_aux_[aux_cv_source_2_] >> 8; }

  inline uint16_t DacCodeFrom16BitValue(uint16_t value) const {
    uint32_t v = static_cast<uint32_t>(value);
    uint32_t scale = calibrated_dac_code_[3] - calibrated_dac_code_[8];
    return static_cast<uint16_t>(calibrated_dac_code_[3] - (scale * v >> 16));
  }

  inline uint16_t note_dac_code() const {
    return note_dac_code_;
  }

  inline uint16_t velocity_dac_code() const {
    return DacCodeFrom16BitValue(mod_velocity_ << 9);
  }
  inline uint16_t modulation_dac_code() const {
    return DacCodeFrom16BitValue(mod_wheel_ << 9);
  }
  inline uint16_t aux_cv_dac_code() const { 
    return DacCodeFrom16BitValue(mod_aux_[aux_cv_source_]);
  }
  inline uint16_t aux_cv_dac_code_2() const { 
    return DacCodeFrom16BitValue(mod_aux_[aux_cv_source_2_]);
  }
  
  inline bool gate_on() const { return gate_; }

  inline bool gate() const { return gate_ && !retrigger_delay_; }
  inline bool trigger() const  {
    return gate_ && trigger_pulse_;
  }
  
  uint16_t trigger_dac_code() const;
  
  inline uint16_t calibration_dac_code(uint8_t note) const {
    return calibrated_dac_code_[note];
  }
  
  inline void set_calibration_dac_code(uint8_t note, uint16_t dac_code) {
    calibrated_dac_code_[note] = dac_code;
    dirty_ = true;
  }
  
  inline void set_audio_mode(uint8_t audio_mode) {
    audio_mode_ = audio_mode;
  }
  
  inline void set_tuning(int8_t coarse, int8_t fine) {
    tuning_ = (static_cast<int32_t>(coarse) << 7) + fine;
  }
  
  inline uint8_t audio_mode() {
    return audio_mode_;
  }
  inline void RenderAudio() {
    oscillator_.Render(audio_mode_, note_, gate_);
  }
  inline uint16_t ReadSample() {
    return oscillator_.ReadSample();
  }
  
  void TapLfo(uint32_t target_phase) {
    uint32_t target_increment = target_phase - lfo_pll_previous_target_phase_;
    
    int32_t d_error = target_increment - (lfo_phase_ - lfo_pll_previous_phase_);
    int32_t p_error = target_phase - lfo_phase_;
    int32_t error = d_error + (p_error >> 1);
    
    lfo_pll_phase_increment_ += error >> 11;
    
    lfo_pll_previous_phase_ = lfo_phase_;
    lfo_pll_previous_target_phase_ = target_phase;
  }
  
 private:
  uint16_t NoteToDacCode(int32_t note) const;
  void FillAudioBuffer();

  int32_t note_source_;
  int32_t note_target_;
  int32_t note_portamento_;
  int32_t note_;
  int32_t tuning_;
  bool gate_;
  
  bool dirty_;  // Set to true when the calibration settings have changed.
  uint16_t note_dac_code_;
  uint16_t calibrated_dac_code_[kNumOctaves];
  
  int16_t mod_pitch_bend_;
  uint8_t mod_wheel_;
  uint16_t mod_aux_[8];
  uint8_t mod_velocity_;
  
  uint8_t pitch_bend_range_;
  uint8_t modulation_rate_;
  uint8_t vibrato_range_;
  
  uint8_t trigger_duration_;
  uint8_t trigger_shape_;
  bool trigger_scale_;
  uint8_t aux_cv_source_;
  uint8_t aux_cv_source_2_;
  
  uint32_t lfo_phase_;
  uint32_t portamento_phase_;
  uint32_t portamento_phase_increment_;
  bool portamento_exponential_shape_;
  
  // This counter is used to artificially create a 500µs dip at LOW level when
  // the gate is currently HIGH and a new note arrive with a retrigger command.
  // This happens with note-stealing; or when sending a MIDI sequence with
  // overlapping notes.
  uint16_t retrigger_delay_;
  
  uint16_t trigger_pulse_;
  uint32_t trigger_phase_increment_;
  uint32_t trigger_phase_;
  
  // PLL for clock-synced LFO.
  uint32_t lfo_pll_phase_increment_;
  uint32_t lfo_pll_previous_target_phase_;
  uint32_t lfo_pll_previous_phase_;
  
  uint8_t audio_mode_;
  Oscillator oscillator_;

  DISALLOW_COPY_AND_ASSIGN(Voice);
};

}  // namespace yarns

#endif // YARNS_VOICE_H_
