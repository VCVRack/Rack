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
// Parameter definitions.

#ifndef YARNS_SETTINGS_H_
#define YARNS_SETTINGS_H_

#include "stmlib/stmlib.h"

#include "yarns/multi.h"
#include "yarns/part.h"

namespace yarns {

enum GlobalSetting {
  GLOBAL_ACTIVE_PART
};

struct GlobalSettings {
  uint8_t active_part;
};

enum SettingDomain {
  SETTING_DOMAIN_GLOBAL,
  SETTING_DOMAIN_MULTI,
  SETTING_DOMAIN_PART,
};

enum SettingUnit {
  SETTING_UNIT_UINT8,
  SETTING_UNIT_INT8,
  SETTING_UNIT_INDEX,
  SETTING_UNIT_TEMPO,
  SETTING_UNIT_MIDI_CHANNEL,
  SETTING_UNIT_MIDI_CHANNEL_OFF,
  SETTING_UNIT_BAR_DURATION,
  SETTING_UNIT_VIBRATO_SPEED,
  SETTING_UNIT_PORTAMENTO,
  SETTING_UNIT_ENUMERATION,
};

enum SettingIndex {
  SETTING_LAYOUT,
  SETTING_ACTIVE_PART_4,
  SETTING_ACTIVE_PART_2,
  SETTING_CLOCK_TEMPO,
  SETTING_CLOCK_SWING,
  SETTING_CLOCK_INPUT_DIVISION,
  SETTING_CLOCK_OUTPUT_DIVISION,
  SETTING_CLOCK_BAR_DURATION,
  SETTING_CLOCK_OVERRIDE,
  SETTING_MIDI_CHANNEL,
  SETTING_MIDI_MIN_NOTE,
  SETTING_MIDI_MAX_NOTE,
  SETTING_MIDI_NOTE,
  SETTING_MIDI_MIN_VELOCITY,
  SETTING_MIDI_MAX_VELOCITY,
  SETTING_MIDI_OUT_MODE,
  SETTING_VOICING_ALLOCATION_MODE,
  SETTING_VOICING_ALLOCATION_PRIORITY,
  SETTING_VOICING_PORTAMENTO,
  SETTING_VOICING_LEGATO_MODE,
  SETTING_VOICING_PITCH_BEND_RANGE,
  SETTING_VOICING_VIBRATO_RANGE,
  SETTING_VOICING_MODULATION_RATE,
  SETTING_VOICING_TUNING_TRANSPOSE,
  SETTING_VOICING_TUNING_FINE,
  SETTING_VOICING_TUNING_ROOT,
  SETTING_VOICING_TUNING_SYSTEM,
  SETTING_VOICING_TRIGGER_DURATION,
  SETTING_VOICING_TRIGGER_SCALE,
  SETTING_VOICING_TRIGGER_SHAPE,
  SETTING_VOICING_CV_OUT,
  SETTING_VOICING_CV_OUT_3,
  SETTING_VOICING_CV_OUT_4,
  SETTING_VOICING_AUDIO_MODE,
  SETTING_SEQUENCER_CLOCK_DIVISION,
  SETTING_SEQUENCER_GATE_LENGTH,
  SETTING_SEQUENCER_ARP_RANGE,
  SETTING_SEQUENCER_RHYTHM_GENERATOR,  // Alias for arp range
  SETTING_SEQUENCER_ARP_DIRECTION,
  SETTING_SEQUENCER_ARP_DIRECTION_NO_CHORD,
  SETTING_SEQUENCER_ARP_PATTERN,
  SETTING_SEQUENCER_RHYTHM_PATTERN,  // Alias for arp pattern
  SETTING_SEQUENCER_EUCLIDEAN_LENGTH,
  SETTING_SEQUENCER_EUCLIDEAN_FILL,
  SETTING_SEQUENCER_EUCLIDEAN_ROTATE,
  SETTING_REMOTE_CONTROL_CHANNEL,

  SETTING_LAST
};

struct Setting {
  const char short_name[3];
  const char* const name;
  SettingDomain domain;
  uint16_t address[2];
  SettingUnit unit;
  int16_t min_value;
  int16_t max_value;
  const char* const* values;
  uint8_t part_cc;
  uint8_t remote_control_cc;
  
  uint8_t Scale(uint8_t value_7bits) const {
    uint8_t scaled_value;
    uint8_t range = max_value - min_value + 1;
    scaled_value = range * value_7bits >> 7;
    scaled_value += min_value;
    if (unit == SETTING_UNIT_TEMPO) {
      scaled_value &= 0xfe;
      if (scaled_value <= 38) {
        scaled_value = 39;
      }
    }
    return scaled_value;
  }
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  void Init();
  void Set(const Setting& setting, uint8_t value);
  void Set(const Setting& setting, uint8_t* part, uint8_t value);
  void SetFromCC(uint8_t part, uint8_t controller, uint8_t value);

  uint8_t Get(const Setting& setting) const;
  void Increment(const Setting& setting, int16_t increment);

  void Set(uint8_t address, uint8_t value);
  inline uint8_t Get(uint8_t address) const {
    const uint8_t* bytes;
    bytes = static_cast<const uint8_t*>(static_cast<const void*>(&global_));
    return bytes[address];
  }
  
  void Print(const Setting& setting, char* buffer) const;
  
  inline const Setting& setting(uint8_t index) const {
    return settings_[index];
  }
  
  static void PrintInteger(char* buffer, uint8_t number);
  const SettingIndex* menu() {
    return menus_[multi.layout()];
  }
  
 private:
  GlobalSettings global_;
   
  static const Setting settings_[SETTING_LAST];
  static const SettingIndex* const menus_[LAYOUT_LAST];
  
  uint8_t part_cc_map_[128];
  uint8_t remote_control_cc_map_[128];
  
  DISALLOW_COPY_AND_ASSIGN(Settings);
};

extern Settings settings;

}  // namespace yarns

#endif // YARNS_SETTINGS_H_
