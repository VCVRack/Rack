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
// Part.

#ifndef YARNS_PART_H_
#define YARNS_PART_H_

#include <algorithm>

#include "stmlib/stmlib.h"
#include "stmlib/algorithms/voice_allocator.h"
#include "stmlib/algorithms/note_stack.h"

namespace yarns {

class Voice;

const uint8_t kNumSteps = 64;
const uint8_t kMaxNumVoices = 4;

enum ArpeggiatorDirection {
  ARPEGGIATOR_DIRECTION_UP,
  ARPEGGIATOR_DIRECTION_DOWN,
  ARPEGGIATOR_DIRECTION_UP_DOWN,
  ARPEGGIATOR_DIRECTION_RANDOM,
  ARPEGGIATOR_DIRECTION_AS_PLAYED,
  ARPEGGIATOR_DIRECTION_CHORD,
  ARPEGGIATOR_DIRECTION_LAST
};

enum VoiceAllocationMode {
  VOICE_ALLOCATION_MODE_MONO,
  VOICE_ALLOCATION_MODE_POLY,
  VOICE_ALLOCATION_MODE_POLY_CYCLIC,
  VOICE_ALLOCATION_MODE_POLY_RANDOM,
  VOICE_ALLOCATION_MODE_POLY_VELOCITY,
  VOICE_ALLOCATION_MODE_POLY_SORTED,
  VOICE_ALLOCATION_MODE_POLY_UNISON_1,
  VOICE_ALLOCATION_MODE_POLY_UNISON_2,
  VOICE_ALLOCATION_MODE_LAST
};

enum VoiceAllocationFlags {
  VOICE_ALLOCATION_NOT_FOUND = 0xff
};

enum MidiOutMode {
  MIDI_OUT_MODE_OFF,
  MIDI_OUT_MODE_THRU,
  MIDI_OUT_MODE_GENERATED_EVENTS
};

enum TuningSystem {
  TUNING_SYSTEM_EQUAL,
  TUNING_SYSTEM_JUST_INTONATION,
  TUNING_SYSTEM_PYTHAGOREAN,
  TUNING_SYSTEM_QUARTER_EB,
  TUNING_SYSTEM_QUARTER_E,
  TUNING_SYSTEM_QUARTER_EA,
  TUNING_SYSTEM_RAGA_1,
  TUNING_SYSTEM_RAGA_27 = TUNING_SYSTEM_RAGA_1 + 26,
  TUNING_SYSTEM_CUSTOM,
  TUNING_SYSTEM_LAST
};

struct MidiSettings {
  uint8_t channel;
  uint8_t min_note;
  uint8_t max_note;
  uint8_t min_velocity;
  uint8_t max_velocity;
  uint8_t out_mode;
  uint8_t padding[10];
};

struct VoicingSettings {
  uint8_t allocation_mode;
  uint8_t allocation_priority;
  uint8_t portamento;
  uint8_t legato_mode;
  uint8_t pitch_bend_range;
  uint8_t vibrato_range;
  uint8_t modulation_rate;
  int8_t tuning_transpose;
  int8_t tuning_fine;
  int8_t tuning_root;
  uint8_t tuning_system;
  uint8_t trigger_duration;
  uint8_t trigger_scale;
  uint8_t trigger_shape;
  uint8_t aux_cv;
  uint8_t audio_mode;
  uint8_t aux_cv_2;
  uint8_t padding[15];
};


enum PartSetting {
  PART_MIDI_CHANNEL,
  PART_MIDI_MIN_NOTE,
  PART_MIDI_MAX_NOTE,
  PART_MIDI_MIN_VELOCITY,
  PART_MIDI_MAX_VELOCITY,
  PART_MIDI_OUT_MODE,
  PART_MIDI_LAST = PART_MIDI_CHANNEL + sizeof(MidiSettings) - 1,
  PART_VOICING_ALLOCATION_MODE,
  PART_VOICING_ALLOCATION_PRIORITY,
  PART_VOICING_PORTAMENTO,
  PART_VOICING_LEGATO_MODE,
  PART_VOICING_PITCH_BEND_RANGE,
  PART_VOICING_VIBRATO_RANGE,
  PART_VOICING_MODULATION_RATE,
  PART_VOICING_TUNING_TRANSPOSE,
  PART_VOICING_TUNING_FINE,
  PART_VOICING_TUNING_ROOT,
  PART_VOICING_TUNING_SYSTEM,
  PART_VOICING_TRIGGER_DURATION,
  PART_VOICING_TRIGGER_SCALE,
  PART_VOICING_TRIGGER_SHAPE,
  PART_VOICING_AUX_CV,
  PART_VOICING_AUDIO_MODE,
  PART_VOICING_AUX_CV_2,
  PART_VOICING_LAST = PART_VOICING_ALLOCATION_MODE + sizeof(VoicingSettings) - 1,
  PART_SEQUENCER_CLOCK_DIVISION,
  PART_SEQUENCER_GATE_LENGTH,
  PART_SEQUENCER_ARP_RANGE,
  PART_SEQUENCER_ARP_DIRECTION,
  PART_SEQUENCER_ARP_PATTERN,
  PART_SEQUENCER_EUCLIDEAN_LENGTH,
  PART_SEQUENCER_EUCLIDEAN_FILL,
  PART_SEQUENCER_EUCLIDEAN_ROTATE
};

enum SequencerStepFlags {
  SEQUENCER_STEP_REST = 0x80,
  SEQUENCER_STEP_TIE = 0x81
};

struct SequencerStep {
  // BYTE 0:
  // 0x00 to 0x7f: note
  // 0x80: rest
  // 0x81: tie
  //
  // BYTE 1:
  // 7 bits of velocity + 1 bit for slide flag.
  SequencerStep() { }
  SequencerStep(uint8_t data_0, uint8_t data_1) {
    data[0] = data_0;
    data[1] = data_1;
  }
  
