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
// Keyframe interpolator.

#ifndef FRAMES_KEYFRAMER_H_
#define FRAMES_KEYFRAMER_H_

#include "stmlib/stmlib.h"

namespace frames {
  
const uint8_t kNumChannels = 4;
const uint8_t kMaxNumKeyframe = 64;
const uint32_t kMagicNumber = 0xCAFEBABE;
const uint8_t kNumPaletteEntries = 8;

enum EasingCurve {
  EASING_CURVE_STEP,
  EASING_CURVE_LINEAR,
  EASING_CURVE_IN_QUARTIC,
  EASING_CURVE_OUT_QUARTIC,
  EASING_CURVE_SINE,
  EASING_CURVE_BOUNCE
};

struct ChannelSettings {
  EasingCurve easing_curve;
  uint8_t response;
};

struct Keyframe {
  uint16_t timestamp;
  uint16_t id;
  uint16_t values[kNumChannels];
};

struct KeyframeLess {
  bool operator()(const Keyframe& lhs, const Keyframe& rhs) {
    return lhs.timestamp < rhs.timestamp;
  }
};

class Keyframer {
 public:
  Keyframer() { }
  ~Keyframer() { }
  
  void Init();
  void Save(uint16_t slot);
  void Load(uint16_t slot);

  void Calibrate(int32_t dc_offset_frame_modulation);
  
  bool AddKeyframe(uint16_t timestamp, uint16_t* values);
  bool RemoveKeyframe(uint16_t timestamp);
  
  int16_t FindNearestKeyframe(uint16_t timestamp, uint16_t tolerance);
  
  inline void set_immediate(uint8_t channel, uint16_t value) {
    immediate_[channel] = value;
  }
  
  inline uint16_t dac_code(uint8_t channel) const {
    return dac_code_[channel];
  }

  inline uint16_t level(uint8_t channel) const {
    return levels_[channel];
  }

  inline uint8_t color(uint8_t component) const {
    return color_[component];
  }
  inline const uint8_t* color() const {
    return &color_[0];
  }
  
  void Evaluate(uint16_t timestamp);
  
  inline ChannelSettings* mutable_settings(uint8_t channel) {
    return &settings_[channel];
  }

  inline const ChannelSettings& mutable_settings(uint8_t channel) const {
    return settings_[channel];
  }
  
  inline Keyframe* mutable_keyframe(uint16_t index) {
    return &keyframes_[index];
  }
  
  inline const Keyframe& keyframe(uint16_t index) const {
    return keyframes_[index];
  }
  
  inline uint16_t num_keyframes() const { return num_keyframes_; }
  
  uint16_t Easing(int32_t from, int32_t to, uint32_t scale, EasingCurve curve);
  
  // This creates a sample animation (between 0 to 65535 and back to 0) used
  // for animating the LED when editing the easing curve or response.
  uint16_t SampleAnimation(uint8_t channel, uint16_t tick, bool easing);
  
  inline int16_t position() const { return position_; }
  inline int16_t nearest_keyframe() const { return nearest_keyframe_; }
  
  void Clear();
  void Reset();
  void Randomize();

  static uint16_t ConvertToDacCode(uint16_t gain, uint8_t response);

  // In addition to the keyframer data, this extra word stores in
  // persistent storage things like sequencer mode on/off and
  // poly lfo mode on/off states.
  inline int32_t dc_offset_frame_modulation() const {
    return dc_offset_frame_modulation_;
  }

 private:
  uint16_t FindKeyframe(uint16_t timestamp);
   
  Keyframe keyframes_[kMaxNumKeyframe];
  ChannelSettings settings_[kNumChannels];
  uint16_t num_keyframes_;
  uint16_t id_counter_;
 public:
  uint8_t feature_mode_;
  uint8_t euclidean_length_[kNumChannels];
  uint16_t euclidean_shape_[kNumChannels];
 private:
  int32_t dc_offset_frame_modulation_;
  uint32_t magic_number_;
  uint8_t padding_[3];

#ifndef TEST
  enum SettingsSize {
    SETTINGS_SIZE = sizeof(keyframes_) + sizeof(settings_) +            \
    sizeof(num_keyframes_) + sizeof(id_counter_) + sizeof(feature_mode_) + \
    sizeof(euclidean_length_) + sizeof(euclidean_shape_) + \
    sizeof(dc_offset_frame_modulation_) + sizeof(magic_number_) + sizeof(padding_)
  };
#endif  // TEST

  uint16_t version_token_;

  int16_t position_;
  int16_t nearest_keyframe_;

  uint16_t dac_code_[kNumChannels];
  uint16_t levels_[kNumChannels];
  uint16_t immediate_[kNumChannels];
  
  uint8_t color_[3];
  
  static const uint8_t palette_[kNumPaletteEntries][3];
  
  DISALLOW_COPY_AND_ASSIGN(Keyframer);
};

}  // namespace frames

#endif  // FRAMES_KEYFRAMER_H_
