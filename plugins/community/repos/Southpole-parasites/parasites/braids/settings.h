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
// Settings

#ifndef BRAIDS_SETTINGS_H_
#define BRAIDS_SETTINGS_H_

#include "stmlib/stmlib.h"

namespace braids {

enum MacroOscillatorShape {
  MACRO_OSC_SHAPE_CSAW,
  MACRO_OSC_SHAPE_MORPH,
  MACRO_OSC_SHAPE_SAW_SQUARE,
  MACRO_OSC_SHAPE_SINE_TRIANGLE,
  MACRO_OSC_SHAPE_BUZZ,
  
  MACRO_OSC_SHAPE_SQUARE_SYNC,
  MACRO_OSC_SHAPE_SAW_SYNC,
  MACRO_OSC_SHAPE_TRIPLE_SAW,
  MACRO_OSC_SHAPE_TRIPLE_SQUARE,
  MACRO_OSC_SHAPE_TRIPLE_TRIANGLE,
  MACRO_OSC_SHAPE_TRIPLE_SINE,
  MACRO_OSC_SHAPE_TRIPLE_RING_MOD,
  MACRO_OSC_SHAPE_SAW_SWARM,
  MACRO_OSC_SHAPE_SAW_COMB,
  MACRO_OSC_SHAPE_TOY,

  MACRO_OSC_SHAPE_DIGITAL_FILTER_LP,
  MACRO_OSC_SHAPE_DIGITAL_FILTER_PK,
  MACRO_OSC_SHAPE_DIGITAL_FILTER_BP,
  MACRO_OSC_SHAPE_DIGITAL_FILTER_HP,
  MACRO_OSC_SHAPE_VOSIM,
  MACRO_OSC_SHAPE_VOWEL,
  MACRO_OSC_SHAPE_VOWEL_FOF,
  
  MACRO_OSC_SHAPE_HARMONICS,

  MACRO_OSC_SHAPE_FM,
  MACRO_OSC_SHAPE_FEEDBACK_FM,
  MACRO_OSC_SHAPE_CHAOTIC_FEEDBACK_FM,

  MACRO_OSC_SHAPE_PLUCKED,
  MACRO_OSC_SHAPE_BOWED,
  MACRO_OSC_SHAPE_BLOWN,
  MACRO_OSC_SHAPE_FLUTED,
  MACRO_OSC_SHAPE_STRUCK_BELL,
  MACRO_OSC_SHAPE_STRUCK_DRUM,
  MACRO_OSC_SHAPE_KICK,
  MACRO_OSC_SHAPE_CYMBAL,
  MACRO_OSC_SHAPE_SNARE,

  MACRO_OSC_SHAPE_WAVETABLES,
  MACRO_OSC_SHAPE_WAVE_MAP,
  MACRO_OSC_SHAPE_WAVE_LINE,
  MACRO_OSC_SHAPE_WAVE_PARAPHONIC,

  MACRO_OSC_SHAPE_FILTERED_NOISE,
  MACRO_OSC_SHAPE_TWIN_PEAKS_NOISE,
  MACRO_OSC_SHAPE_CLOCKED_NOISE,
  MACRO_OSC_SHAPE_GRANULAR_CLOUD,
  MACRO_OSC_SHAPE_PARTICLE_NOISE,
  
  MACRO_OSC_SHAPE_DIGITAL_MODULATION,

  MACRO_OSC_SHAPE_QUESTION_MARK,
  // MACRO_OSC_SHAPE_YOUR_ALGO
  MACRO_OSC_SHAPE_LAST,
  MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META = MACRO_OSC_SHAPE_DIGITAL_MODULATION
};

enum Resolution {
  RESOLUTION_2_BIT,
  RESOLUTION_3_BIT,
  RESOLUTION_4_BIT,
  RESOLUTION_6_BIT,
  RESOLUTION_8_BIT,
  RESOLUTION_12_BIT,
  RESOLUTION_16_BIT,
  RESOLUTION_LAST
};

enum SampleRate {
  SAMPLE_RATE_4K,
  SAMPLE_RATE_8K,
  SAMPLE_RATE_16K,
  SAMPLE_RATE_24k,
  SAMPLE_RATE_32k,
  SAMPLE_RATE_48K,
  SAMPLE_RATE_96K,
  SAMPLE_RATE_LAST
};

enum PitchRange {
  PITCH_RANGE_EXTERNAL,
  PITCH_RANGE_FREE,
  PITCH_RANGE_EXTENDED,
  PITCH_RANGE_440,
  PITCH_RANGE_LFO  // This setting is hidden by default!
};

enum Setting {
  SETTING_OSCILLATOR_SHAPE,
  SETTING_RESOLUTION,
  SETTING_SAMPLE_RATE,
  SETTING_AD_TIMBRE,
  SETTING_TRIG_SOURCE,
  SETTING_TRIG_DELAY,
  SETTING_META_MODULATION,
  SETTING_PITCH_RANGE,
  SETTING_PITCH_OCTAVE,
  SETTING_QUANTIZER_SCALE,
  SETTING_VCO_FLATTEN,
  SETTING_VCO_DRIFT,
  SETTING_SIGNATURE,
  SETTING_BRIGHTNESS,
  SETTING_AD_ATTACK,
  SETTING_AD_DECAY,
  SETTING_AD_FM,
  SETTING_AD_COLOR,
  SETTING_AD_VCA,
  SETTING_QUANTIZER_ROOT,
  SETTING_LAST_EDITABLE_SETTING = SETTING_QUANTIZER_ROOT,
  