  uint8_t data[2];
  
  inline bool has_note() const { return !(data[0] & 0x80); }
  inline bool is_rest() const { return data[0] == SEQUENCER_STEP_REST; }
  inline bool is_tie() const { return data[0] == SEQUENCER_STEP_TIE; }
  inline uint8_t note() const { return data[0] & 0x7f; }
  
  inline bool is_slid() const { return data[1] & 0x80; }
  inline uint8_t velocity() const { return data[1] & 0x7f; }
};

struct SequencerSettings {
  uint8_t clock_division;
  uint8_t gate_length;
  uint8_t arp_range;
  uint8_t arp_direction;
  uint8_t arp_pattern;
  uint8_t euclidean_length;
  uint8_t euclidean_fill;
  uint8_t euclidean_rotate;
  uint8_t num_steps;
  SequencerStep step[kNumSteps];
  uint8_t padding[7];
  
  int16_t first_note() {
    for (uint8_t i = 0; i < num_steps; ++i) {
      if (step[i].has_note()) {
        return step[i].note();
      }
    }
    return 60;
  }
};

class Part {
 public:
  Part() { }
  ~Part() { }
  
  void Init();
  
  // The return value indicates whether the message can be forwarded to the
  // MIDI out (soft-thru). For example, when the arpeggiator is on, NoteOn
  // or NoteOff can return false to make sure that the chord that triggers
  // the arpeggiator does not find its way to the MIDI out. Instead it will 
  // be sent note by note within InternalNoteOn and InternalNoteOff.
  //
  // Also, note that channel / keyrange / velocity range filtering is not
  // applied here. It is up to the caller to call accepts() first to check
  // whether the message should be sent to the part.
  bool NoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  bool NoteOff(uint8_t channel, uint8_t note);
  bool ControlChange(uint8_t channel, uint8_t controller, uint8_t value);
  bool PitchBend(uint8_t channel, uint16_t pitch_bend);
  bool Aftertouch(uint8_t channel, uint8_t note, uint8_t velocity);
  bool Aftertouch(uint8_t channel, uint8_t velocity);
  void AllNotesOff();
  void Reset();
  void Clock();
  void Start(bool started_by_keyboard);
  void Stop();
  void StopRecording() {
    seq_recording_ = false;
  }
  void StartRecording();
  
  inline void RecordStep(const SequencerStep& step) {
    if (seq_recording_) {
      seq_.step[seq_rec_step_].data[0] = step.data[0];
      seq_.step[seq_rec_step_].data[1] |= step.data[1];
      ++seq_rec_step_;
      uint8_t last_step = seq_overdubbing_ ? seq_.num_steps : kNumSteps;
      // Extend sequence.
      if (!seq_overdubbing_ && seq_rec_step_ > seq_.num_steps) {
        seq_.num_steps = seq_rec_step_;
      }
      // Wrap to first step.
      if (seq_rec_step_ >= last_step) {
        seq_rec_step_ = 0;
      }
    }
  }

  inline void ModifyNoteAtCurrentStep(uint8_t note) {
    if (seq_recording_) {
      seq_.step[seq_rec_step_].data[0] = note;
    }
  }
  
  inline void RecordStep(SequencerStepFlags flag) {
    RecordStep(SequencerStep(flag, 0));
  }
  
  inline bool accepts(uint8_t channel) const {
    return midi_.channel == 0x10 || midi_.channel == channel;
  }
  
