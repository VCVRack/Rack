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
// Keyframe interpolator

#include "frames/keyframer.h"

#include <algorithm>

#ifndef TEST
#include "stmlib/system/storage.h"
#endif  // TEST

#include "stmlib/utils/random.h"

#include "frames/resources.h"

namespace frames {

using namespace std;


/* static */
const uint8_t Keyframer::palette_[kNumPaletteEntries][3] = {
  { 255, 0, 0 },
  { 255, 64, 0 },
  { 255, 255, 0 },
  { 64, 255, 0 },
  { 0, 255, 64 },
  { 0, 0, 255 },
  { 255, 0, 255 },
  { 255, 0, 64 },
};

#ifndef TEST
stmlib::Storage<0x8020000, 4> storage;
stmlib::Storage<0x801f000, 4> preset_storage;
#endif  // TEST

void Keyframer::Reset() {
    for (uint8_t i = 0; i < kNumChannels; ++i) {
      settings_[i].easing_curve = EASING_CURVE_LINEAR;
      settings_[i].response = 0;
    }
    feature_mode_ = 0;
    for (int i=0; i<kNumChannels; i++) {
      euclidean_length_[i] = 16;
      euclidean_shape_[i] = 4000;
    }
    magic_number_ = kMagicNumber;
    dc_offset_frame_modulation_ = 32767;
    Clear();
}

void Keyframer::Init() {
#ifndef TEST
  if (!storage.ParsimoniousLoad(keyframes_, SETTINGS_SIZE, &version_token_)) {
    Reset();
  }
#endif  // TEST
}

void Keyframer::Save(uint16_t slot) {
#ifndef TEST
  if (slot == 0)
    storage.ParsimoniousSave(keyframes_, SETTINGS_SIZE, &version_token_);
  else
    preset_storage.Save(keyframes_, SETTINGS_SIZE, slot - 1);
#endif  // TEST
}

void Keyframer::Load(uint16_t slot) {
#ifndef TEST
  if (slot == 0){
    storage.ParsimoniousLoad(keyframes_, SETTINGS_SIZE, &version_token_);
  } else {
    int32_t calibration = dc_offset_frame_modulation_;
    preset_storage.Load(keyframes_, SETTINGS_SIZE, slot - 1);
    // calibration data needs to remain like in the default slot.
    dc_offset_frame_modulation_ = calibration;
    // lightweight protection against corrupted/empty memory slots
    if (magic_number_ != kMagicNumber)
      Reset();
  }
#endif  // TEST
}

void Keyframer::Calibrate(int32_t dc_offset_frame_modulation) {
  dc_offset_frame_modulation_ = dc_offset_frame_modulation;
#ifndef TEST
  storage.ParsimoniousSave(keyframes_, SETTINGS_SIZE, &version_token_);
#endif  // TEST
}

void Keyframer::Clear() {
  Keyframe empty;
  empty.timestamp = 0;
  empty.id = 0;
  fill(&empty.values[0], &empty.values[kNumChannels], 0);
  fill(&keyframes_[0], &keyframes_[kMaxNumKeyframe], empty);
  num_keyframes_ = 0;
  id_counter_ = 0;
}

void Keyframer::Randomize() {
  for (int i=0; i<kMaxNumKeyframe; i++) {
    for (int j=0; j<kNumChannels; j++) {
      keyframes_[i].values[j] = stmlib::Random::GetWord();
    }
  }
}


uint16_t Keyframer::FindKeyframe(uint16_t timestamp) {
  if (!num_keyframes_) {
    return 0;
  }
  Keyframe dummy;
  dummy.timestamp = timestamp;
  return lower_bound(
      keyframes_,
      keyframes_ + num_keyframes_,
      dummy,
      KeyframeLess()) - keyframes_;
}

/* static */
uint16_t Keyframer::ConvertToDacCode(uint16_t gain, uint8_t response) {
  // Exponential response is easy, straight to the 2164.
  int32_t exponential = 65535 - gain;
  
  // Use a lookup table and interpolation to linearize the 2164
  int32_t a = lut_vca_linear[gain >> 6];
  int32_t b = lut_vca_linear[(gain >> 6) + 1];
  int32_t linear = a + ((b - a) * ((gain << 10) & 0xffff) >> 16);
  
  // Blend linear and exponential responses.
  uint16_t balance = lut_response_balance[response];
  return (linear + ((exponential - linear) * balance >> 15)) >> 4;
}

inline uint16_t Keyframer::Easing(
    int32_t from,
    int32_t to,
    uint32_t scale,
    EasingCurve curve) {
  int32_t shaped_scale = scale;
  if (curve == EASING_CURVE_STEP) {
    shaped_scale = scale < 32768 ? 0 : 65535;
  } else if (curve >= EASING_CURVE_IN_QUARTIC) {
    const uint16_t* easing_curve = lookup_table_table[
        curve - EASING_CURVE_IN_QUARTIC];
    int32_t scale_a = easing_curve[scale >> 6];
    int32_t scale_b = easing_curve[(scale >> 6) + 1];
    shaped_scale = scale_a + (((scale_b - scale_a) >> 1) * \
      ((scale << 10) & 0xffff) >> 15);
  }
  return from + ((to - from) * (shaped_scale >> 1) >> 15);
}

uint16_t Keyframer::SampleAnimation(
    uint8_t channel,
    uint16_t tick,
    bool easing) {
  uint16_t sample = Easing(
      tick > 32768 ? 65535 : 0,
      tick > 32768 ? 0 : 65535,
      (tick << 1) & 0xffff,
      easing ? settings_[channel].easing_curve : EASING_CURVE_LINEAR);
  if (!easing) {
    int32_t linear = sample;
    int32_t exponential = lut_exponential[sample >> 8];
    uint16_t balance = lut_response_balance[settings_[channel].response];
    sample = linear + ((exponential - linear) * balance >> 15);
  }
  return sample;
}

int16_t Keyframer::FindNearestKeyframe(uint16_t timestamp, uint16_t tolerance) {
  if (!num_keyframes_) {
    return -1;
  }
  uint16_t index = FindKeyframe(timestamp);
  uint16_t search_start = index ? index - 1 : 0;
  uint16_t search_end = index < num_keyframes_ - 1 ? index + 2 : num_keyframes_;
  for (uint16_t i = search_start; i < search_end; ++i) {
    uint16_t t = keyframes_[i].timestamp;
    int32_t distance = static_cast<int32_t>(t) - static_cast<int32_t>(timestamp);
    if (distance < tolerance && -distance < tolerance) {
      return i;
    }
  }
  return -1;
}

bool Keyframer::AddKeyframe(uint16_t timestamp, uint16_t* values) {
  if (num_keyframes_ == kMaxNumKeyframe) {
    return false;
  }
  
  uint16_t insertion_point = FindKeyframe(timestamp);
  if (insertion_point >= num_keyframes_ ||
      keyframes_[insertion_point].timestamp != timestamp) {
    for (int16_t i = num_keyframes_ - 1; i >= insertion_point; --i) {
      keyframes_[i + 1] = keyframes_[i];
    }
    keyframes_[insertion_point].timestamp = timestamp;
    keyframes_[insertion_point].id = id_counter_++;
    ++num_keyframes_;
  }
  copy(values, values + kNumChannels, keyframes_[insertion_point].values);
  return true;
}

bool Keyframer::RemoveKeyframe(uint16_t timestamp) {
  if (!num_keyframes_) {
    return false;
  }
  uint16_t splice_point = FindKeyframe(timestamp);
  if (keyframes_[splice_point].timestamp != timestamp) {
    return false;
  }
  
  for (uint16_t i = splice_point; i < num_keyframes_ - 1; ++i) {
    keyframes_[i] = keyframes_[i + 1];
  }
  --num_keyframes_;
  return true;
}

void Keyframer::Evaluate(uint16_t timestamp) {
  if (!num_keyframes_) {
    copy(immediate_, immediate_ + kNumChannels, levels_);
    fill(color_, color_ + 3, 0xff);
    position_ = -1;
    nearest_keyframe_ = -1;
  } else {
    uint16_t position = FindKeyframe(timestamp);
    position_ = position;

    // This is where the real interpolation takes place.
    volatile int8_t a_index = position == 0 ? num_keyframes_ - 1 :
      (position - 1) % num_keyframes_;
    int8_t b_index = position % num_keyframes_;
    volatile const Keyframe& a = keyframes_[a_index];
    volatile const Keyframe& b = keyframes_[b_index];

    volatile uint32_t scale;
    if (a.timestamp > b.timestamp) {
      // wraparound
      uint16_t rest = UINT16_MAX - a.timestamp;
      if (timestamp > a.timestamp) {
        scale = timestamp - a.timestamp;
      } else /* if (timestamp < b.timestamp) */ {
        scale = rest + timestamp;
      }
      scale <<= 16;
      scale /= rest + b.timestamp;
    } else {
      // normal
      scale = timestamp - a.timestamp;
      scale <<= 16;
      scale /= (b.timestamp - a.timestamp);
    }

    for (uint8_t i = 0; i < kNumChannels; ++i) {
      int32_t from = a.values[i];
      int32_t to = b.values[i];
      levels_[i] = Easing(from, to, scale, settings_[i].easing_curve);
    }
    for (uint8_t i = 0; i < 3; ++i) {
      uint8_t a_color = palette_[a.id & (kNumPaletteEntries - 1)][i];
      uint8_t b_color = palette_[b.id & (kNumPaletteEntries - 1)][i];
      color_[i] = a_color + ((b_color - a_color) * scale >> 16);
    }

    nearest_keyframe_ = scale < 32768 ? a_index + 1 : b_index + 1;
  }
  
  for (uint16_t i = 0; i < kNumChannels; ++i) {
    dac_code_[i] = ConvertToDacCode(levels_[i], settings_[i].response);
  }
}

}  // namespace frames
