// Copyright 2013 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "peaks/processors.h"

using namespace peaks;
using namespace stmlib;

const uint32_t kSampleRate = 48000;

void write_wav_header(FILE* fp, int num_samples, int num_channels) {
  uint32_t l;
  uint16_t s;
  
  fwrite("RIFF", 4, 1, fp);
  l = 36 + num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  fwrite("WAVE", 4, 1, fp);
  
  fwrite("fmt ", 4, 1, fp);
  l = 16;
  fwrite(&l, 4, 1, fp);
  s = 1;
  fwrite(&s, 2, 1, fp);
  s = num_channels;
  fwrite(&s, 2, 1, fp);
  l = kSampleRate;
  fwrite(&l, 4, 1, fp);
  l = static_cast<uint32_t>(kSampleRate) * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  s = 2 * num_channels;
  fwrite(&s, 2, 1, fp);
  s = 16;
  fwrite(&s, 2, 1, fp);
  
  fwrite("data", 4, 1, fp);
  l = num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
}

int main(void) {
  FILE* fp = fopen("peaks.wav", "wb");
  write_wav_header(fp, kSampleRate * 10, 1);
  processors[0].Init(1);
  processors[0].set_control_mode(CONTROL_MODE_HALF);
  processors[0].set_function(PROCESSOR_FUNCTION_BASS_DRUM);
  processors[0].set_parameter(0, 60000);
  processors[0].set_parameter(1, 40000);
  
  uint32_t period = kSampleRate / 2;
  for (uint32_t i = 0; i < kSampleRate * 10 ; ++i) {
    uint16_t tri = i;
    tri = tri > 32767 ? 65535 - tri : tri;
    uint8_t control = 0;
    if (i % period < (period / 4)) {
      control |= CONTROL_GATE;
    }
    if (i % period == 0) {
      control |= CONTROL_GATE_RISING;
    }
    int16_t s = processors[0].Process(control);
    fwrite(&s, sizeof(s), 1, fp);
    processors[0].Buffer();
  }
}
