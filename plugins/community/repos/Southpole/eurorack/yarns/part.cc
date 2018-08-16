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

#include "yarns/part.h"

#include <algorithm>

#include "stmlib/midi/midi.h"
#include "stmlib/utils/random.h"

#include "yarns/just_intonation_processor.h"
#include "yarns/midi_handler.h"
#include "yarns/resources.h"
#include "yarns/voice.h"

namespace yarns {

const uint8_t clock_divisions[] = {
  96, 48, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1
};
  
using namespace stmlib;
using namespace stmlib_midi;
using namespace std;

void Part::Init() {
  pressed_keys_.Init();
  mono_allocator_.Init();
  poly_allocator_.Init();
  generated_notes_.Init();
  std::fill(
      &active_note_[0],
      &active_note_[kMaxNumVoices],
      VOICE_ALLOCATION_NOT_FOUND);
  num_voices_ = 0;
  polychained_ = false;
  ignore_note_off_messages_ = false;
  seq_recording_ = false;
  seq_running_ = false;
  release_latched_keys_on_next_note_on_ = false;
}
  
void Part::AllocateVoices(Voice* voice, uint8_t num_voices, bool polychain) {
  AllNotesOff();
    
  num_voices_ = std::min(num_voices, kMaxNumVoices);
  polychained_ = polychain;
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i] = voice + i;
  }
  poly_allocator_.Clear();
  poly_allocator_.set_size(num_voices_ * (polychain ? 2 : 1));
  TouchVoices();
}

bool Part::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  bool sent_from_step_editor = channel & 0x80;
  
  if (release_latched_keys_on_next_note_on_) {
    bool still_latched = ignore_note_off_messages_;

    // Releasing all latched key will generate "fake" NoteOff messages. We
    // should note ignore them.
    ignore_note_off_messages_ = false;
    ReleaseLatchedNotes();
    
    release_latched_keys_on_next_note_on_ = still_latched;
    ignore_note_off_messages_ = still_latched;
  }
  pressed_keys_.NoteOn(note, velocity);
  
  if ((!(seq_.num_steps && seq_running_) && !seq_.arp_range)
      || sent_from_step_editor) {
    InternalNoteOn(note, velocity);
  }

  if (seq_recording_ && !sent_from_step_editor) {
    RecordStep(SequencerStep(note, velocity));
  }
  return midi_.out_mode == MIDI_OUT_MODE_THRU && !polychained_;
}

bool Part::NoteOff(uint8_t channel, uint8_t note) {
  bool sent_from_step_editor = channel & 0x80;

  if (ignore_note_off_messages_) {
    for (uint8_t i = 1; i <= pressed_keys_.max_size(); ++i) {
      // Flag the note so that it is removed once the sustain pedal is released.
      NoteEntry* e = pressed_keys_.mutable_note(i);
      if (e->note == note && e->velocity) {
        e->velocity |= 0x80;
      }
    }
  } else {
    pressed_keys_.NoteOff(note);
    
    if ((!(seq_.num_steps && seq_running_) && !seq_.arp_range) ||
        sent_from_step_editor) {
      InternalNoteOff(note);
    }
  }
  return midi_.out_mode == MIDI_OUT_MODE_THRU && !polychained_;
}

bool Part::ControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
  switch (controller) {
    case kCCModulationWheelMsb:
    case kCCBreathController:
    case kCCFootPedalMsb:
      for (uint8_t i = 0; i < num_voices_; ++i) {
        voice_[i]->ControlChange(controller, value);
      }
      break;
      
    case kCCOmniModeOff:
      midi_.channel = channel;
      break;
      
    case kCCOmniModeOn:
      midi_.channel = 0x10;
      break;
      
    case kCCMonoModeOn:
      voicing_.allocation_mode = VOICE_ALLOCATION_MODE_MONO;
      TouchVoiceAllocation();
      break;
      
    case kCCPolyModeOn:
      voicing_.allocation_mode = VOICE_ALLOCATION_MODE_POLY;
      TouchVoiceAllocation();
      break;
      
    case kCCHoldPedal:
      {
        if (value >= 64) {
          ignore_note_off_messages_ = true;
        } else {
          ignore_note_off_messages_ = false;
          ReleaseLatchedNotes();
        }
      }
      break;
    
    case 0x70:
      if (seq_recording_) {
        RecordStep(SEQUENCER_STEP_TIE);
      }
      break;
    
    case 0x71:
      if (seq_recording_) {
        RecordStep(SEQUENCER_STEP_REST);
      }
      break;
    
    case 0x78:
      AllNotesOff();
      break;
      
    case 0x79:
      ResetAllControllers();
      break;
      
    case 0x7b:
      AllNotesOff();
      break;
  }
  return midi_.out_mode != MIDI_OUT_MODE_OFF;
}

