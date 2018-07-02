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
// Settings written to eeprom

#ifndef EDGES_SETTINGS_H_
#define EDGES_SETTINGS_H_

#include "avrlibx/avrlibx.h"
#include "avrlibx/utils/op.h"

#include "edges/digital_oscillator.h"
#include "edges/hardware_config.h"
#include "edges/timer_oscillator.h"

namespace edges {

static const uint16_t kHysteresisThreshold = 32;

struct ChannelData {
  uint8_t pw;
  bool quantized;
  bool arpeggio;
  uint8_t num_arpeggio_steps;
  
  int8_t arpeggio_step;
  // What are stored are not CV values; but instead values relative to the
  // first note of the sequence. For example, the sequence
  // 1000 2000 4000 2000
  // is stored as:
  // +1000, +2000, -2000
  // This sequence of values is added to the current CV. This allows
  // transposition.
  int16_t arpeggio_steps[7];
  
  int16_t offset;
  int16_t scale;
  int16_t fm_offset;
  
  inline int16_t dac_to_pitch(int16_t dac_code) const {
    int16_t pitch = S16U12MulShift12(scale, dac_code);
    pitch += offset;
    if (pitch < 0) {
      pitch = 0;
    }
    if (quantized) {
      pitch += 64;  // rounding
      pitch &= 0xff80;
    }
    if (pitch >= 16383) {
      pitch = 16383;
    }
    return pitch;
  }
  
  inline int16_t dac_to_fm(int16_t dac_code) const {
    return pw == PULSE_WIDTH_CV_CONTROLLED
        ? 0
        : S16U12MulShift12(scale, dac_code) + fm_offset;
  }

  inline int16_t arpeggio_offset() const {
    if (!arpeggio ||
        arpeggio_step == 0 ||
        arpeggio_step >= num_arpeggio_steps ||
        num_arpeggio_steps == 1) {
      return 0;
    } else {
      if (arpeggio_step == -1) {
        return arpeggio_steps[num_arpeggio_steps - 2];
      } else {
        return arpeggio_steps[arpeggio_step - 1];
      }
    }
  }
  
  inline void StartRecording() {
    num_arpeggio_steps = 1;
    arpeggio_step = -1;  // -1 = play the step currently being recorded
    arpeggio = true;
  }
  
  inline void UpdateArpeggiatorStep(
      uint16_t root_dac_code,
      uint16_t dac_code) {
    if (num_arpeggio_steps > 1) {
      int16_t this_note = dac_to_pitch(dac_code);
      int16_t root_note = dac_to_pitch(root_dac_code);
      arpeggio_steps[num_arpeggio_steps - 2] = this_note - root_note;
    }   
  }
  
  inline void StopRecording() {
    arpeggio_step = 0;
  }
  
  inline bool NextArpeggiatorStep() {
    if (num_arpeggio_steps < 8) {
      ++num_arpeggio_steps;
      return true;
    } else {
      StopRecording();
      return false;
    }
  }
};

struct SettingsData {
  ChannelData channel[4];
  uint8_t midi_channel;
  uint8_t midi_mode;
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  static void Init(bool reset_to_factory_defaults);
  
  static PulseWidth pw(uint8_t channel) {
    return static_cast<PulseWidth>(data_.channel[channel].pw);
  }

  static bool quantized(uint8_t channel) {
    return data_.channel[channel].quantized;
  }

  static bool arpeggio(uint8_t channel) {
    return data_.channel[channel].arpeggio;
  }

  static OscillatorShape shape(uint8_t channel) {
    return static_cast<OscillatorShape>(data_.channel[channel].pw);
  }
 
  static inline uint8_t num_steps(uint8_t channel) {
    return data_.channel[channel].num_arpeggio_steps;
  }
  
  static inline int16_t dac_to_fm(uint8_t channel, int16_t dac_code) {
#ifdef OCTAL_ADC
    return data_.channel[channel].dac_to_fm(dac_code);
#else
    return 0;
#endif  // OCTAL_ADC
  }
  
  static int16_t dac_to_pitch(uint8_t channel, int16_t dac_code) {
    const ChannelData& data = data_.channel[channel];
    int16_t pitch = data.dac_to_pitch(dac_code);
    if (data.quantized) {
      // When pitch is quantized, apply some hysteresis to DAC code.
      int16_t delta = dac_code - previous_code_[channel];
      if (delta < 0) {
        delta = -delta;
      }
      if (delta < kHysteresisThreshold) {
        pitch = previous_pitch_[channel];
      } else {
        previous_pitch_[channel] = pitch;
        previous_code_[channel] = dac_code;
      }
    }
    return pitch + data.arpeggio_offset();
  }
  
  static inline uint8_t midi_channel() { return data_.midi_channel; }
  static inline uint8_t midi_mode() { return data_.midi_mode; }
  
  static void set_midi_channel(uint8_t midi_channel) {
    data_.midi_channel = midi_channel;
    Save();
  }

  static void ToggleMidiMode() {
    data_.midi_mode = (data_.midi_mode + 1) & 1;
    Save();
  }
  
  static void StepPW(uint8_t channel) {
    uint8_t value = data_.channel[channel].pw + 1;
    if (value > PULSE_WIDTH_CV_CONTROLLED) {
      value = PULSE_WIDTH_50;
    }
    data_.channel[channel].pw = value;
  }
  
  static void StepArpeggio(uint8_t channel) {
    ChannelData* data = &data_.channel[channel];
    if (data->arpeggio_step != -1)  {
      ++data->arpeggio_step;
      if (data->arpeggio_step >= data->num_arpeggio_steps) {
        data->arpeggio_step = 0;
      }
    }
  }
  
  static void ToggleQuantizer(uint8_t channel) {
    data_.channel[channel].quantized ^= true;
  }
  
  static void ToggleArpeggio(uint8_t channel) {
    data_.channel[channel].arpeggio ^= true;
  }
  
  static void Calibrate(
      uint8_t channel,
      int16_t dac_code_c2,
      int16_t dac_code_c4,
      int16_t dac_code_fm) {
    if (dac_code_c4 != dac_code_c2) {
      int16_t scale = (24 * 128 * 4096L) / (dac_code_c4 - dac_code_c2);
      data_.channel[channel].scale = scale;
      data_.channel[channel].offset = (60 << 7) - \
          S16U12MulShift12(scale, (dac_code_c2 + dac_code_c4) >> 1);
    }
    data_.channel[channel].fm_offset = \
        -S16U12MulShift12(data_.channel[channel].scale, dac_code_fm);
    Save();
  }
  
  static ChannelData* mutable_channel_data(uint8_t channel) {
    return &data_.channel[channel];
  }

  static void Save();
  
 private:
  static int16_t previous_code_[kNumChannels];
  static int16_t previous_pitch_[kNumChannels];
  static int16_t pitch_bend_[kNumChannels];
  static SettingsData data_;
  
  DISALLOW_COPY_AND_ASSIGN(Settings);
};

extern Settings settings;

}  // namespace edges

#endif  // EDGES_SETTINGS_H_