  inline bool accepts(uint8_t channel, uint8_t note) const {
    if (!accepts(channel)) {
      return false;
    }
    if (midi_.min_note <= midi_.max_note) {
      return note >= midi_.min_note && note <= midi_.max_note;
    } else {
      return note <= midi_.max_note || note >= midi_.min_note;
    }
  }
  
  inline bool accepts(uint8_t channel, uint8_t note, uint8_t velocity) const {
    return accepts(channel, note) && \
        velocity >= midi_.min_velocity && \
        velocity <= midi_.max_velocity;
  }
  
  void AllocateVoices(Voice* voice, uint8_t num_voices, bool polychain);
  inline void set_custom_pitch_table(int8_t* table) {
    custom_pitch_table_ = table;
  }
  
  inline uint8_t tx_channel() const {
    return midi_.channel == 0x10 ? 0 : midi_.channel;
  }
  inline bool direct_thru() const {
    return midi_.out_mode == MIDI_OUT_MODE_THRU && !polychained_;
  }
  
  inline uint8_t FindVoiceForNote(uint8_t note) const {
    for (uint8_t i = 0; i < num_voices_; ++i) {
      if (active_note_[i] == note) {
        return i;
      }
    }
    return VOICE_ALLOCATION_NOT_FOUND;
  }
  
  void Set(uint8_t address, uint8_t value);
  inline uint8_t Get(uint8_t address) const {
    const uint8_t* bytes;
    bytes = static_cast<const uint8_t*>(static_cast<const void*>(&midi_));
    return bytes[address];
  }

  inline const MidiSettings& midi_settings() const { return midi_; }
  inline const VoicingSettings& voicing_settings() const { return voicing_; }
  inline const SequencerSettings& sequencer_settings() const { return seq_; }
  inline MidiSettings* mutable_midi_settings() { return &midi_; }
  inline VoicingSettings* mutable_voicing_settings() { return &voicing_; }
  inline SequencerSettings* mutable_sequencer_settings() { return &seq_; }

  inline bool has_notes() const { return pressed_keys_.size() != 0; }
  
  inline bool recording() const { return seq_recording_; }
  inline bool overdubbing() const { return seq_overdubbing_; }
  inline uint8_t recording_step() const { return seq_rec_step_; }
  inline uint8_t num_steps() const { return seq_.num_steps; }
  inline void set_recording_step(uint8_t n) { seq_rec_step_ = n; }
  
  void Touch() {
    TouchVoices();
    TouchVoiceAllocation();
  }
  
  inline void Latch() {
    ignore_note_off_messages_ = true;
    release_latched_keys_on_next_note_on_ = true;
  }
  inline void Unlatch() {
    ignore_note_off_messages_ = false;
    release_latched_keys_on_next_note_on_ = true;
  }
  
  void set_siblings(bool has_siblings) {
    has_siblings_ = has_siblings;
  }
  
 private:
  int16_t Tune(int16_t note);
  void ResetAllControllers();
  void TouchVoiceAllocation();
  void TouchVoices();
  void InternalNoteOn(uint8_t note, uint8_t velocity);
  void InternalNoteOff(uint8_t note);
  
  void ReleaseLatchedNotes();
  void DispatchSortedNotes(bool unison);
  void KillAllInstancesOfNote(uint8_t note);
  
  void ClockSequencer();
  void ClockArpeggiator();
  void StopSequencerArpeggiatorNotes();

  MidiSettings midi_;
  VoicingSettings voicing_;
  SequencerSettings seq_;
  
  Voice* voice_[kMaxNumVoices];
  int8_t* custom_pitch_table_;
  uint8_t num_voices_;
  bool polychained_;
  
  bool ignore_note_off_messages_;
  bool release_latched_keys_on_next_note_on_;
  
  stmlib::NoteStack<12> pressed_keys_;
  stmlib::NoteStack<12> generated_notes_;  // by sequencer or arpeggiator.
  stmlib::NoteStack<12> mono_allocator_;
  stmlib::VoiceAllocator<kMaxNumVoices * 2> poly_allocator_;
  uint8_t active_note_[kMaxNumVoices];
  uint8_t cyclic_allocation_note_counter_;
  
  uint8_t arp_seq_prescaler_;
  
  uint8_t arp_step_;
  int8_t arp_note_;
  int8_t arp_octave_;
  int8_t arp_direction_;
  
  bool seq_running_;
  bool seq_recording_;
  bool seq_overdubbing_;
  uint8_t seq_step_;
  uint8_t seq_rec_step_;
  
  uint16_t gate_length_counter_;
  uint16_t lfo_counter_;
  
  bool has_siblings_;
  
  DISALLOW_COPY_AND_ASSIGN(Part);
};

}  // namespace yarns

#endif // YARNS_PART_H_