bool Part::PitchBend(uint8_t channel, uint16_t pitch_bend) {
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i]->PitchBend(pitch_bend);
  }
  
  if (seq_recording_ &&
      (pitch_bend > 8192 + 2048 || pitch_bend < 8192 - 2048)) {
    seq_.step[seq_rec_step_].data[1] |= 0x80;
  }
  
  return midi_.out_mode != MIDI_OUT_MODE_OFF;
}

bool Part::Aftertouch(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (voicing_.allocation_mode != VOICE_ALLOCATION_MODE_MONO) {
    uint8_t voice_index = \
        voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY ? \
        poly_allocator_.Find(note) : \
        FindVoiceForNote(note);
    if (voice_index < poly_allocator_.size()) {
      voice_[voice_index]->Aftertouch(velocity);
    }
  } else {
    Aftertouch(channel, velocity);
  }
  return midi_.out_mode != MIDI_OUT_MODE_OFF;
}

bool Part::Aftertouch(uint8_t channel, uint8_t velocity) {
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i]->Aftertouch(velocity);
  }
  return midi_.out_mode != MIDI_OUT_MODE_OFF;
}

void Part::Reset() {
  Stop();
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i]->NoteOff();
    voice_[i]->ResetAllControllers();
  }
}

void Part::Clock() {
  if (!arp_seq_prescaler_) {
    if (seq_.num_steps && seq_running_) {
      ClockSequencer();
    } else if (seq_.arp_range) {
      ClockArpeggiator();
    }
  }
  
  if (!gate_length_counter_ && generated_notes_.size()) {
    StopSequencerArpeggiatorNotes();
  } else {
    --gate_length_counter_;
  }
  
  ++arp_seq_prescaler_;
  if (arp_seq_prescaler_ >= clock_divisions[seq_.clock_division]) {
    arp_seq_prescaler_ = 0;
  }
  
  if (voicing_.modulation_rate >= 100) {
    uint32_t num_ticks = clock_divisions[voicing_.modulation_rate - 100];
    uint32_t expected_phase = (lfo_counter_ % num_ticks) * 65536 / num_ticks;
    for (uint8_t i = 0; i < num_voices_; ++i) {
      voice_[i]->TapLfo(expected_phase << 16);
    }
  }
  ++lfo_counter_;
}

void Part::Start(bool started_by_keyboard) {
  arp_seq_prescaler_ = 0;

  seq_step_ = 0;
  seq_running_ = !started_by_keyboard;
  
  release_latched_keys_on_next_note_on_ = false;
  ignore_note_off_messages_ = false;
  
  if (seq_.arp_direction == ARPEGGIATOR_DIRECTION_DOWN) {
    arp_note_ = pressed_keys_.size() - 1;
    arp_octave_ = seq_.arp_range - 1;
    arp_direction_ = -1;
  } else {
    arp_note_ = 0;
    arp_octave_ = 0;
    arp_direction_ = 1;
  }
  arp_step_ = 0;
  
  lfo_counter_ = 0;
  
  generated_notes_.Clear();
}

void Part::Stop() {
  seq_running_ = false;
  StopSequencerArpeggiatorNotes();
  AllNotesOff();
}

