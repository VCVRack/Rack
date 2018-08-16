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

#include "edges/settings.h"

#include "edges/storage.h"

namespace edges {

/* static */
SettingsData Settings::data_;

/* static */
int16_t Settings::previous_code_[kNumChannels];

/* static */
int16_t Settings::previous_pitch_[kNumChannels];

/* extern */
Settings settings;

typedef SettingsData PROGMEM prog_SettingsData;

const prog_SettingsData init_settings PROGMEM = {
  { { 0, false, false, 3, 0,
      { 512, 1536, 0, 0, 0, 0, 0 }, 1536, 15360, -7680
    },
    
    { 0, false, false, 3, 0,
      { 512, 896, 0, 0, 0, 0, 0 }, 1536, 15360, -7680
    },
    
    { 0, false, false, 4, 0,
      { 512, 896, 1536, 0, 0, 0, 0 }, 1536, 15360, -7680
    },
    
    { 0, false, false, 5, 0,
      { 512, 896, 1536, 3072, 0, 0, 0 }, 1536, 15360, -7680
    } },
  0,
  0
};

template<>
struct StorageLayout<SettingsData> {
  static uint16_t eeprom_address() { 
    return 0;
  }
  static const prog_char* init_data() {
    return (prog_char*)&init_settings;
  }
};

/* static */
void Settings::Init(bool reset_to_factory_defaults) {
  if (reset_to_factory_defaults) {
    storage.ResetToFactoryDefaults(&data_);
  } else {
    storage.Load(&data_);
  }
}

/* static */
void Settings::Save() {
  storage.Save(data_);
}

}  // namespace edges