  // Not settings per-se, but used for menu display!
  SETTING_CALIBRATION,
  SETTING_CV_TESTER,
  SETTING_MARQUEE,
  SETTING_VERSION,
  SETTING_LAST
};

struct SettingsData {
  uint8_t shape;
  uint8_t resolution;
  uint8_t sample_rate;
  uint8_t ad_timbre;
  uint8_t auto_trig;
  uint8_t trig_delay;
  uint8_t meta_modulation;
  uint8_t pitch_range;
  uint8_t pitch_octave;
  uint8_t quantizer_scale;
  uint8_t vco_flatten;
  uint8_t vco_drift;
  uint8_t signature;
  uint8_t brightness;
  uint8_t ad_attack;
  uint8_t ad_decay;
  uint8_t ad_fm;
  uint8_t ad_color;
  uint8_t ad_vca;
  uint8_t quantizer_root;
  
  int32_t pitch_cv_offset;
  int32_t pitch_cv_scale;
  int32_t fm_cv_offset;
  
  char marquee_text[63];
  char magic_byte;
};

struct SettingMetadata {
  uint8_t min_value;
  uint8_t max_value;
  const char name[5];
  const char* const* strings;
  
  int16_t Clip(int16_t value) const {
    if (value > max_value) {
      value = max_value;
    } else if (value < min_value) {
      value = min_value;
    }
    return value;
  }
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  void Init();
  void Save();
  void Reset();
  
  void SetValue(Setting setting, uint8_t value) {
    uint8_t* data = static_cast<uint8_t*>(static_cast<void*>(&data_));
    data[setting] = value;
  }
  
  uint8_t GetValue(Setting setting) const {
    const uint8_t* data = static_cast<const uint8_t*>(
        static_cast<const void*>(&data_));
    return data[setting];
  }
  
  inline MacroOscillatorShape shape() const {
    return static_cast<MacroOscillatorShape>(data_.shape);
  }
  
  inline Resolution resolution() const {
    return static_cast<Resolution>(data_.resolution);
  }
  
  inline SampleRate sample_rate() const {
    return static_cast<SampleRate>(data_.sample_rate);
  }
  
  inline bool vco_flatten() const {
    return data_.vco_flatten;
  }

  inline uint8_t vco_drift() const {
    return data_.vco_drift;
  }

  inline uint8_t signature() const {
    return data_.signature;
  }

  inline bool meta_modulation() const {
    return data_.meta_modulation;
  }
  
  inline uint8_t trig_delay() const {
    return data_.trig_delay;
  }
  
  inline int32_t quantizer_root() const {
    return data_.quantizer_root;
  }
  
  inline const char* marquee_text() const {
    return data_.marquee_text;
  }
  
  inline char* mutable_marquee_text() {
    return data_.marquee_text;
  }
  
  inline const SettingsData& data() const { return data_; }
  inline SettingsData* mutable_data() { return &data_; }
  
  void Calibrate(
      int32_t adc_code_c2,
      int32_t adc_code_c4,
      int32_t adc_code_fm) {
    if (adc_code_c4 != adc_code_c2) {
      int32_t scale = (24 * 128 * 4096L) / (adc_code_c4 - adc_code_c2);
      data_.pitch_cv_scale = scale;
      data_.pitch_cv_offset = (60 << 7) - 
          (scale * ((adc_code_c2 + adc_code_c4) >> 1) >> 12);
      data_.fm_cv_offset = adc_code_fm;
    }
    Save();
  }
  
  inline int32_t adc_to_pitch(int32_t pitch_adc_code) const {
    if (data_.pitch_range == PITCH_RANGE_EXTERNAL ||
        data_.pitch_range == PITCH_RANGE_LFO) {
      pitch_adc_code = pitch_adc_code * data_.pitch_cv_scale >> 12;
      pitch_adc_code += data_.pitch_cv_offset;
    } else if (data_.pitch_range == PITCH_RANGE_FREE) {
      pitch_adc_code = (pitch_adc_code - 1638);
      pitch_adc_code = pitch_adc_code * data_.pitch_cv_scale >> 12;
      pitch_adc_code += 60 << 7;
    } else if (data_.pitch_range == PITCH_RANGE_440) {
      pitch_adc_code = 69 << 7;
    } else {
      pitch_adc_code = (pitch_adc_code - 1638) * 9 >> 1;
      pitch_adc_code += 60 << 7;
    }
    return pitch_adc_code;
  }
  
  inline int32_t pitch_transposition() const {
    int32_t t = data_.pitch_range == PITCH_RANGE_LFO ? -36 << 7 : 0;
    t += (static_cast<int32_t>(data_.pitch_octave) - 2) * 12 * 128;
    return t;
  }
  
  inline int32_t adc_to_fm(int32_t fm_adc_code) const {
    fm_adc_code -= data_.fm_cv_offset;
    fm_adc_code = fm_adc_code * 7680 >> 12;
    if (data_.pitch_range == PITCH_RANGE_440) {
      fm_adc_code = 0;
    }
    return fm_adc_code;
  }

  inline bool paques() const {
    return paques_;
  }
  
  static const SettingMetadata& metadata(Setting setting) {
    return metadata_[setting];
  }

  static const Setting setting_at_index(int16_t index) {
    return settings_order_[index];
  }
  
 private:
  void CheckPaques();

  SettingsData data_;
  
  uint16_t version_token_;
  bool paques_;
  
  static const SettingMetadata metadata_[SETTING_LAST];
  static const Setting settings_order_[SETTING_LAST];

  DISALLOW_COPY_AND_ASSIGN(Settings);
};

extern Settings settings;

}  // namespace braids

#endif  // BRAIDS_SETTINGS_H_