void Part::StartRecording() {
  if (seq_recording_) {
    return;
  }
  seq_recording_ = true;
  seq_rec_step_ = 0;
  seq_overdubbing_ = seq_.num_steps && seq_running_;
  if (!seq_overdubbing_) {
    std::fill(
        &seq_.step[0],
        &seq_.step[kNumSteps],
        SequencerStep(SEQUENCER_STEP_REST, 0));
    seq_.num_steps = 0;
  }
}

void Part::StopSequencerArpeggiatorNotes() {
  while (generated_notes_.size()) {
    uint8_t note = generated_notes_.sorted_note(0).note;
    generated_notes_.NoteOff(note);
    InternalNoteOff(note);
  }
}

void Part::ClockSequencer() {
  const SequencerStep& step = seq_.step[seq_step_];

  if (step.has_note()) {
    int16_t note = step.note();
    if (pressed_keys_.size() && !seq_recording_) {
      // When we play a monophonic sequence, we can make the guess that root
      // note = first note.
      // But this is not the case when we are playing several sequences at the
      // same time. In this case, we use root note = 60.
      int8_t root_note = !has_siblings_ ? seq_.first_note() : 60;
      note += pressed_keys_.most_recent_note().note - root_note;
      while (note > 127) {
        note -= 12;
      }
      while (note < 0) {
        note += 12;
      }
    }
    if (!step.is_slid()) {
      StopSequencerArpeggiatorNotes();
      InternalNoteOn(note, step.velocity());
    } else {
      InternalNoteOn(note, step.velocity());
      StopSequencerArpeggiatorNotes();
    }
    generated_notes_.NoteOn(note, step.velocity());
    gate_length_counter_ = seq_.gate_length;
  }
  ++seq_step_;
  if (seq_step_ >= seq_.num_steps) {
    seq_step_ = 0;
  }
  if (seq_.step[seq_step_].is_tie() || seq_.step[seq_step_].is_slid()) {
    // The next step contains a "sustain" message; or a slid note. Extends
    // the duration of the current note.
    gate_length_counter_ += clock_divisions[seq_.clock_division];
  }
}

void Part::ClockArpeggiator() {
  uint32_t pattern_mask = 1 << arp_step_;

  uint32_t pattern = lut_arpeggiator_patterns[seq_.arp_pattern];
  uint32_t pattern_length = 16;
  
  if (seq_.euclidean_length != 0) {
    pattern_length = seq_.euclidean_length;
    pattern_mask = 1 << ((arp_step_ + seq_.euclidean_rotate) % pattern_length);
    // Read euclidean pattern from ROM.
    uint16_t offset = static_cast<uint16_t>(seq_.euclidean_length - 1) * 32;
    pattern = lut_euclidean[offset + seq_.euclidean_fill];
  }
  
  uint8_t num_notes = pressed_keys_.size();
  if ((pattern_mask & pattern) && num_notes) {
    // Update arepggiator note/octave counter.
    if (num_notes == 1 && seq_.arp_range == 1) {
      // This is a corner case for the Up/down pattern code.
      // Get it out of the way.
      arp_note_ = 0;
      arp_octave_ = 0;
    } else {
      if (seq_.arp_direction == ARPEGGIATOR_DIRECTION_RANDOM) {
        uint16_t random = Random::GetSample();
        arp_octave_ = (random & 0xff) % seq_.arp_range;
        arp_note_ = (random >> 8) % num_notes;
      } else {
        bool wrapped = true;
        while (wrapped) {
          if (arp_note_ >= num_notes || arp_note_ < 0) {
            arp_octave_ += arp_direction_;
            arp_note_ = arp_direction_ > 0 ? 0 : num_notes - 1;
          }
          wrapped = false;
          if (arp_octave_ >= seq_.arp_range || arp_octave_ < 0) {
            arp_octave_ = arp_direction_ > 0 ? 0 : seq_.arp_range - 1;
            if (seq_.arp_direction == ARPEGGIATOR_DIRECTION_UP_DOWN) {
              arp_direction_ = -arp_direction_;
              arp_note_ = arp_direction_ > 0 ? 1 : num_notes - 2;
              arp_octave_ = arp_direction_ > 0 ? 0 : seq_.arp_range - 1;
              wrapped = true;
            }
          }
        }
      }
    }
    
    // Kill pending notes (if any).
    StopSequencerArpeggiatorNotes();
    
    // Trigger arpeggiated note or chord.
    if (seq_.arp_direction != ARPEGGIATOR_DIRECTION_CHORD) {
      const NoteEntry* arpeggio_note = \
          seq_.arp_direction == ARPEGGIATOR_DIRECTION_AS_PLAYED ?
          &pressed_keys_.played_note(arp_note_) :
          &pressed_keys_.sorted_note(arp_note_);
      uint8_t note = arpeggio_note->note;
      uint8_t velocity = arpeggio_note->velocity & 0x7f;
      note += 12 * arp_octave_;
      while (note > 127) {
        note -= 12;
      }
      generated_notes_.NoteOn(note, velocity);
      InternalNoteOn(note, velocity);
    } else {
      for (uint8_t i = 0; i < num_notes; ++i) {
        const NoteEntry& chord_note = pressed_keys_.played_note(i);
        generated_notes_.NoteOn(chord_note.note, chord_note.velocity & 0x7f);
        InternalNoteOn(chord_note.note, chord_note.velocity & 0x7f);
      }
    }
    arp_note_ += arp_direction_;
    gate_length_counter_ = seq_.gate_length;
  }
  
  ++arp_step_;
  if (arp_step_ >= pattern_length) {
    arp_step_ = 0;
  }
}

