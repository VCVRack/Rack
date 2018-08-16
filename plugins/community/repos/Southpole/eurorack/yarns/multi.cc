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
// Multi.

#include "yarns/multi.h"

#include <algorithm>

#include "stmlib/algorithms/voice_allocator.h"

#include "yarns/just_intonation_processor.h"
#include "yarns/midi_handler.h"
#include "yarns/settings.h"

namespace yarns {
  
using namespace std;
using namespace stmlib;

const uint8_t clock_divisions[] = {
  96, 48, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1
};

void Multi::Init() {
  just_intonation_processor.Init();
  
  fill(
      &settings_.custom_pitch_table[0],
      &settings_.custom_pitch_table[12],
      0);
  
  for (uint8_t i = 0; i < kNumParts; ++i) {
    part_[i].Init();
    part_[i].set_custom_pitch_table(settings_.custom_pitch_table);
  }
  for (uint8_t i = 0; i < kNumVoices; ++i) {
    voice_[i].Init();
  }
  running_ = false;
  latched_ = false;
  recording_ = false;
  
  // Put the multi in a usable state. Even if these settings will later be
  // overriden with some data retrieved from Flash (presets).
  settings_.clock_tempo = 120;
  settings_.clock_swing = 0;
  settings_.clock_input_division = 1;
  settings_.clock_output_division = 7;
  settings_.clock_bar_duration = 4;
  settings_.clock_override = 0;
  
  MidiSettings* midi = part_[0].mutable_midi_settings();
  midi->channel = 0;
  midi->min_note = 0;
  midi->max_note = 127;
  midi->min_velocity = 0;
  midi->max_velocity = 127;
  midi->out_mode = MIDI_OUT_MODE_GENERATED_EVENTS;
  
  VoicingSettings* voicing = part_[0].mutable_voicing_settings();
  voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
  voicing->allocation_mode = VOICE_ALLOCATION_MODE_MONO;
  voicing->legato_mode = false;
  voicing->portamento = 0;
  voicing->pitch_bend_range = 2;
  voicing->vibrato_range = 1;
  voicing->modulation_rate = 50;
  voicing->trigger_duration = 2;
  voicing->aux_cv = 1;
  voicing->aux_cv_2 = 6;
  voicing->tuning_transpose = 0;
  voicing->tuning_fine = 0;
  voicing->tuning_root = 0;
  voicing->tuning_system = 0;
  voicing->audio_mode = 0;

  SequencerSettings* seq = part_[0].mutable_sequencer_settings();
  seq->clock_division = 7;
  seq->gate_length = 3;
  seq->arp_range = 0;
  seq->arp_direction = 0;
  seq->arp_pattern = 0;
  
  fill(
      &seq->step[0],
      &seq->step[kNumSteps],
      SequencerStep(SEQUENCER_STEP_REST, 0));
  seq->num_steps = 0;
  // A test sequence...
  // seq->num_steps = 4;
  // seq->step[0].data[0] = 48;
  // seq->step[0].data[1] = 0x7f;
  // seq->step[1].data[0] = 72;
  // seq->step[1].data[1] = 0x7f;
  // seq->step[2].data[0] = 60;
  // seq->step[2].data[1] = 0x7f;
  // seq->step[3].data[0] = 72;
  // seq->step[3].data[1] = 0x7f;
  // voicing->audio_mode = 1;  // To hear it...

  num_active_parts_ = 1;
  part_[0].AllocateVoices(&voice_[0], 1, false);
  settings_.layout = LAYOUT_MONO;
}

void Multi::Clock() {
  if (!running_) {
    return;
  }
  
  uint16_t output_division = clock_divisions[settings_.clock_output_division];
  uint16_t input_division = settings_.clock_input_division;
  
  // Logic equation for computing a clock output with a 50% duty cycle.
  if (output_division > 1) {
    if (clock_output_prescaler_ == 0 && clock_input_prescaler_ == 0) {
      clock_pulse_counter_ = 0xffff;
    }
    if (clock_output_prescaler_ == (output_division >> 1) &&
        clock_input_prescaler_ == (input_division >> 1)) {
      clock_pulse_counter_ = 0;
    }
  } else {
    if (input_division > 1) {
      clock_pulse_counter_ = \
          clock_input_prescaler_ <= (input_division - 1) >> 1 ? 0xffff : 0;
    } else {
      // Because no division is used, neither on the output nor on the input,
      // we don't have a sufficient fast time base to derive a 50% duty cycle
      // output. Instead, we output 5ms pulses.
      clock_pulse_counter_ = 40;
    }
  }
  
  if (!clock_input_prescaler_) {
    midi_handler.OnClock();
    
    if (song_pointer_) {
      ClockSong();
    } else {
      for (uint8_t i = 0; i < num_active_parts_; ++i) {
        part_[i].Clock();
      }
    }
    
    ++bar_position_;
    if (bar_position_ >= settings_.clock_bar_duration * 24) {
      bar_position_ = 0;
    }
    if (bar_position_ == 0) {
      reset_pulse_counter_ = 81;
    }
    if (settings_.clock_bar_duration > kMaxBarDuration) {
      bar_position_ = 1;
    }
    
    ++clock_output_prescaler_;
    if (clock_output_prescaler_ >= output_division) {
      clock_output_prescaler_ = 0;
    }
  }

  ++clock_input_prescaler_;
  if (clock_input_prescaler_ >= settings_.clock_input_division) {
    clock_input_prescaler_ = 0;
  }
  
  if (stop_count_down_) {
    --stop_count_down_;
    if (!stop_count_down_ && started_by_keyboard_ && internal_clock()) {
      Stop();
    }
  }
}

void Multi::Start(bool started_by_keyboard) {
  if (running_ || recording_) {
    return;
  }
  if (internal_clock()) {
    internal_clock_ticks_ = 0;
    internal_clock_.Start(settings_.clock_tempo, settings_.clock_swing);
  }
  midi_handler.OnStart();

  started_by_keyboard_ = started_by_keyboard;
  running_ = true;
  latched_ = false;
  clock_input_prescaler_ = 0;
  clock_output_prescaler_ = 0;
  stop_count_down_ = 0;
  bar_position_ = 0xffff;
  for (uint8_t i = 0; i < num_active_parts_; ++i) {
    part_[i].Start(started_by_keyboard);
  }
  song_pointer_ = NULL;
}

void Multi::Stop() {
  if (!running()) {
    return;
  }
  for (uint8_t i = 0; i < num_active_parts_; ++i) {
    part_[i].Stop();
  }
  midi_handler.OnStop();
  clock_pulse_counter_ = 0;
  reset_pulse_counter_ = 0;
  stop_count_down_ = 0;
  running_ = false;
  latched_ = false;
  started_by_keyboard_ = false;
  song_pointer_ = NULL;
}

void Multi::Refresh() {
  if (clock_pulse_counter_) {
    --clock_pulse_counter_;
  }
  if (reset_pulse_counter_) {
    --reset_pulse_counter_;
  }
  
  for (uint8_t i = 0; i < kNumVoices; ++i) {
    voice_[i].Refresh();
  }
}

void Multi::Set(uint8_t address, uint8_t value) {
  uint8_t* bytes;
  bytes = static_cast<uint8_t*>(static_cast<void*>(&settings_));
  uint8_t previous_value = bytes[address];
  bytes[address] = value;
  if (value != previous_value) {
    if (address == MULTI_LAYOUT) {
      ChangeLayout(
          static_cast<Layout>(previous_value),
          static_cast<Layout>(value));
    } else if (address == MULTI_CLOCK_TEMPO) {
      internal_clock_.set_tempo(settings_.clock_tempo);
    } else if (address == MULTI_CLOCK_SWING) {
      internal_clock_.set_swing(settings_.clock_swing);
    }
  }
}

void Multi::GetCvGate(uint16_t* cv, bool* gate) {
  switch (settings_.layout) {
    case LAYOUT_MONO:
    case LAYOUT_DUAL_POLYCHAINED:
      cv[0] = voice_[0].note_dac_code();
      cv[1] = voice_[0].velocity_dac_code();
      cv[2] = voice_[0].aux_cv_dac_code();
      cv[3] = voice_[0].aux_cv_dac_code_2();
      gate[0] = voice_[0].gate();
      gate[1] = voice_[0].trigger();
      gate[2] = clock();
      gate[3] = reset_or_playing_flag();
      break;
      
    case LAYOUT_DUAL_MONO:
      cv[0] = voice_[0].note_dac_code();
      cv[1] = voice_[1].note_dac_code();
      cv[2] = voice_[0].aux_cv_dac_code();
      cv[3] = voice_[1].aux_cv_dac_code();
      gate[0] = voice_[0].gate();
      gate[1] = voice_[1].gate();
      gate[2] = clock();
      gate[3] = reset_or_playing_flag();
      break;
    
    case LAYOUT_DUAL_POLY:
    case LAYOUT_QUAD_POLYCHAINED:
      cv[0] = voice_[0].note_dac_code();
      cv[1] = voice_[1].note_dac_code();
      cv[2] = voice_[0].aux_cv_dac_code();
      cv[3] = voice_[1].aux_cv_dac_code_2();
      gate[0] = voice_[0].gate();
      gate[1] = voice_[1].gate();
      gate[2] = clock();
      gate[3] = reset_or_playing_flag();
      break;
    
    case LAYOUT_QUAD_MONO:
    case LAYOUT_QUAD_POLY:
    case LAYOUT_OCTAL_POLYCHAINED:
    case LAYOUT_THREE_ONE:
      cv[0] = voice_[0].note_dac_code();
      cv[1] = voice_[1].note_dac_code();
      cv[2] = voice_[2].note_dac_code();
      cv[3] = voice_[3].note_dac_code();
      gate[0] = voice_[0].gate();
      gate[1] = voice_[1].gate();
      if (settings_.clock_override) {
        gate[2] = clock();
        gate[3] = reset_or_playing_flag();
      } else {
        gate[2] = voice_[2].gate();
        gate[3] = voice_[3].gate();
      }
      break;
    
    case LAYOUT_QUAD_TRIGGERS:
      cv[0] = voice_[0].trigger_dac_code();
      cv[1] = voice_[1].trigger_dac_code();
      cv[2] = voice_[2].trigger_dac_code();
      cv[3] = voice_[3].trigger_dac_code();
      gate[0] = voice_[0].trigger() && ~voice_[1].gate();
      gate[1] = voice_[0].trigger() && voice_[1].gate();
      gate[2] = clock();
      gate[3] = reset_or_playing_flag();
      break;

    case LAYOUT_QUAD_VOLTAGES:
      cv[0] = voice_[0].aux_cv_dac_code();
      cv[1] = voice_[1].aux_cv_dac_code();
      cv[2] = voice_[2].aux_cv_dac_code();
      cv[3] = voice_[3].aux_cv_dac_code();
      gate[0] = voice_[0].gate();
      gate[1] = voice_[1].gate();
      if (settings_.clock_override) {
        gate[2] = clock();
        gate[3] = reset_or_playing_flag();
      } else {
        gate[2] = voice_[2].gate();
        gate[3] = voice_[3].gate();
      }
      break;
  }
}

bool Multi::GetAudioSource(uint8_t* audio_source) {
  bool has_audio_source = false;
  switch (settings_.layout) {
    case LAYOUT_MONO:
    case LAYOUT_DUAL_POLYCHAINED:
      audio_source[0] = 0xff;
      audio_source[1] = 0xff;
      audio_source[2] = 0xff;
      audio_source[3] = voice_[0].audio_mode() ? 0 : 0xff;
      has_audio_source = voice_[0].audio_mode();
      break;
      
    case LAYOUT_DUAL_MONO:
    case LAYOUT_DUAL_POLY:
    case LAYOUT_QUAD_POLYCHAINED:
      audio_source[0] = 0xff;
      audio_source[1] = 0xff;
      audio_source[2] = voice_[0].audio_mode() ? 0 : 0xff;
      audio_source[3] = voice_[1].audio_mode() ? 1 : 0xff;
      has_audio_source = voice_[0].audio_mode() || voice_[1].audio_mode();
      break;
      
    case LAYOUT_QUAD_MONO:
    case LAYOUT_QUAD_POLY:
    case LAYOUT_OCTAL_POLYCHAINED:
    case LAYOUT_THREE_ONE:
      audio_source[0] = voice_[0].audio_mode() ? 0 : 0xff;
      audio_source[1] = voice_[1].audio_mode() ? 1 : 0xff;
      audio_source[2] = voice_[2].audio_mode() ? 2 : 0xff;
      audio_source[3] = voice_[3].audio_mode() ? 3 : 0xff;
      has_audio_source = voice_[0].audio_mode() || voice_[1].audio_mode() || \
          voice_[2].audio_mode() || voice_[3].audio_mode();
      break;
    
    case LAYOUT_QUAD_TRIGGERS:
    case LAYOUT_QUAD_VOLTAGES:
      audio_source[0] = 0xff;
      audio_source[1] = 0xff;
      audio_source[2] = 0xff;
      audio_source[3] = 0xff;
      has_audio_source = false;
      break;
  }
  return has_audio_source;
}

void Multi::GetLedsBrightness(uint8_t* brightness) {
  if (layout_configurator_.learning()) {
    fill(&brightness[0], &brightness[kNumVoices], 0);
    for (uint8_t i = 0; i < layout_configurator_.num_notes(); ++i) {
      brightness[i] = 255;
    }
    return;
  }
  
  switch (settings_.layout) {
    case LAYOUT_MONO:
    case LAYOUT_DUAL_POLYCHAINED:
      brightness[0] = voice_[0].gate() ? 255 : 0;
      brightness[1] = voice_[0].velocity() << 1;
      brightness[2] = voice_[0].aux_cv();
      brightness[3] = voice_[0].aux_cv_2();
      break;
      
    case LAYOUT_DUAL_MONO:
      brightness[0] = voice_[0].gate() ? 255 : 0;
      brightness[1] = voice_[1].gate() ? 255 : 0;
      brightness[2] = voice_[0].aux_cv();
      brightness[3] = voice_[1].aux_cv();
      break;

    case LAYOUT_DUAL_POLY:
    case LAYOUT_QUAD_POLYCHAINED:
      brightness[0] = voice_[0].gate() ? 255 : 0;
      brightness[1] = voice_[1].gate() ? 255 : 0;
      brightness[2] = voice_[0].aux_cv();
      brightness[3] = voice_[1].aux_cv_2();
      break;
      
    case LAYOUT_QUAD_MONO:
    case LAYOUT_QUAD_POLY:
    case LAYOUT_OCTAL_POLYCHAINED:
    case LAYOUT_QUAD_TRIGGERS:
    case LAYOUT_THREE_ONE:
      brightness[0] = voice_[0].gate() ? (voice_[0].velocity() << 1) : 0;
      brightness[1] = voice_[1].gate() ? (voice_[1].velocity() << 1) : 0;
      brightness[2] = voice_[2].gate() ? (voice_[2].velocity() << 1) : 0;
      brightness[3] = voice_[3].gate() ? (voice_[3].velocity() << 1) : 0;
      break;
      
    case LAYOUT_QUAD_VOLTAGES:
      brightness[0] = voice_[0].aux_cv();
      brightness[1] = voice_[1].aux_cv();
      brightness[2] = voice_[2].aux_cv();
      brightness[3] = voice_[3].aux_cv();
      break;
  }
}

void Multi::UpdateLayout() {
  // Reset and close all parts and voices.
  for (uint8_t i = 0; i < kNumParts; ++i) {
    part_[i].Reset();
  }
  for (uint8_t i = 0; i < kNumVoices; ++i) {
    voice_[i].NoteOff();
  }
  
  switch (settings_.layout) {
    case LAYOUT_MONO:
    case LAYOUT_DUAL_MONO:
    case LAYOUT_QUAD_MONO:
      {
        uint8_t num_parts = settings_.layout == LAYOUT_MONO ? 1 : \
            (settings_.layout == LAYOUT_DUAL_MONO ? 2 : 4);
        for (uint8_t i = 0; i < num_parts; ++i) {
          part_[i].AllocateVoices(&voice_[i], 1, false);
        }
        num_active_parts_ = num_parts;
      }
      break;
    
    case LAYOUT_DUAL_POLY:
    case LAYOUT_QUAD_POLY:
    case LAYOUT_DUAL_POLYCHAINED:
    case LAYOUT_QUAD_POLYCHAINED:
    case LAYOUT_OCTAL_POLYCHAINED:
      {
        uint8_t num_voices = settings_.layout == LAYOUT_DUAL_POLY || \
            settings_.layout == LAYOUT_QUAD_POLYCHAINED ? 2 : \
            (settings_.layout == LAYOUT_DUAL_POLYCHAINED ? 1 : 4);
        part_[0].AllocateVoices(
            &voice_[0],
            num_voices,
            settings_.layout >= LAYOUT_DUAL_POLYCHAINED);
        num_active_parts_ = 1;
      }
      break;
      
    case LAYOUT_QUAD_TRIGGERS:
    case LAYOUT_QUAD_VOLTAGES:
      {
        for (uint8_t i = 0; i < 4; ++i) {
          part_[i].AllocateVoices(&voice_[i], 1, false);
        }
        num_active_parts_ = 4;
      }
      break;
      
    case LAYOUT_THREE_ONE:
      {
        part_[0].AllocateVoices(&voice_[0], 3, false);
        part_[1].AllocateVoices(&voice_[3], 1, false);
        num_active_parts_ = 2;
      }
      break;
    
    default:
      break;
  }
}

void Multi::ChangeLayout(Layout old_layout, Layout new_layout) {
  // Reset and close all parts and voices.
  for (uint8_t i = 0; i < kNumParts; ++i) {
    part_[i].Reset();
  }
  for (uint8_t i = 0; i < kNumVoices; ++i) {
    voice_[i].NoteOff();
  }
  
  switch (new_layout) {
    case LAYOUT_MONO:
    case LAYOUT_DUAL_MONO:
    case LAYOUT_QUAD_MONO:
      {
        uint8_t num_parts = new_layout == LAYOUT_MONO ? 1 : \
            (new_layout == LAYOUT_DUAL_MONO ? 2 : 4);

        for (uint8_t i = 0; i < num_parts; ++i) {
          MidiSettings* midi = part_[i].mutable_midi_settings();
          if (old_layout == LAYOUT_QUAD_TRIGGERS) {
            midi->min_note = 0;
            midi->max_note = 127;
          }
          midi->min_velocity = 0;
          midi->max_velocity = 127;

          VoicingSettings* voicing = part_[i].mutable_voicing_settings();
          voicing->allocation_mode = VOICE_ALLOCATION_MODE_MONO;
          voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
        }
        
        // Duplicate uninitialized voices.
        for (uint8_t i = 1; i < num_parts; ++i) {
          uint8_t destination = i;
          uint8_t source = i % num_active_parts_;
          if (destination != source) {
            memcpy(
                part_[destination].mutable_midi_settings(),
                part_[source].mutable_midi_settings(),
                sizeof(MidiSettings));
            memcpy(
                part_[destination].mutable_voicing_settings(),
                part_[source].mutable_voicing_settings(),
                sizeof(VoicingSettings));
            memcpy(
                part_[destination].mutable_sequencer_settings(),
                part_[source].mutable_sequencer_settings(),
                sizeof(SequencerSettings));
          }
        }
        
        for (uint8_t i = 0; i < num_parts; ++i) {
          part_[i].AllocateVoices(&voice_[i], 1, false);
        }
        num_active_parts_ = num_parts;
      }
      break;
    
    case LAYOUT_DUAL_POLY:
    case LAYOUT_QUAD_POLY:
    case LAYOUT_DUAL_POLYCHAINED:
    case LAYOUT_QUAD_POLYCHAINED:
    case LAYOUT_OCTAL_POLYCHAINED:
      {
        uint8_t num_voices = settings_.layout == LAYOUT_DUAL_POLY || \
            settings_.layout == LAYOUT_QUAD_POLYCHAINED ? 2 : \
            (settings_.layout == LAYOUT_DUAL_POLYCHAINED ? 1 : 4);
        
        MidiSettings* midi = part_[0].mutable_midi_settings();
        if (old_layout == LAYOUT_QUAD_TRIGGERS) {
          midi->min_note = 0;
          midi->max_note = 127;
        }
        midi->min_velocity = 0;
        midi->max_velocity = 127;

        VoicingSettings* voicing = part_[0].mutable_voicing_settings();
        voicing->allocation_mode = VOICE_ALLOCATION_MODE_POLY;
        voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
        voicing->portamento = 0;
        voicing->legato_mode = 0;
        
        part_[0].AllocateVoices(
            &voice_[0],
            num_voices,
            new_layout >= LAYOUT_DUAL_POLYCHAINED);
        num_active_parts_ = 1;
      }
      break;
      
    case LAYOUT_QUAD_TRIGGERS:
      {
        for (uint8_t i = 0; i < 4; ++i) {
          MidiSettings* midi = part_[i].mutable_midi_settings();
          if (old_layout != LAYOUT_QUAD_TRIGGERS) {
            midi->min_note = 36 + i * 2;
            midi->max_note = 36 + i * 2;
          }
          midi->min_velocity = 0;
          midi->max_velocity = 127;
          midi->channel = part_[0].mutable_midi_settings()->channel;
          midi->out_mode = part_[0].mutable_midi_settings()->out_mode;
          
          VoicingSettings* voicing = part_[i].mutable_voicing_settings();
          voicing->allocation_mode = VOICE_ALLOCATION_MODE_MONO;
          voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
          voicing->portamento = 0;
          voicing->legato_mode = 0;
        }
        
        // Duplicate sequencer settings.
        for (uint8_t i = 1; i < 4; ++i) {
          uint8_t destination = i;
          uint8_t source = i % num_active_parts_;
          if (destination != source) {
            memcpy(
                part_[destination].mutable_sequencer_settings(),
                part_[source].mutable_sequencer_settings(),
                sizeof(SequencerSettings));
          }
        }
        
        for (uint8_t i = 0; i < 4; ++i) {
          part_[i].AllocateVoices(&voice_[i], 1, false);
        }
        num_active_parts_ = 4;
      }
      break;
    
    case LAYOUT_THREE_ONE:
      {
        MidiSettings* midi = part_[0].mutable_midi_settings();
        if (old_layout == LAYOUT_QUAD_TRIGGERS) {
          midi->min_note = 0;
          midi->max_note = 127;
        }
        midi->min_velocity = 0;
        midi->max_velocity = 127;

        VoicingSettings* voicing = part_[0].mutable_voicing_settings();
        voicing->allocation_mode = VOICE_ALLOCATION_MODE_POLY;
        voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
        voicing->portamento = 0;
        voicing->legato_mode = 0;
        part_[0].AllocateVoices(&voice_[0], 3, false);

        midi = part_[1].mutable_midi_settings();
        if (old_layout == LAYOUT_QUAD_TRIGGERS) {
          midi->min_note = 0;
          midi->max_note = 127;
        }
        midi->min_velocity = 0;
        midi->max_velocity = 127;

        voicing = part_[1].mutable_voicing_settings();
        voicing->allocation_mode = VOICE_ALLOCATION_MODE_MONO;
        voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
        voicing->portamento = 0;
        voicing->legato_mode = 0;
        part_[1].AllocateVoices(&voice_[3], 1, false);

        num_active_parts_ = 2;
      }
      break;
    
    case LAYOUT_QUAD_VOLTAGES:
      {
        uint8_t num_parts = 4;
        for (uint8_t i = 0; i < num_parts; ++i) {
          MidiSettings* midi = part_[i].mutable_midi_settings();
          if (old_layout == LAYOUT_QUAD_TRIGGERS) {
            midi->min_note = 0;
            midi->max_note = 127;
          }
          midi->min_velocity = 0;
          midi->max_velocity = 127;
          VoicingSettings* voicing = part_[i].mutable_voicing_settings();
          voicing->allocation_mode = VOICE_ALLOCATION_MODE_MONO;
          voicing->allocation_priority = NOTE_STACK_PRIORITY_LAST;
        }
      
        // Duplicate uninitialized voices.
        for (uint8_t i = 1; i < num_parts; ++i) {
          uint8_t destination = i;
          uint8_t source = i % num_active_parts_;
          if (destination != source) {
            memcpy(
                part_[destination].mutable_midi_settings(),
                part_[source].mutable_midi_settings(),
                sizeof(MidiSettings));
            memcpy(
                part_[destination].mutable_voicing_settings(),
                part_[source].mutable_voicing_settings(),
                sizeof(VoicingSettings));
            memcpy(
                part_[destination].mutable_sequencer_settings(),
                part_[source].mutable_sequencer_settings(),
                sizeof(SequencerSettings));
          }
        }
        for (uint8_t i = 0; i < num_parts; ++i) {
          part_[i].AllocateVoices(&voice_[i], 1, false);
        }
        num_active_parts_ = num_parts;
      }
      break;
    
    default:
      break;
  }
  
  for (uint8_t i = 0; i < num_active_parts_; ++i) {
    part_[i].set_siblings(num_active_parts_ > 1);
  }
}

void Multi::Touch() {
  Stop();

  internal_clock_.set_tempo(settings_.clock_tempo);
  UpdateLayout();
  
  for (uint8_t i = 0; i < kNumParts; ++i) {
    part_[i].Touch();
  }
}


const uint8_t song[] = {
  #include "song/song.h"
  255,
};

void Multi::StartSong() {
  Set(MULTI_LAYOUT, LAYOUT_QUAD_MONO);
  part_[0].mutable_voicing_settings()->audio_mode = 0x83;
  part_[1].mutable_voicing_settings()->audio_mode = 0x83;
  part_[2].mutable_voicing_settings()->audio_mode = 0x84;
  part_[3].mutable_voicing_settings()->audio_mode = 0x86;
  UpdateLayout();
  settings_.clock_tempo = 140;
  Stop();
  Start(false);
  
  song_pointer_ = &song[0];
  song_clock_ = 0;
  song_delta_ = 0;
}

void Multi::ClockSong() {
  while (song_clock_ >= song_delta_) {
    if (*song_pointer_ == 255) {
      song_pointer_ = &song[0];
    }
    if (*song_pointer_ == 254) {
      song_delta_ += 6;
    } else {
      uint8_t part = *song_pointer_ >> 6;
      uint8_t note = *song_pointer_ & 0x3f;
      if (note == 0) {
        part_[part].AllNotesOff();
      } else {
        part_[part].NoteOn(0, note + 24, 100);
      }
      song_clock_ = 0;
      song_delta_ = 0;
    }
    ++song_pointer_;
  }
  ++song_clock_;
}

bool Multi::ControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
  bool thru = true;
  
  if (channel + 1 == settings_.remote_control_channel) {
    yarns::settings.SetFromCC(0xff, controller, value);
    if (num_active_parts_ >= 4 && \
        (controller == 0x78 || controller == 0x79 || controller == 0x7b)) {
      // Do not continue to avoid treating these messages as "all sound off",
      // "reset all controllers" and "all notes off" CC.
      return true;
    }
  }
  
  for (uint8_t i = 0; i < num_active_parts_; ++i) {
    if (part_[i].accepts(channel) && \
        channel + 1 != settings_.remote_control_channel) {
      thru = part_[i].ControlChange(channel, controller, value) && thru;
      yarns::settings.SetFromCC(i, controller, value);
    }
  }
  return thru;
}


/* extern */
Multi multi;

}  // namespace yarns
