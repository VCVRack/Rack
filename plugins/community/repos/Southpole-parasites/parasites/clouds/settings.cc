// Copyright 2014 Olivier Gillet.
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
// Settings storage.

#include "clouds/settings.h"

#include "stmlib/system/storage.h"

#include "clouds/dsp/granular_processor.h"

namespace clouds {

stmlib::Storage<1> storage;

void Settings::Init() {
  freshly_baked_ = false;
  if (!storage.ParsimoniousLoad(&data_, &version_token_)) {
    data_.calibration_data.pitch_offset = 66.67f;
    data_.calibration_data.pitch_scale = -84.26f;
    for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
      data_.calibration_data.offset[i] = 0.505f;
    }
    data_.state.quality = 0;
    data_.state.blend_parameter = 0;
    data_.state.playback_mode = PLAYBACK_MODE_GRANULAR;
    data_.state.blend_value[0] = 255;
    data_.state.blend_value[1] = 128;
    data_.state.blend_value[2] = 0;
    data_.state.blend_value[3] = 0;
    freshly_baked_ = true;
    Save();
  }
}

void Settings::SaveSampleMemory(
    uint32_t index,
    PersistentBlock* blocks,
    size_t num_blocks) {
  uint32_t* data = mutable_sample_flash_data(index);
  
  // Unprotect flash and erase sector.
  FLASH_Unlock();
  FLASH_ClearFlag(
      FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
      FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR); 
  FLASH_EraseSector(sample_flash_sector(index) * 8, VoltageRange_3);
  
  // Write all data blocks.
  for (size_t block = 0; block < num_blocks; ++block) {
    FLASH_ProgramWord((uint32_t)(data++), blocks[block].tag);
    FLASH_ProgramWord((uint32_t)(data++), blocks[block].size);
    size_t size = blocks[block].size;
    const uint32_t* words = (const uint32_t*)(blocks[block].data);
    while (size >= 4) {
      FLASH_ProgramWord((uint32_t)(data++), *words++);
      size -= 4;
    }
  }
}

void Settings::Save() {
  storage.ParsimoniousSave(data_, &version_token_);
}

}  // namespace clouds