void Part::ResetAllControllers() {
  ignore_note_off_messages_ = false;
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i]->ResetAllControllers();
  }
}

void Part::AllNotesOff() {
  poly_allocator_.ClearNotes();
  mono_allocator_.Clear();
  pressed_keys_.Clear();
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i]->NoteOff();
  }
  std::fill(
      &active_note_[0],
      &active_note_[kMaxNumVoices],
      VOICE_ALLOCATION_NOT_FOUND);
  release_latched_keys_on_next_note_on_ = false;
  ignore_note_off_messages_ = false;
}

void Part::ReleaseLatchedNotes() {
  for (uint8_t i = 1; i <= pressed_keys_.max_size(); ++i) {
    NoteEntry* e = pressed_keys_.mutable_note(i);
    if (e->velocity & 0x80) {
      NoteOff(tx_channel(), e->note);
    }
  }
}

void Part::DispatchSortedNotes(bool unison) {
  uint8_t n = mono_allocator_.size();
  for (uint8_t i = 0; i < num_voices_; ++i) {
    uint8_t index = 0xff;
    if (unison) {
      index = n ? (i * n / num_voices_) : 0xff;
    } else {
      index = i < mono_allocator_.size() ? i : 0xff;
    }
    if (index != 0xff) {
      voice_[i]->NoteOn(
          Tune(mono_allocator_.sorted_note(index).note),
          mono_allocator_.sorted_note(index).velocity,
          voicing_.portamento,
          !voice_[i]->gate_on());
      active_note_[i] = mono_allocator_.sorted_note(index).note;
    } else {
      voice_[i]->NoteOff();
      active_note_[i] = VOICE_ALLOCATION_NOT_FOUND;
    }
  }
}

