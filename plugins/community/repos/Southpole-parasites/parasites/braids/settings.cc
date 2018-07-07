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

#include "braids/settings.h"

#include <cstring>

#include "stmlib/system/storage.h"
#include "stmlib/utils/murmurhash3.h"

namespace braids {

using namespace stmlib;

const SettingsData kInitSettings = {
  MACRO_OSC_SHAPE_CSAW,
  
  RESOLUTION_16_BIT,
  SAMPLE_RATE_96K,
  
  0,  // AD->timbre
  false,  // Trig source
  1,  // Trig delay
  false,  // Meta modulation
  
  PITCH_RANGE_EXTERNAL,
  2,
  0,  // Quantizer is off
  false,
  false,
  false,
  
  2,  // Brightness
  0,  // AD attack
  5,  // AD decay
  0,  // AD->FM
  0,  // AD->COLOR
  0,  // AD->VCA
  0,  // Quantizer root
  
  50,
  15401,
  2048,
  "GREETINGS FROM MUTABLE INSTRUMENTS *EDIT ME*",
};

Storage<0x8020000, 4> storage;

void Settings::Init() {
  if (!storage.ParsimoniousLoad(&data_, &version_token_)) {
    Reset();
  }
  bool settings_within_range = true;
  for (int32_t i = 0; i <= SETTING_LAST_EDITABLE_SETTING; ++i) {
    const Setting setting = static_cast<Setting>(i);
    const SettingMetadata& setting_metadata = metadata(setting);
    uint8_t value = GetValue(setting);
    settings_within_range = settings_within_range && \
        value >= setting_metadata.min_value && \
        value <= setting_metadata.max_value;
  }
  settings_within_range = settings_within_range && data_.magic_byte == 'M';
  if (!settings_within_range) {
    Reset();
  }
  CheckPaques();
}

void Settings::Reset() {
  memcpy(&data_, &kInitSettings, sizeof(SettingsData));
  data_.magic_byte = 'M';
}

void Settings::Save() {
  data_.magic_byte = 'M';
  storage.ParsimoniousSave(data_, &version_token_);
  CheckPaques();
}

void Settings::CheckPaques() {
  paques_ = !strcmp(data_.marquee_text, "49");
}

const char* const boolean_values[] = { "OFF ", "ON " };
const char* const intensity_values[] = {
    "OFF ",
    "   1",
    "   2",
    "   3",
    "FULL" };

const char* const zero_to_fifteen_values[] = {
    "   0",
    "   1",
    "   2",
    "   3",
    "   4",
    "   5",
    "   6",
    "   7",
    "   8",
    "   9",
    "  10",
    "  11",
    "  12",
    "  13",
    "  14",
    "  15"};

const char* const algo_values[] = {
    "CSAW",
    "^\x88\x8D_",
    "\x88\x8A\x8C\x8D",
    "FOLD",
    "\x8E\x8E\x8E\x8E",
    "SYN\x8C",
    "SYN\x88",
    "\x88\x88x3",
    "\x8C_x3",
    "/\\x3",
    "SIx3",
    "RING",
    "\x88\x89\x88\x89",
    "\x88\x88\x8E\x8E",
    "TOY*",
    "ZLPF",
    "ZPKF",
    "ZBPF",
    "ZHPF",
    "VOSM",
    "VOWL",
    "VFOF",
    "HARM",
    "FM  ",
    "FBFM",
    "WTFM",
    "PLUK",
    "BOWD",
    "BLOW",
    "FLUT",
    "BELL",
    "DRUM",
    "KICK",
    "CYMB",
    "SNAR",
    "WTBL",
    "WMAP",
    "WLIN",
    "WTx4",
    "NOIS",
    "TWNQ",
    "CLKN",
    "CLOU",
    "PRTC",
    "QPSK",
    // "NAME" // For your algorithm
};

const char* const bits_values[] = {
    "2BIT",
    "3BIT",
    "4BIT",
    "6BIT",
    "8BIT",
    "12B",
    "16B " };
    
const char* const rates_values[] = {
    "4KHZ",
    "8KHZ",
    "16K ",
    "24K ",
    "32K ",
    "48K ",
    "96K " };
    
const char* const quantization_values[] = {
    "OFF ",
    "SEMI",
    "IONI",
    "DORI",
    "PHRY",
    "LYDI",
    "MIXO",
    "AEOL",
    "LOCR",
    "BLU+",
    "BLU-",
    "PEN+",
    "PEN-",
    "FOLK",
    "JAPA",
    "GAME",
    "GYPS",
    "ARAB",
    "FLAM",
    "WHOL",
    "PYTH",
    "EB/4",
    "E /4",
    "EA/4",
    "BHAI",
    "GUNA",
    "MARW",
    "SHRI",
    "PURV",
    "BILA",
    "YAMA",
    "KAFI",
    "BHIM",
    "DARB",
    "RAGE",
    "KHAM",
    "MIMA",
    "PARA",
    "RANG",
    "GANG",
    "KAME",
    "PAKA",
    "NATB",
    "KAUN",
    "BAIR",
    "BTOD",
    "CHAN",
    "KTOD",
    "JOGE" };

const char* const trig_source_values[] = { "EXT.", "AUTO" };

const char* const pitch_range_values[] = {
    "EXT.",
    "FREE",
    "XTND",
    "440 ",
    "LFO "
};

const char* const octave_values[] = { "-2", "-1", "0", "1", "2" };

const char* const trig_delay_values[] = {
    "NONE",
    "125u",
    "250u",
    "500u",
    "1ms ",
    "2ms ",
    "4ms "
};

const char* const brightness_values[] = {
    "\xff   ",
    "\xff\xff  ",
    "\xff\xff\xff\xff",
};

const char* const note_values[] = {
    "C",
    "Db",
    "D",
    "Eb",
    "E",
    "F",
    "Gb",
    "G",
    "Ab",
    "A",
    "Bb",
    "B",
};

/* static */
const SettingMetadata Settings::metadata_[] = {
  { 0, MACRO_OSC_SHAPE_LAST - 2, "WAVE", algo_values },
  { 0, RESOLUTION_LAST - 1, "BITS", bits_values },
  { 0, SAMPLE_RATE_LAST - 1, "RATE", rates_values },
  { 0, 15, "\x8F""TIM", zero_to_fifteen_values },
  { 0, 1, "TSRC", trig_source_values },
  { 0, 6, "TDLY", trig_delay_values },
  { 0, 1, "META", boolean_values },
  { 0, 3, "RANG", pitch_range_values },
  { 0, 4, "OCTV", octave_values },
  { 0, 48, "QNTZ", quantization_values },
  { 0, 1, "FLAT", boolean_values },
  { 0, 4, "DRFT", intensity_values },
  { 0, 4, "SIGN", intensity_values },
  { 0, 2, "BRIG", brightness_values },
  { 0, 15, "\x8F""ATT", zero_to_fifteen_values },
  { 0, 15, "\x8F""DEC", zero_to_fifteen_values },
  { 0, 15, "\x8F""FM ", zero_to_fifteen_values },
  { 0, 15, "\x8F""COL", zero_to_fifteen_values },
  { 0, 1, "\x8F""VCA", boolean_values },
  { 0, 11, "ROOT", note_values },
  { 0, 0, "CAL.", NULL },
  { 0, 0, "    ", NULL },  // Placeholder for CV tester
  { 0, 0, "    ", NULL },  // Placeholder for marquee
  { 0, 0, "v1.8", NULL },  // Placeholder for version string
};

/* static */
const Setting Settings::settings_order_[] = {
  SETTING_OSCILLATOR_SHAPE,
  SETTING_META_MODULATION,
  SETTING_RESOLUTION,
  SETTING_SAMPLE_RATE,
  SETTING_TRIG_SOURCE,
  SETTING_TRIG_DELAY,
  SETTING_AD_ATTACK,
  SETTING_AD_DECAY,
  SETTING_AD_FM,
  SETTING_AD_TIMBRE,
  SETTING_AD_COLOR,
  SETTING_AD_VCA,
  SETTING_PITCH_RANGE,
  SETTING_PITCH_OCTAVE,
  SETTING_QUANTIZER_SCALE,
  SETTING_QUANTIZER_ROOT,
  SETTING_VCO_FLATTEN,
  SETTING_VCO_DRIFT,
  SETTING_SIGNATURE,
  SETTING_BRIGHTNESS,
  SETTING_CALIBRATION,
  SETTING_CV_TESTER,
  SETTING_MARQUEE,
  SETTING_VERSION,
};

/* extern */
Settings settings;

}  // namespace braids
