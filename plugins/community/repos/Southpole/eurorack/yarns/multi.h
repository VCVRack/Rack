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

#ifndef YARNS_MULTI_H_
#define YARNS_MULTI_H_

#include "stmlib/stmlib.h"

#include "yarns/internal_clock.h"
#include "yarns/layout_configurator.h"
#include "yarns/part.h"
#include "yarns/voice.h"

namespace yarns {

const uint8_t kNumParts = 4;
const uint8_t kNumVoices = 4;
const uint8_t kMaxBarDuration = 32;

struct MultiSettings {
  uint8_t layout;
  uint8_t clock_tempo;
  uint8_t clock_swing;
  uint8_t clock_input_division;
  uint8_t clock_output_division;
  uint8_t clock_bar_duration;
  uint8_t clock_override;
  int8_t custom_pitch_table[12];
  uint8_t remote_control_channel;
  uint8_t padding[12];
};

enum MultiSetting {
  MULTI_LAYOUT,
  MULTI_CLOCK_TEMPO,
  MULTI_CLOCK_SWING,
  MULTI_CLOCK_INPUT_DIVISION,
  MULTI_CLOCK_OUTPUT_DIVISION,
  MULTI_CLOCK_BAR_DURATION,
  MULTI_CLOCK_OVERRIDE,
  MULTI_PITCH_1,
  MULTI_PITCH_2,
  MULTI_PITCH_3,
  MULTI_PITCH_4,
  MULTI_PITCH_5,
  MULTI_PITCH_6,
  MULTI_PITCH_7,
  MULTI_PITCH_8,
  MULTI_PITCH_9,
  MULTI_PITCH_10,
  MULTI_PITCH_11,
  MULTI_PITCH_12,
  MULTI_REMOTE_CONTROL_CHANNEL
};

enum Layout {
  LAYOUT_MONO,
  LAYOUT_DUAL_MONO,
  LAYOUT_QUAD_MONO,
  LAYOUT_DUAL_POLY,
  LAYOUT_QUAD_POLY,
  LAYOUT_DUAL_POLYCHAINED,
  LAYOUT_QUAD_POLYCHAINED,
  LAYOUT_OCTAL_POLYCHAINED,
  LAYOUT_QUAD_TRIGGERS,
  LAYOUT_QUAD_VOLTAGES,
  LAYOUT_THREE_ONE,
  LAYOUT_LAST
};

class Multi {
 public:
  Multi() { }
  ~Multi() { }
  
  void Init();
  
  inline uint8_t paques() const {
    return settings_.clock_tempo == 49 && \
        settings_.clock_swing == 49 && \
        settings_.clock_output_division == 3 && \
        settings_.clock_bar_duration == 9;
  }
  
  bool NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    layout_configurator_.RegisterNote(channel, note);

    bool thru = true;
    bool received = false;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (part_[i].accepts(channel, note, velocity)) {
        received = true;
        thru = part_[i].NoteOn(channel, note, velocity) && thru;
      }
    }
    
    if (received && !running() && internal_clock()) {
      // Start the arpeggiators.
      Start(true);
    }
    
    stop_count_down_ = 0;
    