void Part::InternalNoteOn(uint8_t note, uint8_t velocity) {
  if (midi_.out_mode == MIDI_OUT_MODE_GENERATED_EVENTS && !polychained_) {
    midi_handler.OnInternalNoteOn(tx_channel(), note, velocity);
  }
  
  if (voicing_.allocation_mode == VOICE_ALLOCATION_MODE_MONO) {
    const NoteEntry& before = mono_allocator_.note_by_priority(
        static_cast<NoteStackFlags>(voicing_.allocation_priority));
    mono_allocator_.NoteOn(note, velocity);
    const NoteEntry& after = mono_allocator_.note_by_priority(
        static_cast<NoteStackFlags>(voicing_.allocation_priority));
    // Check if the note that has been played should be triggered according
    // to selected voice priority rules.
    if (before.note != after.note) {
      bool legato = mono_allocator_.size() > 1;
      for (uint8_t i = 0; i < num_voices_; ++i) {
        voice_[i]->NoteOn(
            Tune(after.note),
            after.velocity,
            !voicing_.legato_mode || legato ? voicing_.portamento : 0,
            !voicing_.legato_mode || !legato);
      }
    }
  } else if (voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_SORTED ||
             voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_UNISON_1 ||
             voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_UNISON_2) {
    mono_allocator_.NoteOn(note, velocity);
    DispatchSortedNotes(
        voicing_.allocation_mode != VOICE_ALLOCATION_MODE_POLY_SORTED);
  } else {
    uint8_t voice_index = 0;
    switch (voicing_.allocation_mode) {
      case VOICE_ALLOCATION_MODE_POLY:
        voice_index = poly_allocator_.NoteOn(note);
        break;
        
      case VOICE_ALLOCATION_MODE_POLY_CYCLIC:
        if (cyclic_allocation_note_counter_ >= num_voices_) {
          cyclic_allocation_note_counter_ = 0;
        }
        voice_index = cyclic_allocation_note_counter_;
        ++cyclic_allocation_note_counter_;
        break;
        
      case VOICE_ALLOCATION_MODE_POLY_RANDOM:
        voice_index = (Random::GetWord() >> 24) % num_voices_;
        break;
        
      case VOICE_ALLOCATION_MODE_POLY_VELOCITY:
        voice_index = (static_cast<uint16_t>(velocity) * num_voices_) >> 7;
        break;
        
      default:
        break;
    }
    
    if (voice_index < num_voices_) {
      // Prevent the same note from being simultaneously played on two channels.
      KillAllInstancesOfNote(note);
      voice_[voice_index]->NoteOn(
          Tune(note),
          velocity,
          voicing_.portamento,
          true);
      active_note_[voice_index] = note;
    } else {
      // Polychaining forwarding.
      midi_handler.OnInternalNoteOn(tx_channel(), note, velocity);
    }
  }
}

void Part::KillAllInstancesOfNote(uint8_t note) {
  while (true) {
    uint8_t index = FindVoiceForNote(note);
    if (index != VOICE_ALLOCATION_NOT_FOUND) {
      voice_[index]->NoteOff();
      active_note_[index] = VOICE_ALLOCATION_NOT_FOUND;
    } else {
      break;
    }
  }
}

void Part::InternalNoteOff(uint8_t note) {
  if (midi_.out_mode == MIDI_OUT_MODE_GENERATED_EVENTS && !polychained_) {
    midi_handler.OnInternalNoteOff(tx_channel(), note);
  }
  
  if (voicing_.tuning_system == TUNING_SYSTEM_JUST_INTONATION) {
    just_intonation_processor.NoteOff(note);
  }
  
  if (voicing_.allocation_mode == VOICE_ALLOCATION_MODE_MONO) {
    const NoteEntry& before = mono_allocator_.note_by_priority(
        static_cast<NoteStackFlags>(voicing_.allocation_priority));
    mono_allocator_.NoteOff(note);
    const NoteEntry& after = mono_allocator_.note_by_priority(
        static_cast<NoteStackFlags>(voicing_.allocation_priority));
    if (mono_allocator_.size() == 0) {
      // No key is pressed, we just close the gate.
      for (uint8_t i = 0; i < num_voices_; ++i) {
        voice_[i]->NoteOff();
      }
    } else if (before.note != after.note) {
      // Removing the note gives priority to another note that is still being
      // pressed. Slide to this note (or retrigger is legato mode is off).
      for (uint8_t i = 0; i < num_voices_; ++i) {
        voice_[i]->NoteOn(
            Tune(after.note),
            after.velocity,
            voicing_.portamento,
            !voicing_.legato_mode);
      }
    }
  } else if (voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_SORTED ||
             voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_UNISON_1 ||
             voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_UNISON_2) {
    mono_allocator_.NoteOff(note);
    KillAllInstancesOfNote(note);
    if (voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY_UNISON_1) {
      DispatchSortedNotes(true);
    }
  } else {
    uint8_t voice_index = \
        voicing_.allocation_mode == VOICE_ALLOCATION_MODE_POLY ? \
        poly_allocator_.NoteOff(note) : \
        FindVoiceForNote(note);
    if (voice_index < num_voices_) {
      voice_[voice_index]->NoteOff();
      active_note_[voice_index] = VOICE_ALLOCATION_NOT_FOUND;
    } else {
       midi_handler.OnInternalNoteOff(tx_channel(), note);
    }
  }
}

void Part::TouchVoiceAllocation() {
  AllNotesOff();
  ResetAllControllers();
}

void Part::TouchVoices() {
  CONSTRAIN(voicing_.aux_cv, 0, 7);
  CONSTRAIN(voicing_.aux_cv_2, 0, 7);
  for (uint8_t i = 0; i < num_voices_; ++i) {
    voice_[i]->set_pitch_bend_range(voicing_.pitch_bend_range);
    voice_[i]->set_modulation_rate(voicing_.modulation_rate);
    voice_[i]->set_vibrato_range(voicing_.vibrato_range);
    voice_[i]->set_trigger_duration(voicing_.trigger_duration);
    voice_[i]->set_trigger_scale(voicing_.trigger_scale);
    voice_[i]->set_trigger_shape(voicing_.trigger_shape);
    voice_[i]->set_aux_cv(voicing_.aux_cv);
    voice_[i]->set_aux_cv_2(voicing_.aux_cv_2);
    voice_[i]->set_audio_mode(voicing_.audio_mode);
    voice_[i]->set_tuning(voicing_.tuning_transpose, voicing_.tuning_fine);
  }
}

void Part::Set(uint8_t address, uint8_t value) {
  uint8_t* bytes;
  bytes = static_cast<uint8_t*>(static_cast<void*>(&midi_));
  uint8_t previous_value = bytes[address];
  bytes[address] = value;
  if (value != previous_value) {
    switch (address) {
      case PART_MIDI_CHANNEL:
      case PART_MIDI_MIN_NOTE:
      case PART_MIDI_MAX_NOTE:
      case PART_MIDI_MIN_VELOCITY:
      case PART_MIDI_MAX_VELOCITY:
        // Shut all channels off when a MIDI parameter is changed to prevent
        // stuck notes.
        AllNotesOff();
        break;
        
      case PART_VOICING_ALLOCATION_MODE:
        TouchVoiceAllocation();
        break;
        
      case PART_VOICING_PITCH_BEND_RANGE:
      case PART_VOICING_MODULATION_RATE:
      case PART_VOICING_VIBRATO_RANGE:
      case PART_VOICING_TRIGGER_DURATION:
      case PART_VOICING_TRIGGER_SHAPE:
      case PART_VOICING_TRIGGER_SCALE:
      case PART_VOICING_AUX_CV:
      case PART_VOICING_AUX_CV_2:
      case PART_VOICING_AUDIO_MODE:
      case PART_VOICING_TUNING_TRANSPOSE:
      case PART_VOICING_TUNING_FINE:
        TouchVoices();
        break;
        
      case PART_SEQUENCER_ARP_DIRECTION:
        arp_direction_ = \
            seq_.arp_direction == ARPEGGIATOR_DIRECTION_DOWN ? -1 : 1;
        break;
    }
  }
}

int16_t Part::Tune(int16_t midi_note) {
  int16_t note = midi_note;
  int16_t pitch = note << 7;
  uint8_t pitch_class = (note + 240) % 12;

  // Just intonation.
  if (voicing_.tuning_system == TUNING_SYSTEM_JUST_INTONATION) {
    pitch = just_intonation_processor.NoteOn(note);
  } else if (voicing_.tuning_system == TUNING_SYSTEM_CUSTOM) {
    pitch += custom_pitch_table_[pitch_class];
  } else if (voicing_.tuning_system > TUNING_SYSTEM_JUST_INTONATION) {
    note -= voicing_.tuning_root;
    pitch_class = (note + 240) % 12;
    pitch += lookup_table_signed_table[LUT_SCALE_PYTHAGOREAN + \
        voicing_.tuning_system - TUNING_SYSTEM_PYTHAGOREAN][pitch_class];
  }
  
  return pitch;
}

}  // namespace yarns