    return thru;
  }
  
  bool NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    bool thru = true;
    bool has_notes = false;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (part_[i].accepts(channel, note)) {
        thru = part_[i].NoteOff(channel, note) && thru;
      }
      has_notes = has_notes || part_[i].has_notes();
    }
    
    if (!has_notes && internal_clock() && started_by_keyboard_) {
      stop_count_down_ = 12;
    }
    
    return thru;
  }
  
  bool ControlChange(uint8_t channel, uint8_t controller, uint8_t value);

  bool PitchBend(uint8_t channel, uint16_t pitch_bend) {
    bool thru = true;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (part_[i].accepts(channel)) {
        thru = part_[i].PitchBend(channel, pitch_bend) && thru;
      }
    }
    return thru;
  }

  bool Aftertouch(uint8_t channel, uint8_t note, uint8_t velocity) {
    bool thru = true;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (part_[i].accepts(channel, note)) {
        thru = part_[i].Aftertouch(channel, note, velocity) && thru;
      }
    }
    return thru;
  }

  bool Aftertouch(uint8_t channel, uint8_t velocity) {
    bool thru = true;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (part_[i].accepts(channel)) {
        thru = part_[i].Aftertouch(channel, velocity) && thru;
      }
    }
    return thru;
  }
  
  void Reset() {
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      part_[i].Reset();
    }
  }
  
  void Clock();
  
  // A start initiated by a MIDI 0xfa event or the front panel start button will
  // start the sequencers. A start initiated by the keyboard will not start
  // the sequencers, and give priority to the arpeggiator. This allows the
  // arpeggiator to be played without erasing a sequence.
  void Start(bool started_by_keyboard);
  
  void Stop();
  
  void Continue() {
    Start(false);
  }
  
  void StartRecording(uint8_t part) {
    if (!recording_) {
      // Do not record while the arpeggiator is running!
      if (started_by_keyboard_ && running()) {
        Stop();
      }
      part_[part].StartRecording();
      recording_ = true;
    }
  }
  
  void StopRecording(uint8_t part) {
    if (recording_) {
      part_[part].StopRecording();
      recording_ = false;
    }
  }
  
  inline void Latch() {
    if (!latched_) {
      for (uint8_t i = 0; i < num_active_parts_; ++i) {
        part_[i].Latch();
      }
      latched_ = true;
    }
  }

  inline void Unlatch() {
    if (latched_) {
      for (uint8_t i = 0; i < num_active_parts_; ++i) {
        part_[i].Unlatch();
      }
      latched_ = false;
    }
  }
  
  void PushItNoteOn(uint8_t note) {
    uint8_t mask = recording_ ? 0x80 : 0;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (settings_.layout == LAYOUT_QUAD_TRIGGERS) {
        note = part_[i].midi_settings().min_note;
      }
      if (!recording_ || part_[i].recording()) {
        part_[i].NoteOn(part_[i].tx_channel() | mask, note, 127);
      }
    }
    if (!running() && internal_clock()) {
      // Start the arpeggiators.
      Start(true);
    }
  }
  
  void PushItNoteOff(uint8_t note) {
    uint8_t mask = recording_ ? 0x80 : 0;
    bool has_notes = false;
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (settings_.layout == LAYOUT_QUAD_TRIGGERS) {
        note = part_[i].midi_settings().min_note;
      }
      if (!recording_ || part_[i].recording()) {
        part_[i].NoteOff(part_[i].tx_channel() | mask, note);
      }
      has_notes = has_notes || part_[i].has_notes();
    }
    if (!has_notes && internal_clock()) {
      Stop();
    }
  }
  
  void Touch();
  void Refresh();
  void RefreshInternalClock() {
    if (running() && internal_clock() && internal_clock_.Process()) {
      ++internal_clock_ticks_;
    }
  }
  void ProcessInternalClockEvents() {
    while (internal_clock_ticks_) {
      Clock();
      --internal_clock_ticks_;
    }
  }

  inline void RenderAudio() {
    for (uint8_t i = 0; i < kNumVoices; ++i) {
      voice_[i].RenderAudio();
    }
  }
  
  void Set(uint8_t address, uint8_t value);
  inline uint8_t Get(uint8_t address) const {
    const uint8_t* bytes;
    bytes = static_cast<const uint8_t*>(static_cast<const void*>(&settings_));
    return bytes[address];
  }
  
  inline Layout layout() const { return static_cast<Layout>(settings_.layout); }
  inline bool internal_clock() const { return settings_.clock_tempo >= 40; }
  inline uint8_t tempo() const { return settings_.clock_tempo; }
  inline bool running() const { return running_; }
  inline bool latched() const { return latched_; }
  inline bool recording() const { return recording_; }
  inline bool clock() const { return clock_pulse_counter_ > 0; }
  inline bool reset() const {
    return reset_pulse_counter_ > 0;
  }
  inline bool reset_or_playing_flag() const {
    return reset() || ((settings_.clock_bar_duration == 0) && running_);
  }
  
  inline const Part& part(uint8_t index) const { return part_[index]; }
  inline const Voice& voice(uint8_t index) const { return voice_[index]; }
  inline const MultiSettings& settings() const { return settings_; }
  inline uint8_t num_active_parts() const { return num_active_parts_; }
  
  inline Voice* mutable_voice(uint8_t index) { return &voice_[index]; }
  inline Part* mutable_part(uint8_t index) { return &part_[index]; }
  inline MultiSettings* mutable_settings() { return &settings_; }
  
  void set_custom_pitch(uint8_t pitch_class, int8_t correction) {
    settings_.custom_pitch_table[pitch_class] = correction;
  }
  
  // Returns true when no part does anything fancy with the MIDI stream (such
  // as producing arpeggiated notes, or suppressing messages). This means that
  // the MIDI dispatcher can just copy to the MIDI out a MIDI data byte as soon
  // as it is received. Otherwise, merging and message reformatting will be
  // necessary and the output stream will be delayed :(
  inline bool direct_thru() const {
    for (uint8_t i = 0; i < num_active_parts_; ++i) {
      if (!part_[i].direct_thru()) {
        return false;
      }
    }
    return true;
  }
  
  void GetCvGate(uint16_t* cv, bool* gate);
  bool GetAudioSource(uint8_t* audio_source);
  void GetLedsBrightness(uint8_t* brightness);

  template<typename T>
  void Serialize(T* stream_buffer) {
    stream_buffer->Write(settings());
    for (uint8_t i = 0; i < kNumParts; ++i) {
      stream_buffer->Write(part_[i].midi_settings());
      stream_buffer->Write(part_[i].voicing_settings());
      stream_buffer->Write(part_[i].sequencer_settings());
    }
  };
  
  template<typename T>
  void Deserialize(T* stream_buffer) {
    Stop();
    stream_buffer->Read(mutable_settings());
    for (uint8_t i = 0; i < kNumParts; ++i) {
      stream_buffer->Read(part_[i].mutable_midi_settings());
      stream_buffer->Read(part_[i].mutable_voicing_settings());
      stream_buffer->Read(part_[i].mutable_sequencer_settings());
    }
    Touch();
  };
  
  template<typename T>
  void SerializeCalibration(T* stream_buffer) {
    for (uint8_t i = 0; i < kNumVoices; ++i) {
      for (uint8_t j = 0; j < kNumOctaves; ++j) {
        stream_buffer->Write(voice_[i].calibration_dac_code(j));
      }
    }
  };
  
  template<typename T>
  void DeserializeCalibration(T* stream_buffer) {
    for (uint8_t i = 0; i < kNumVoices; ++i) {
      for (uint8_t j = 0; j < kNumOctaves; ++j) {
        uint16_t v;
        stream_buffer->Read(&v);
        voice_[i].set_calibration_dac_code(j, v);
      }
    }
  };

  void StartLearning() {
    layout_configurator_.StartLearning();
  }

  void StopLearning() {
    layout_configurator_.StopLearning(this);
  }

  inline bool learning() const {
    return layout_configurator_.learning();
  }
  
  void StartSong();

 private:
  void ChangeLayout(Layout old_layout, Layout new_layout);
  void UpdateLayout();
  void ClockSong();
  void HandleRemoteControlCC(uint8_t controller, uint8_t value);
  
  MultiSettings settings_;
  
  bool running_;
  bool started_by_keyboard_;
  bool latched_;
  bool recording_;
  
  InternalClock internal_clock_;
  uint8_t internal_clock_ticks_;
  
  uint8_t clock_input_prescaler_;
  uint8_t clock_output_prescaler_;
  uint16_t bar_position_;
  uint8_t stop_count_down_;
  
  uint16_t clock_pulse_counter_;
  uint16_t reset_pulse_counter_;
  
  // Indicates that a setting has been changed and that the multi should
  // be saved in memory.
  bool dirty_;
  
  uint8_t num_active_parts_;
  
  Part part_[kNumParts];
  Voice voice_[kNumVoices];

  LayoutConfigurator layout_configurator_;
  
  const uint8_t* song_pointer_;
  uint32_t song_clock_;
  uint8_t song_delta_;

  DISALLOW_COPY_AND_ASSIGN(Multi);
};

extern Multi multi;

}  // namespace yarns

#endif // YARNS_MULTI_H_
